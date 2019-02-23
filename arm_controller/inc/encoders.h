#ifndef _ENCODERS_H_
#define _ENCODERS_H_

#define POTA_PIN 	GPIO_PIN_0
#define POTA_PORT	GPIOB

#define POTB_PIN
#define POTB_PORT

#define ENCODER_START_VAL 30000


typedef enum {
	ENCODER_POTENTIOMETER, // Analog potentiometer (or any analog value)
	ENCODER_QUADRATURE, // Any quadrature encoder
	ENCODER_ABSOLUTE_DIGITAL, // specifically the digital encoder we are using
} encoder_type_t;


void potA_init();
void encoderA_init();
uint32_t potA_read();


#endif
