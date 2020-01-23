#include "encoders.h"
#include "hal.h"

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

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

void encoder_init(int encoder_type)
{
	switch (encoder_type) {
		case (ENCODER_POTENTIOMETER):
			pot_init();
			read_encoder = pot_read;
			break;
		case (ENCODER_QUADRATURE):
			quadrature_init();
			read_encoder = quadrature_read;
			break;
		case (ENCODER_ABSOLUTE_DIGITAL):
			ems22_init();
			read_encoder = ems22_read;
			break;
		default:
			// TODO decent error handling
			while (1);
	}
}

int32_t encoder_read(void)
{
	return read_encoder();
}