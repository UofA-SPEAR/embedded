#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"
#include "core_cm4.h"

#include "vnh5019.h"
#include "encoders.h"
#include "clocks.h"
#include "coms.h"
#include "flash_settings.h"

#define ARM_MATH_CM4
#include "arm_math.h"

// these bounds are needed, as not the potentiometers will not experience their full range
#define LOWER_POT_BOUND 0
#define UPPER_POT_BOUND 4096

#define BASE_NODE_ID 30

//////////// Needs to be set for each joint
int64_t actuator_id  = 0;
int16_t desiredPos; // where we want the pot to be

vnh5019_t motorA, motorB;
arm_pid_instance_f32 pid_A, pid_B;

uint32_t time_since_run = 0;

void setup(){
	HAL_Init();

	// init clock
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	SystemCoreClockUpdate();
	SystemClock_Config();
}

float doPID(arm_pid_instance_f32* pid){
	static float32_t position; // position is the value directly from the pot

	position = (potA_read() - desiredPos) / 4096.0;

	return arm_pid_f32(pid, position);
}

// Run PID and motor control
void run_motors() {
	if (saved_settings.motor[0].encoder.type == ENCODER_POTENTIOMETER) {
		uint32_t current_position = potA_read();

		arm_pid_f32(&pid_A, error);

	}

}

void motor_init() {
	motorA.digital.in_a = GPIO_PIN_4;
	motorA.digital.in_b = GPIO_PIN_3;
	motorA.digital.en_a = GPIO_PIN_6;
	motorA.digital.en_b = GPIO_PIN_5;
	motorA.digital.port = GPIOB;

	motorA.pwm.pin 		= GPIO_PIN_7;
	motorA.pwm.port 	= GPIOB;
	motorA.pwm.tim_af	= GPIO_AF2_TIM4;

	vnh5019_init(&motorA);
}

uint8_t read_node_id(void) {
	// Enable GPIO clock
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_14 |
			GPIO_PIN_13 | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	uint8_t node_id = BASE_NODE_ID;
	uint8_t tmp = 0;

	tmp =  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) << 3;
	tmp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) << 2;
	tmp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) << 1;
	tmp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);

	node_id += tmp;
	return node_id;
}

// To make a system reset, use NVIC_SystemReset()
int main(void) {
	uint8_t node_id = 0;
	setup();

	load_settings();
	motor_init();
	motor_enable(&motorA, 1);
	potInit();

	comInit();
	node_id = read_node_id();
	canardSetLocalNodeID(&m_canard_instance, node_id);

	desiredPos = 2000;


	// setup PID
	memset(&pid_A, 0, sizeof(arm_pid_instance_f32));
	pid_A.Kp = saved_settings.motor[0].pid.Kp;
	pid_A.Ki = saved_settings.motor[0].pid.Ki;
	pid_A.Kd = saved_settings.motor[0].pid.Kd;
	arm_pid_A_init_f32(&pid_A, 1);

	// to hold the return value of the pid_A
	static float velocity;

	for (;;) {

		velocity = doPID(&pid_A);

		motor_set(&motorA, abs(velocity * 1000), (velocity >= 0) ? FORWARD : REVERSE);
		HAL_Delay(100);
		publish_nodeStatus();
		tx_once();
		rx_once();
	}
}


