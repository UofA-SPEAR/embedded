/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#define MOTORA_KP 0.3
#define MOTORB_KP 0.3

#define MOTORA_REVERSED 0
#define MOTORB_REVERSED 0

#define ENCODERA_REVERSED 0
#define ENCODERB_REVERSED 0

// Motor encoder counts per revolution
#define MOTOR_CPR				100
// Circumference of motor, in metres
#define MOTOR_CIRCUMFERENCE		(PI * 0.3)


#define GET_LINEAR_DISTANCE(new, old)	(new - old) / MOTOR_CPR * MOTOR_CIRCUMFERENCE

#include "main.h"

// Speed targets for motors
float motorA_speed = 0;
float motorB_speed = 0;

uint32_t motor_timeout;

UART_HandleTypeDef huart;

bool motor_handle_flag = false;

void serial_write8(uint8_t data) {
	// Write a byte out onto UART
	HAL_UART_Transmit(&huart, &data, 1, 1000);
}

void uart_init() {
	huart.Instance 			= USART3;
	huart.Init.BaudRate		= 9600; // Should this be a different value?
	huart.Init.WordLength 	= UART_WORDLENGTH_8B;
	huart.Init.StopBits 	= UART_STOPBITS_1;
	huart.Init.Parity		= UART_PARITY_NONE;
	huart.Init.Mode			= UART_MODE_TX;
	huart.Init.HwFlowCtl	= UART_HWCONTROL_NONE;
	HAL_UART_Init(&huart);
}

// Using USART3, on PB10
void HAL_UART_MspInit(UART_HandleTypeDef* instance) {
	__HAL_RCC_USART3_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_10;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
	gpio.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOB, &gpio);
}

void motors_init() {
	saberA.write8 = serial_write8;
	saberA.address = 128;
	saberA.deadband = 0;
	saberA.timeout = 5;
	saberA.min_voltage = 16;
	saberA.max_voltage = 30;
	saberA.ramp_setting = 30;

	saberB.write8 = serial_write8;
	saberB.address = 129;
	saberB.deadband = 0;
	saberB.timeout = 5;
	saberB.min_voltage = 16;
	saberB.max_voltage = 30;
	saberB.ramp_setting = 30;

	sabertooth_init(&saberA);
	sabertooth_init(&saberA);
}

int main(void)
{
	uint16_t motorA_last_pos;
	uint16_t motorB_last_pos;

	HAL_Init();

	clocks_init();
	uart_init();
	libcanard_init();
	motors_init();
	encoders_init();

	motorA_last_pos = TIM2->CNT;
	motorB_last_pos = TIM1->CNT;

	// Start the timeout thing
	motor_timeout = HAL_GetTick();

	// Setup PID
	arm_pid_instance_f32 pidA, pidB;

	memset(&pidA, 0, sizeof(arm_pid_instance_f32));
	memset(&pidB, 0, sizeof(arm_pid_instance_f32));

	pidA.Kp = MOTORA_KP;
	pidA.Ki = 0;
	pidA.Kd = 0;
	pidB.Kp = MOTORB_KP;
	pidB.Ki = 0;
	pidB.Kd = 0;

	arm_pid_init_f32(&pidA, 1);
	arm_pid_init_f32(&pidB, 1);


	for(;;) {
		static uint32_t last_thing = 0;
		//static uint32_t pid_timer = 0;
		int8_t motorA_out_int = 0;
		int8_t motorB_out_int = 0;

		handle_frame(); // literally nothing else to do
		tx_once();

		if ((HAL_GetTick() - last_thing) > 1000) {
			last_thing = HAL_GetTick();

			coms_send_NodeStatus(UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK,
				UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL, 0);

		}

		// 10 Hz for now
		// Check PID stuff
		if (motor_handle_flag) {
			int32_t motorA_cur_pos, motorB_cur_pos;

			//pid_timer = HAL_GetTick();

			motorA_cur_pos = TIM2->CNT;
			motorB_cur_pos = TIM1->CNT;

			#if ENCODERA_REVERSED
			// Convert encoder count to linear distance
			float motorA_distance = GET_LINEAR_DISTANCE(motorA_last_pos, motorA_cur_pos);
			#else 
			float motorA_distance = GET_LINEAR_DISTANCE(motorA_cur_pos, motorA_last_pos);
			#endif 

			#if ENCODERB_REVERSED
			float motorB_distance = GET_LINEAR_DISTANCE(motorB_last_pos, motorB_cur_pos);
			#else
			float motorB_distance = GET_LINEAR_DISTANCE(motorB_cur_pos, motorB_last_pos);
			#endif

			// Get the error for PID to do it's magic
			float motorA_error = motorA_speed - motorA_distance;
			float motorB_error = motorB_speed - motorB_distance;

			// Run PID
			float motorA_out = arm_pid_f32(&pidA, motorA_error);
			float motorB_out = arm_pid_f32(&pidB, motorB_error);

			// Get the new output value
			motorA_out = motorA_out * 127 + motorA_out_int;
			motorB_out = motorB_out * 127 + motorB_out_int;;

			motorA_out_int = motorA_out;
			motorB_out_int = motorB_out;

			// Cap the output so we don't overflow
			if (motorA_out > 127) { motorA_out_int = 127; }
			if (motorA_out < -127) { motorA_out_int = -127; }
			if (motorB_out > 127) { motorB_out_int = 127; }
			if (motorB_out < -127) { motorB_out_int = -127; }

			// Send outputs to motors
			#if MOTORA_REVERSED
			sabertooth_set_motor(&saberA, 0, -motorA_out_int);
			#else
			sabertooth_set_motor(&saberA, 0, motorA_out_int);
			#endif

			#if MOTORB_REVERSED
			sabertooth_set_motor(&saberA, 1, -motorB_out_int);
			#else
			sabertooth_set_motor(&saberA, 1, motorB_out_int);
			#endif

			// Motor A is right back, Motor B is left back
			motorB_distance = motorB_speed / 10;
			motorA_distance = motorA_speed /10;
			coms_odom_broadcast(0, motorB_distance);
			coms_odom_broadcast(1, motorB_distance);
			coms_odom_broadcast(2, motorA_distance);
			coms_odom_broadcast(3, motorA_distance);

			motor_handle_flag = false;
		}

		if ((HAL_GetTick() - motor_timeout) > MOTOR_TIMEOUT_MS) {
			sabertooth_set_motor(&saberA, 0, 0);
			sabertooth_set_motor(&saberA, 1, 0);
			motor_timeout = HAL_GetTick();
		}

	}
}
