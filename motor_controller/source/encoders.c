#include "encoders.h"
#include "hal.h"
#include "coms.h"
#include "settings.h"

#include "uavcan/protocol/NodeStatus.h"

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#define ENCODER_CC PAL_LINE(GPIOB, 2)
#define ENCODER_C0 PAL_LINE(GPIOB, 0)
#define ENCODER_C1 PAL_LINE(GPIOB, 1)

static int32_t (*read_encoder)(void);

const ADCConversionGroup adcgrpcfg1 = {
	FALSE,
	1,
	NULL,
	NULL,
	ADC_CFGR_CONT,            /* CFGR    */
	ADC_TR(0, 4095),          /* TR1     */
	{                         /* SMPR[2] */
		ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_61P5),
		0
	},
	{                         /* SQR[4]  */
		ADC_SQR1_SQ1_N(ADC_CHANNEL_IN2),
		0,
		0,
		0
	}
};

static void pot_init(void) {
	// do stuff
	adcStart(&ADCD1, NULL);
}

static int32_t pot_read(void) {
	adcsample_t buf;

	adcConvert(&ADCD1, &adcgrpcfg1, &buf, 1);

	return buf;
}

static void quadrature_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;

	palSetPadMode(GPIOB, 6, PAL_STM32_ALTERNATE(10));
	palSetPadMode(GPIOB, 8, PAL_STM32_ALTERNATE(10));

	TIM8->SMCR = 3 & TIM_SMCR_SMS; // Encoder mode 3
	TIM8->CCMR1 = ((uint32_t)0b01 << TIM_CCMR1_CC2S_Pos) |
		((uint32_t)0b01 << TIM_CCMR1_CC1S_Pos);
	TIM8->CR1 = TIM_CR1_CEN;

	// Start counter at the middle so we can go negative
	TIM8->CNT = ENCODER_START_VAL;
}

static int32_t quadrature_read(void)
{
	return (TIM8->CNT) - ENCODER_START_VAL;
}

static void ems22_init(void) {
	SPIConfig spi_config = {
		.circular = false,
		.end_cb = NULL,
		.ssport = GPIOC,
		.sspad = 11,
		.cr1 = SPI_CR1_RXONLY | SPI_CR1_SPE | 
			(0b101 << SPI_CR1_BR_Pos) | SPI_CR1_MSTR,
		.cr2 = (0xFF << SPI_CR2_DS_Pos)
	};

	palSetPadMode(GPIOB, 3, PAL_STM32_ALTERNATE(11));
	palSetPadMode(GPIOB, 4, PAL_STM32_ALTERNATE(11));

	spiStart(&SPID1, &spi_config);
}

static int32_t ems22_read(void) {
	struct {
		uint16_t abs_position : 10;
		uint8_t offset_compensation : 1;
		uint8_t cordic_overflow : 1;
		uint8_t linearity_alarm : 1;
		uint8_t magnitude_increase : 1;
		uint8_t magnitude_decrease : 1;
		uint8_t even_parity : 1;
	} in_data;

	spiSelectI(&SPID1);
	spiStartReceiveI(&SPID1, 2, &in_data);
	spiUnselectI(&SPID1);

	{
		uint16_t parity_check;
		uint8_t bit_count = 0;

		memcpy((void*) &parity_check, (void*) &in_data, 2);

		// Count bits
		while (parity_check) {
			bit_count += parity_check & 1;
			parity_check = parity_check >> 1;
		}

		// Parity error
		if ((parity_check % 2) == in_data.even_parity) {
			return -1;
		}
	}

	// If no errors, return data
	return in_data.abs_position;
}

static int32_t (*get_position)(float in_angle);

static struct {
	float to_radians;
	int32_t encoder_min;
	int32_t encoder_max;
} general;

static int32_t pot_get_position(float in_angle)
{
	int32_t position = in_angle / general.to_radians;

	position += general.encoder_min;

	if (position > general.encoder_max)
		position = general.encoder_max;

	return position;
}

static int32_t digital_get_position(float in_angle)
{
	return in_angle / general.to_radians;
}

static struct {
	float support_length;
	float arm_length;
	float length_min;
	float length_max;
} linear;

static int32_t linear_get_position(float in_angle)
{
	float desired_length;
	int32_t position;

	// Comes from cosine law
	// c^2 = a^2 + b^2 - 2ab*cos(C)
	desired_length = sqrt(
				pow(linear.support_length, 2) +
				pow(linear.arm_length, 2) -
				(2 * (linear.support_length) *
				(linear.arm_length) *
				cos(in_angle))
			);

	// TODO set nodestatus
	if (desired_length < linear.length_min) {
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING;
		desired_length = linear.length_min;
	} else if (desired_length > linear.length_max) {
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING;
		desired_length = linear.length_max;
	} else {
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
	}

	// These are checked to be positive in check_settings()
	uint32_t encoder_range = general.encoder_max -
			general.encoder_min;
	float linear_range = linear.length_max -
			linear.length_min;



	// set position properly
	position =
			// fit length into encoder range
			(desired_length - linear.length_min) *
			// Convert from length range into the encoder range
			(encoder_range / linear_range) +
			// Add the minimum encoder value
			general.encoder_min;


	return position;
}

void encoder_init(void)
{
	int encoder_type =
		run_settings[get_id_by_name("spear.motor.encoder.type")].value.integer;

	general.to_radians = 
		run_settings[get_id_by_name("spear.motor.encoder.to-radians")].value.real;
	general.encoder_min = 
		run_settings[get_id_by_name("spear.motor.encoder.min")].value.integer;
	general.encoder_max =
		run_settings[get_id_by_name("spear.motor.encoder.max")].value.integer;

	linear.support_length =
		run_settings[get_id_by_name("spear.motor.linear.support_length")].value.real;
	linear.arm_length =
		run_settings[get_id_by_name("spear.motor.linear.arm_length")].value.real;
	linear.length_min =
		run_settings[get_id_by_name("spear.motor.linear.length_min")].value.real;
	linear.length_max =
		run_settings[get_id_by_name("spear.motor.linear.length_max")].value.real;

	palSetLineMode(ENCODER_CC, PAL_MODE_OUTPUT_PUSHPULL);
	palSetLineMode(ENCODER_C0, PAL_MODE_OUTPUT_PUSHPULL);
	palSetLineMode(ENCODER_C1, PAL_MODE_OUTPUT_PUSHPULL);
	palSetLine(ENCODER_CC, 0);
	palSetLine(ENCODER_C0, 0);
	palSetLine(ENCODER_C1, 0);

	switch (encoder_type) {
		case (ENCODER_POTENTIOMETER):
			pot_init();
			palSetLine(ENCODER_C1, 1);
			read_encoder = pot_read;
			get_position = pot_get_position;
			break;
		case (ENCODER_QUADRATURE):
			quadrature_init();
			// We need to bodge the pins...
			read_encoder = quadrature_read;
			get_position = digital_get_position;
			break;
		case (ENCODER_ABSOLUTE_DIGITAL):
			ems22_init();
			palSetLine(ENCODER_CC, 1);
			palSetLine(ENCODER_C0, 1);
			read_encoder = ems22_read;
			get_position = digital_get_position;
			break;
		default:
			// TODO decent error handling
			while (0);
	}

	if (general.to_radians == (float) 0.0) {
		get_position = linear_get_position;
	}
}

int32_t encoder_read(void)
{
	return read_encoder();
}

int32_t encoder_get_position(float in_angle)
{
	return get_position(in_angle);
}