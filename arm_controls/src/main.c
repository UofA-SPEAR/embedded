#include "stm32f1xx.h"
#include "stm32f1xx_nucleo.h"

#include "stm32f1xx_hal.h"


#define ARM_MATH_CM3
#include "arm_math.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "motor.h"
#include "pot.h"
#include "clocks.h"
#include "coms.h"

//////////// Needs to be set for each joint
int joint_id  = 0;
// per motor PID settings
#define PID_P 0x8FFF000F
#define PID_I 0
#define PID_D 0
// these bounds are needed, as not the potentiometers will not experience their full range
#define LOWER_POT_BOUND 0
#define UPPER_POT_BOUND 4096
int degree_to_pot_pos(int angle){
	return angle * (UPPER_POT_BOUND - LOWER_POT_BOUND) / 3600 + LOWER_POT_BOUND;
}
/////// Everything else can be made general

int16_t desiredPos; // where we want the pot to be

void setup(){
	HAL_Init();

	// init clock
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	SystemCoreClockUpdate();
	SystemClock_Config();
}

float doPID(arm_pid_instance_q31* pid){
	static q31_t pos, vel; // input position and output velocity. The arm math data structure for small high precision floating
	static float32_t position; // position is the value directly from the pot
	static float32_t velocity; // velocity is the number that goes directly to the motor

	position = (readPot() - desiredPos) / 4096.0;
	arm_float_to_q31(&position, &pos, 1);

	vel = arm_pid_q31(pid, pos);
	arm_q31_to_float(&vel, &velocity, 1);

	return velocity;
}


int main(void) {
	setup();


	motorInit();
	potInit();
	comInit();

	desiredPos = 2000;


	// setup PID
	arm_pid_instance_q31 pid;
	memset(&pid, 0, sizeof(arm_pid_instance_q31));
	pid.Kp = PID_P;
	pid.Ki = PID_I;
	pid.Kd = PID_D;
	arm_pid_init_q31(&pid, 1);

	// to hold the return value of the pid
	static float velocity;

	for (;;) {
		updateComs();
		velocity = doPID(&pid);

		setMotor(abs(velocity * 1000), velocity >= 0);
		HAL_Delay(100);

	}
}


