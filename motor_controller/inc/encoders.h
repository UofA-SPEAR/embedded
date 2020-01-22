#ifndef _ENCODERS_H_
#define _ENCODERS_H_

#include <stdint.h>

#define POTA_PIN 	GPIO_PIN_0
#define POTA_PORT	GPIOB

#define POTB_PIN
#define POTB_PORT

#define ENCODERA_PIN_CH1	GPIO_PIN_6
#define ENCODERA_PIN_CH2	GPIO_PIN_7
#define ENCODERA_PORT		GPIOA

#define ENCODER_START_VAL 30000

// TODO find a more general way to handle this
#define NUM_ENCODER_TYPES	3


typedef enum {
	ENCODER_POTENTIOMETER 		= 0, // Analog potentiometer (or any analog value)
	ENCODER_QUADRATURE 			= 1, // Any quadrature encoder
	ENCODER_ABSOLUTE_DIGITAL 	= 2, // specifically the digital encoder we are using
} encoder_type_t;

void encoder_init(int encoder_type);
int32_t encoder_read(void);

#endif
