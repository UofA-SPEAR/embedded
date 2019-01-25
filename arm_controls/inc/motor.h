#ifndef _MOTOR_H_
#define _MOTOR_H_

#define MOTOR_PWM_PIN 	GPIO_PIN_7
#define MOTOR_INA_PIN 	GPIO_PIN_4
#define MOTOR_INB_PIN 	GPIO_PIN_3
#define MOTOR_ENA_PIN 	GPIO_PIN_6
#define MOTOR_ENB_PIN 	GPIO_PIN_5

#define MOTOR_PORT		GPIOB

enum Direction {REVERSE, FORWARD, COAST, BRAKE};
TIM_HandleTypeDef htim4;

/** @brief Set motor pwm and direction
 *
 * @param speed PWM duty cycle - 0 to 1000
 * @param dir 	Direction to set motor.
 */
void motorSet(int speed, enum Direction dir);


/** @brief Enable or disable motor
 *
 * @param enable 1 to enable 0 to disable.
 */
void motorEnable(int enable);


/** @brief Initialize motor.
 */
void motorInit();


#endif
