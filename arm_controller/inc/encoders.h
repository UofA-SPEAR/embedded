#ifndef _ENCODERS_H_
#define _ENCODERS_H_

#define POTA_PIN 	GPIO_PIN_0
#define POTA_PORT	GPIOB

#define POTB_PIN
#define POTB_PORT


void potA_init();
uint32_t potA_read();


#endif