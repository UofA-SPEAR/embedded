/* VNH5019 motor controller library.
 *
 * Assumptions:
 * - Using STM32F303
 * - Only using one timer
 * - Digital pins are all on one GPIO port
 * - Only using one ADC???
 */

#ifndef _VNH5019_H_
#define _VNH5019_H_

#define MOTOR_PWM_PIN 	GPIO_PIN_7
#define MOTOR_INA_PIN 	GPIO_PIN_4
#define MOTOR_INB_PIN 	GPIO_PIN_3
#define MOTOR_ENA_PIN 	GPIO_PIN_6
#define MOTOR_ENB_PIN 	GPIO_PIN_5

#define MOTOR_PORT		GPIOB
// Change these to change the timer instance.
#define VNH5019_TIM_INSTANCE		TIM4
#define VNH5019_TIM_CLK_ENABLE() 	__HAL_RCC_TIM4_CLK_ENABLE()

enum Direction {REVERSE, FORWARD, COAST, BRAKE};

TIM_HandleTypeDef htim4;

typedef struct {
	struct {
		uint16_t in_a;
		uint16_t in_b;
		uint16_t en_a;
		uint16_t en_b;
		GPIO_TypeDef* port;
	} digital;

	struct {
		uint16_t pin;
		GPIO_TypeDef* port;
		uint16_t tim_af;
		uint16_t tim_ch;
	} pwm;

	// TODO Add current sense stuff here

} vnh5019_t;




/** @brief Set motor pwm and direction
 *
 * @param speed PWM duty cycle - 0 to 1000
 * @param dir 	Direction to set motor.
 */
void motorSet(int speed, enum Direction dir);

void motorEnable(int enable);

/** @brief Initialize motor.
 */
void vnh5019_init(vnh5019_t* settings);


#endif
