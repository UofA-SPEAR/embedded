#ifndef _ENCODERS_H_
#define _ENCODERS_H_

#define POTA_PIN 	GPIO_PIN_0
#define POTA_PORT	GPIOB

#define POTB_PIN
#define POTB_PORT

#define ENCODERA_PIN_CH1	GPIO_PIN_6
#define ENCODERA_PIN_CH2	GPIO_PIN_7
#define ENCODERA_PORT		GPIOA

#define ENCODER_START_VAL 30000


typedef enum {
	ENCODER_POTENTIOMETER 		= 0, // Analog potentiometer (or any analog value)
	ENCODER_QUADRATURE 			= 1, // Any quadrature encoder
	ENCODER_ABSOLUTE_DIGITAL 	= 2, // specifically the digital encoder we are using
} encoder_type_t;


void pot_init(uint8_t motor);
void encoderA_init();
uint32_t pot_read(uint8_t motor);


#endif
