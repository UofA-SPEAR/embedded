#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"

#include "motor.h"
#include "pot.h"
#include "clocks.h"
#include "coms.h"

#define ARM_MATH_CM4
#include "arm_math.h"

// Initial motor PID settings.
// Eventually we will actually set stuff in flash.
#define PID_P 0x8FFF000F
#define PID_I 0
#define PID_D 0

// these bounds are needed, as not the potentiometers will not experience their full range
#define LOWER_POT_BOUND 0
#define UPPER_POT_BOUND 4096

//////////// Needs to be set for each joint
int64_t actuator_id  = 0;
int16_t desiredPos; // where we want the pot to be

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

	position = (readPot() - desiredPos) / 4096.0;

	return arm_pid_f32(pid, position);
}


int main(void) {
	setup();

	motorInit();
	motorEnable(1);
	potInit();
	comInit();
	publish_nodeStatus();

	desiredPos = 2000;


	// setup PID
	arm_pid_instance_f32 pid;
	memset(&pid, 0, sizeof(arm_pid_instance_f32));
	pid.Kp = PID_P;
	pid.Ki = PID_I;
	pid.Kd = PID_D;
	arm_pid_init_f32(&pid, 1);

	// to hold the return value of the pid
	static float velocity;

	for (;;) {
		velocity = doPID(&pid);

		motorSet(abs(velocity * 1000), (velocity >= 0) ? FORWARD : REVERSE);
		HAL_Delay(100);
		publish_nodeStatus();

	}
}


