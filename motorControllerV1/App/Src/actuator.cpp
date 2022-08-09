#include "actuator.hpp"
#include "paramServer.hpp"
#include "hal.h"
#include <uavcan/equipment/actuator/ArrayCommand.hpp>
#include <uavcan/equipment/actuator/Status.hpp>
#include "coms.hpp"
#include "drv8701.hpp"
#include "arm_math.h"
#include "chprintf.h"
#include "encoder.hpp"

static const ADCConversionGroup adcCfg = {
  .circular     = true,
  .num_channels = 2,
  .end_cb       = NULL,
  .error_cb     = NULL,
  .cfgr         = ADC_CFGR_CONT,
  .tr1          = ADC_TR_DISABLED,
  .tr2          = ADC_TR_DISABLED,
  .tr3          = ADC_TR_DISABLED,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr         = {
    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_61P5) | ADC_SMPR1_SMP_AN2(ADC_SMPR_SMP_61P5),
    0
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | ADC_SQR1_SQ1_N(ADC_CHANNEL_IN2),
    0,
    0,
    0
  }
};
static adcsample_t sample[2];
static arm_pid_instance_f32 pidHolder;
static Quadrature qeiEnc;
static Potentiometer potEnc;
static objects_fifo_t servoMsg;

static uavcan::equipment::actuator::Command servoBuffer[10];
static msg_t servoMsgBuffer[10];


static objects_fifo_t servoStatus;
static uavcan::equipment::actuator::Status servoStatusBuff[5];
static msg_t servoStatusMsgBuff[5];

float getAngle();
float getSpeed();

static void set_mot(short target, short current, uint8_t ms_per_step) 
{
    while(target != current) {
        if(current > target) {
            current--;
        }
        else if(current < target) {
            current++;
        }
        drv8701_set(current, &gpiover1_0);
        chThdSleepMilliseconds(ms_per_step);
    }
    if(current == 0) {
        drv8701_stop();
    }
}
// Working Thread
static THD_WORKING_AREA(servoTestWorkingArea, 2048);
static THD_FUNCTION(servoTestFn, arg) {
    uavcan::equipment::actuator::Command *cmd;
    uavcan::equipment::actuator::Command cmdStorage;
	systime_t counter;
    uavcan::equipment::actuator::Status statusMsg;
    short current_pos = 0; // for non-sensor
    (void)arg;
    pidHolder.Kd = data.get_setting_real("pid.Kd");
    pidHolder.Ki = data.get_setting_real("pid.Ki");
    pidHolder.Kp = data.get_setting_real("pid.Kp");
    arm_pid_init_f32(&pidHolder, 1);
    qeiEnc.init(data.get_setting_int("quadrature.gear"));
    potEnc.init();
    bool isEnabledBefore = data.get_setting_bool("enabled");
    counter = TIME_I2MS(chVTGetSystemTime());
    systime_t dest = counter + 1000;
    systime_t countdown = counter + 500;
    bool isTimeout = false;
    drv8701_init(&gpiover1_0);
    drv8701_set_current(data.get_setting_real("current"));
    while (true) {
        msg_t status = chFifoReceiveObjectTimeout(&servoMsg, (void**)&cmd, TIME_MS2I(10));
        if(status == MSG_OK) {
            memcpy(&cmdStorage, cmd, sizeof(uavcan::equipment::actuator::Command));
            chFifoReturnObject(&servoMsg, cmd);
            countdown = TIME_I2MS(chVTGetSystemTime()) + 500;
            isTimeout = false;
        }
        else {
            if(TIME_I2MS(chVTGetSystemTime()) >= countdown) {
                isTimeout = true;
            }
        }
        statusMsg.actuator_id = data.get_setting_int("actuator_id");
        if(counter >= dest) {
            dest = counter + 1000;
            void *msg = chFifoTakeObjectTimeout(&servoStatus, TIME_MS2I(500));
            if(msg != nullptr) {
                memcpy(msg, &statusMsg, sizeof(uavcan::equipment::actuator::Status));
                chFifoSendObject(&servoStatus, msg);
            }
        }
        bool isEnabledCurr = data.get_setting_bool("enabled");
        if(isEnabledBefore != isEnabledCurr) {
            isEnabledBefore = isEnabledCurr;
            if(isEnabledCurr) {
                data.lock();
            }
            else {
                data.unlock();
            }
        }
        // Timeout for this only occured with no type and speed type sensor
        if(!isTimeout) {
            if(data.get_setting_int("encoder.type") == 0) {
                short result = cmdStorage.command_value * 1000;
                if(data.get_setting_bool("reversed")) result *= -1;
                chprintf((BaseSequentialStream*)&SD2, "Received Command is %d running at %d\r\n", cmdStorage.command_type, result);
                if(result != current_pos) {
                    set_mot(result, current_pos, 1);
                    current_pos = result;
                }
            }
            else {
                if (cmdStorage.command_type == uavcan::equipment::actuator::Command::COMMAND_TYPE_POSITION) {
                    float speed = getSpeed();
                    float target = arm_pid_f32(&pidHolder, cmdStorage.command_value - speed);
                    drv8701_set((short)target, &gpiover1_0); 
                }
            }
        }

        else if(cmdStorage.command_type != uavcan::equipment::actuator::Command::COMMAND_TYPE_POSITION) {
            cmdStorage.command_value = 0;
            short result = 0;
            if(result != current_pos) {
                set_mot(result, current_pos, 1);
                current_pos = result;
            }
        }

        if(cmdStorage.command_type == uavcan::equipment::actuator::Command::COMMAND_TYPE_POSITION) {
            float angle = getAngle();
            float target = arm_pid_f32(&pidHolder, cmdStorage.command_value - angle);
            drv8701_set((short)target, &gpiover1_0);                
        }
        counter = TIME_I2MS(chVTGetSystemTime());
    }
}

float getAngle()
{
    float angle = 0;
    switch (data.get_setting_int("encoder.type")) {
        case 1:
            angle = qeiEnc.readAngular();
            break;
        case 2:
            adcConvert(&ADCD1, &adcCfg, sample, 2);
            angle = potEnc.readAngular(sample[1]);
            break;
        default:
            break;
    }
    return angle;
}

float getSpeed()
{
    float speed = 0;
    switch (data.get_setting_int("encoder.type")) {
        case 1:
            speed = qeiEnc.readSpeed();
            break;
        default:
            break;
    }
    return speed;
}

uavcan::Subscriber<uavcan::equipment::actuator::ArrayCommand> *actuatorSub;
uavcan::Publisher<uavcan::equipment::actuator::Status> *actuatorStatPub;

void motorCheck(void) 
{
    uavcan::equipment::actuator::Status temp;
    uavcan::equipment::actuator::Status *msg;
    msg_t stat = chFifoReceiveObjectTimeout(&servoStatus, (void**)&msg, TIME_MS2I(1));
    if(stat == MSG_OK) {
        memcpy(&temp, msg, sizeof(uavcan::equipment::actuator::Status));
        chFifoReturnObject(&servoStatus, msg);
        actuatorStatPub->broadcast(temp);
    }
}

void motorInit(void)
{
    palSetPadMode(GPIOB, GPIOB_PIN0, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOB, GPIOB_PIN1, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, GPIOA_PIN0, PAL_MODE_INPUT_ANALOG);
    palSetPadMode(GPIOA, GPIOA_PIN1, PAL_MODE_INPUT_ANALOG);
    palSetPad(GPIOB, GPIOB_PIN0);
    palSetPad(GPIOB, GPIOB_PIN1);
    adcStart(&ADCD1, NULL);
    chFifoObjectInit(&servoMsg, sizeof(uavcan::equipment::actuator::Command), 10, (void*)servoBuffer, servoMsgBuffer);
    chFifoObjectInit(&servoStatus, sizeof(uavcan::equipment::actuator::Status), 5, (void*)servoStatusBuff, servoStatusMsgBuff);
    auto &node = getNode();
    actuatorSub = new uavcan::Subscriber<uavcan::equipment::actuator::ArrayCommand>(node);
    int result = actuatorSub->start([&](const uavcan::ReceivedDataStructure<uavcan::equipment::actuator::ArrayCommand> &msg)
    {
        int actuator_id1 = data.get_setting_int("actuator_id");
        for(auto i : msg.commands) {
            if(i.actuator_id == actuator_id1) {
                void *msgT = chFifoTakeObjectTimeout(&servoMsg, TIME_INFINITE);
                if(msgT != NULL) {
                    memcpy(msgT, &i, sizeof(uavcan::equipment::actuator::Command));
                    chFifoSendObject(&servoMsg, msgT);
                }   
            }
        }
    });
    chDbgAssert(result >= 0, "Error activationg actuator");
    actuatorStatPub = new uavcan::Publisher<uavcan::equipment::actuator::Status>(node);
    actuatorStatPub->init();

    (void) chThdCreateStatic(servoTestWorkingArea, sizeof(servoTestWorkingArea),
                            NORMALPRIO, servoTestFn, NULL);
}