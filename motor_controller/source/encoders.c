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
#define EMS22_CLK PAL_LINE(GPIOA, 5)
#define EMS22_DIN PAL_LINE(GPIOA, 6)
#define EMS22_CS PAL_LINE(GPIOB, 6)

static int32_t (*read_encoder)(void);

const ADCConversionGroup adcgrpcfg1 = {
	FALSE,
	1,
	NULL,
	NULL,
	ADC_CFGR_CONT,            /* CFGR    */
	ADC_TR(0, 4095),          /* TR1     */
	{                         /* SMPR[2] */
		ADC_SMPR1_SMP_AN0(ADC_SMPR_SMP_61P5),
		0
	},
	{                         /* SQR[4]  */
		ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1),
		0,
		0,
		0
	}
};

static void pot_init(void) {
	// Select inputs
	palSetLine(ENCODER_C1);
	palSetLine(ENCODER_C0);

	palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG);
	// do stuff
	adcStart(&ADCD1, NULL);
}

static int32_t pot_read(void) {
	adcsample_t buf[4];
	adcsample_t res = 0;
	adcConvert(&ADCD1, &adcgrpcfg1, buf, 4);
	for(int i = 0; i < 4; i++) {
		res += buf[i];
	}
	res /= 4;
	return res;
}

static void quadrature_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	RCC->APB1RSTR |= RCC_APB1RSTR_TIM3RST;
	RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM3RST);
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	palSetPadMode(GPIOA, 6, PAL_STM32_ALTERNATE(2));
	palSetPadMode(GPIOA, 7, PAL_STM32_ALTERNATE(2));
	// I don't know why the PAL drive doesn't set MODE correctly
	GPIOA->MODER |= (2 << GPIO_MODER_MODER6_Pos);
	GPIOA->MODER |= (2 << GPIO_MODER_MODER7_Pos);

	TIM3->ARR = 0xFFFF;
	TIM3->SMCR = 1 & TIM_SMCR_SMS; // Encoder mode 3
	TIM3->CCMR1 = ((uint32_t)0b01 << TIM_CCMR1_CC2S_Pos) |
		((uint32_t)0b01 << TIM_CCMR1_CC1S_Pos);
	TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
	TIM3->CR1 = TIM_CR1_CEN;

	// Start counter at the middle so we can go negative
	TIM3->CNT = ENCODER_START_VAL;
}

static int32_t quadrature_read(void)
{
	return (TIM3->CNT) - ENCODER_START_VAL;
}

static void ems22_init(void) {
//	SPIConfig spi_config = {
//		.circular = false,
//		.end_cb = NULL,
//		.ssport = GPIOC,
//		.sspad = 11,
//		.cr1 = SPI_CR1_RXONLY | SPI_CR1_SPE |
//			(0b101 << SPI_CR1_BR_Pos) | SPI_CR1_MSTR,
//		.cr2 = (0xFF << SPI_CR2_DS_Pos)
//	};
	palSetLine(ENCODER_C1);
	palClearLine(ENCODER_C0);
	palSetLine(ENCODER_CC);
	palSetLineMode(EMS22_CLK, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetLineMode(EMS22_CS, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetLineMode(EMS22_DIN, PAL_MODE_INPUT | PAL_STM32_OSPEED_HIGHEST);
	palSetLine(EMS22_CLK);
	palClearLine(EMS22_CS);
//	palSetPadMode(GPIOA, 5, PAL_STM32_ALTERNATE(5));
//	palSetPadMode(GPIOA, 6, PAL_STM32_ALTERNATE(5));


	//spiStart(&SPID1, &spi_config);
}

static int32_t ems22_read(void) {
	palSetLine(EMS22_CS);
	palClearLine(EMS22_CS);
	uint8_t buff[16] = {0};
	for (int i = 0; i < 16; i++) {
		palClearLine(EMS22_CLK);
		chThdSleep(TIME_US2I(1));
		palSetLine(EMS22_CLK);
		chThdSleep(TIME_US2I(1));
		buff[i] = palReadLine(EMS22_DIN);

	}
	palClearLine(EMS22_CLK);
	chThdSleep(TIME_US2I(1));
	palSetLine(EMS22_CLK);
	chThdSleep(TIME_US2I(1));
	uint8_t bit_count = 0;
	// Count bits
	for(int i = 0; i < 16; i++)	{
		bit_count += buff[i] & 1;
	}
	if(bit_count % 2 == buff[15]) {
		return -1;
	}
	else {
		uint16_t res = 0;
		for(int i = 0; i < 10; i++) {
			res |= buff[i];
			if(i < 9) res = res << 1;
		}
		return res;
	}

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

	//if (position < general.encoder_min)
	//	position = general.encoder_min;

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

static int32_t none_get_position(float position)
{
	if (position > 1.0)
		position = 1.0;
	else if (position < -1.0)
		position = -1.0;

	if(run_settings[get_id_by_name("spear.motor.reversed")].value.boolean) {
		position *= -1;
	}

	return position * 10000;
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
	palClearLine(ENCODER_CC);
	palClearLine(ENCODER_C0);
	palClearLine(ENCODER_C1);

	switch (encoder_type) {
		case (ENCODER_POTENTIOMETER):
			pot_init();
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
			palSetLine(ENCODER_CC);
			palSetLine(ENCODER_C0);
			read_encoder = ems22_read;
			get_position = digital_get_position;
			break;
		case (ENCODER_NONE):
			read_encoder = NULL;
			get_position = none_get_position;
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
