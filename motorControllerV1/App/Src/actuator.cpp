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

constexpr uint32_t LOOP_FREQ = 100; // Update at 10Hz
constexpr uint32_t LOOP_PERIOD = 1000/LOOP_FREQ; // Loop period (ms)
constexpr uint32_t RESPONSE_TIME = 100; // Response time (in ms)
constexpr uint32_t DIFF_PER_PERIOD = 10000 / (RESPONSE_TIME / LOOP_PERIOD);

constexpr uint32_t TIMEOUT_MS = 5000;

static uint32_t get_linear_target_position(float command_angle) {
  float desired_length;
  int32_t position;
  // TODO add an angle offset

  constexpr float support_length = 2;
  constexpr float arm_length = 2;

  constexpr float length_min = 0;
  constexpr float length_max = 2;
  
  constexpr uint32_t encoder_min = 0;
  constexpr uint32_t encoder_max = 4096;

  // Comes from cosine law
  // c^2 = a^2 + b^2 - 2ab*cos(C)
  desired_length = sqrt(
      pow(support_length, 2) + pow(arm_length, 2) -
      (2 * (support_length) * (arm_length) * cos(command_angle)));

  // TODO set nodestatus
  if (desired_length < length_min) {
    //can_set_node_status(UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING,
    //                    UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL);
    desired_length = length_min;
  } else if (desired_length > length_max) {
    //can_set_node_status(UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING,
    //                    UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL);
    desired_length = length_max;
  } else {
    //can_set_node_status(UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK,
    //                    UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL);
  }

  // These are checked to be positive in check_settings()
  uint32_t encoder_range = encoder_max - encoder_min;
  float linear_range = length_max - length_min;

  // set position properly
  position =
      // fit length into encoder range
      (desired_length - length_min) *
          // Convert from length range into the encoder range
          (encoder_range / linear_range) +
      // Add the minimum encoder value
      encoder_min;

  return position;
}

const ADCConversionGroup adcgrpcfg1 = {
    FALSE,
    1,
    NULL,
    NULL,
    ADC_CFGR_CONT,   /* CFGR    */
    .smpr         = {
    ADC_SMPR1_SMP_AN0(ADC_SMPR_SMP_61P5),
    0
    },
    .sqr          = {
      ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1),
      0,
      0,
      0
    }
};

static int32_t pot_read(void) {
  adcsample_t buf[4];
  adcsample_t res = 0;
  adcConvert(&ADCD1, &adcgrpcfg1, buf, 4);
  for (int i = 0; i < 4; i++) {
    res += buf[i];
  }
  res /= 4;
  return res;
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

    systime_t next_loop = chVTGetSystemTime();
    int32_t current_effort = 0;
    int32_t target_effort = 0;

    systime_t timeout = chVTGetSystemTime() + TIME_MS2I(TIMEOUT_MS);

    // TODO correct default target
    uint32_t target_position = 0; // Target *encoder* position, we translate provided angle into encoder value

    // Set up ADC
    palSetPad(GPIOB, 1);
    palSetPad(GPIOB, 0);
    palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG);
    adcStart(&ADCD1, NULL);

    while (true) {
        msg_t status = chFifoReceiveObjectTimeout(&servoMsg, (void**)&cmd, TIME_US2I(100));

        // TODO timeout
        if(status == MSG_OK) {
            // Received command, update target
            memcpy(&cmdStorage, cmd, sizeof(uavcan::equipment::actuator::Command));
            chFifoReturnObject(&servoMsg, cmd);
            if (cmdStorage.actuator_id == data.get_setting_int("actuator_id")) {
                if (data.get_setting_int("encoder_type") == 0 && cmdStorage.command_type == cmdStorage.COMMAND_TYPE_SPEED) {
                    target_effort = cmdStorage.command_value * 1000;
                    timeout = chVTGetSystemTime() + TIME_MS2I(TIMEOUT_MS);
                } else if (data.get_setting_int("encoder_type") == 1 && cmdStorage.command_type == cmdStorage.COMMAND_TYPE_POSITION) {
                    target_position = get_linear_target_position(cmdStorage.command_value);
                }
            }
        } else {
            systime_t now = chVTGetSystemTime();
            if (now > timeout) {
                target_effort = 0;
            }

            // Skip if it isn't time to update
            if (chVTGetSystemTime() < next_loop) {
                continue;
            }

            // Set time for next update
            next_loop = next_loop + TIME_MS2I(LOOP_PERIOD);

            if (data.get_setting_int("encoder_type") == 0) {
                // TODO not hacky shit
                short diff = target_effort - current_effort;
                if (diff > 0) {
                    current_effort += ((diff < DIFF_PER_PERIOD) ? diff : DIFF_PER_PERIOD);
                } else if (diff < 0) {
                    current_effort += ((abs(diff) < DIFF_PER_PERIOD) ? diff : -DIFF_PER_PERIOD);
                }

                drv8701_set(current_effort, &gpiover1_0);
            } else if (data.get_setting_int("encoder_type") == 1) {
                int32_t error = target_position - pot_read();

                float effort = arm_pid_f32(&pidHolder, (float) error / 4096.0);
                drv8701_set(effort * 10000, &gpiover1_0);
            }
        }

        //if(status == MSG_OK) {
        //    memcpy(&cmdStorage, cmd, sizeof(uavcan::equipment::actuator::Command));
        //    chFifoReturnObject(&servoMsg, cmd);
        //    isTimeout = false;
        //}
        //else {
        //    if(TIME_I2MS(chVTGetSystemTime()) >= countdown) {
        //        isTimeout = true;
        //    }
        //}
        //statusMsg.actuator_id = data.get_setting_int("actuator_id");
        //if(counter >= dest) {
        //    dest = counter + 1000;
        //    void *msg = chFifoTakeObjectTimeout(&servoStatus, TIME_MS2I(500));
        //    if(msg != nullptr) {
        //        memcpy(msg, &statusMsg, sizeof(uavcan::equipment::actuator::Status));
        //        chFifoSendObject(&servoStatus, msg);
        //    }
        //}
        //bool isEnabledCurr = data.get_setting_bool("enabled");
        //if(isEnabledBefore != isEnabledCurr) {
        //    isEnabledBefore = isEnabledCurr;
        //    if(isEnabledCurr) {
        //        data.lock();
        //    }
        //    else {
        //        data.unlock();
        //    }
        //}
        //// Timeout for this only occured with no type and speed type sensor
        //if(!isTimeout) {
        //    if(data.get_setting_int("encoder.type") == 0) {
        //        short result = cmdStorage.command_value * 1000;
        //        if(data.get_setting_bool("reversed")) result *= -1;
        //        chprintf((BaseSequentialStream*)&SD2, "Received Command is %d running at %d\r\n", cmdStorage.command_type, result);
        //        if(result != current_pos) {
        //            set_mot(result, current_pos, 1);
        //            current_pos = result;
        //        }
        //    }
        //    else {
        //        if (cmdStorage.command_type == uavcan::equipment::actuator::Command::COMMAND_TYPE_POSITION) {
        //            float speed = getSpeed();
        //            float target = arm_pid_f32(&pidHolder, cmdStorage.command_value - speed);
        //            drv8701_set((short)target, &gpiover1_0); 
        //        }
        //    }
        //}

        //else if(cmdStorage.command_type != uavcan::equipment::actuator::Command::COMMAND_TYPE_POSITION) {
        //    cmdStorage.command_value = 0;
        //    short result = 0;
        //    if(result != current_pos) {
        //        set_mot(result, current_pos, 1);
        //        current_pos = result;
        //    }
        //}

        //if(cmdStorage.command_type == uavcan::equipment::actuator::Command::COMMAND_TYPE_POSITION) {
        //    float angle = getAngle();
        //    float target = arm_pid_f32(&pidHolder, cmdStorage.command_value - angle);
        //    drv8701_set((short)target, &gpiover1_0);                
        //}
        //counter = TIME_I2MS(chVTGetSystemTime());
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