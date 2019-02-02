#ifndef _MOTOR_H_
#define _MOTOR_H_

#define PWM_PIN GPIO_PIN_1
#define DIR_PIN GPIO_PIN_5

enum Direction {REVERSE, FORWARD};
TIM_HandleTypeDef htim2;

void setMotor(int speed, enum Direction dir);
void motorInit();


#endif
