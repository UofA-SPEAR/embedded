/* VNH5019 motor controller library.
 *
 * Assumptions:
 * - Using STM32F303
 * - Only using one timer
 * - Digital pins are all on one GPIO port
 *
 * To Configure:
 * - Build up a vnh5019_t variable with the appropriate gpio settings
 * - call vnh5019_init() with you settings struct
 * - you should be done
 */

#ifndef _VNH5019_H_
#define _VNH5019_H_

#include "main.h"


// Change these to change the timer instance.
#define VNH5019_TIM_INSTANCE		TIM4
#define VNH5019_TIM_CLK_ENABLE() 	__HAL_RCC_TIM4_CLK_ENABLE()

enum Direction {REVERSE, FORWARD, COAST, BRAKE};

TIM_HandleTypeDef h_timer;

struct pin_info {
	uint32_t pin;
	GPIO_TypeDef* port;
};

typedef struct {
	struct {
		struct pin_info in_a;
		struct pin_info in_b;
		struct pin_info en_a;
		struct pin_info en_b;
	} digital;

	struct {
		uint16_t pin;
		GPIO_TypeDef* port;
		TIM_TypeDef* tim_instance;
		uint16_t tim_af;
		uint16_t tim_ch;
	} pwm;

	// I think I'll keep current sensing external.
	// It's a bit too complicated to do in a generic function
	// and it isn't necessary to use the drivers.
} vnh5019_t;




/** @brief Set motor pwm and direction
 *
 * @param motor pointer to motor instance
 * @param speed PWM duty cycle - 0 to 1000
 * @param dir 	Direction to set motor.
 */
void vnh5019_set(vnh5019_t* motor, int16_t speed);

/** @brief Initialize motor.
 *
 * @param settings Pointer to motor config
 */
void vnh5019_init(vnh5019_t* settings);


#endif
