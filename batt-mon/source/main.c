#include "ch.h"
#include "hal.h"

#include "core_cm4.h"

#include "main.h"

#define O1HEAP_SIZE 1024
/**
 * @brief Runs motor control code once every MOTOR_CONTROL_PERIOD ms.
 */
static THD_WORKING_AREA(RunMotorWorkingArea, 1024);
static THD_FUNCTION(RunMotor, arg)
{

}

static THD_WORKING_AREA(HeartbeatWorkingArea, 512);
static THD_FUNCTION(Heartbeat, arg)
{
	//TODO

}

// To make a system reset, use NVIC_SystemReset()
int main(void) {
	uint8_t node_id = 0;
	halInit();
	chSysInit();
	chCoreAllocAligned(O1HEAP_SIZE, 16); // TODO: import o1heap to git repo and new libcanard system

}

// Simply needs to be defined somewhere
