#ifndef _POT_H_
#define _POT_H_

#define POT_PIN GPIO_PIN_0

ADC_HandleTypeDef hadc1;

void potInit();
int readPot();


#endif
