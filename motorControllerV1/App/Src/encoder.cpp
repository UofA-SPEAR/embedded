//TODO: work on the encoder
#include "encoder.hpp"
#include "hal.h"
#include "chprintf.h"
// #include <stdio.h>
#define QEI_DELAY TIME_MS2I(1)
static QEIConfig qeicfg = {
    QEI_MODE_QUADRATURE,
    QEI_BOTH_EDGES,
    QEI_DIRINV_FALSE,
};

static virtual_timer_t check_vt;

static void updateValue(void *arg) { 
    Quadrature* temp = static_cast<Quadrature*>(arg);
    chSysLockFromISR();
    long long val = qeiGetCountI(&QEID3);
    temp->set(val, QEI_DELAY);
    chVTSetI(&check_vt, QEI_DELAY, updateValue, temp);
    chSysUnlockFromISR();
}

void Potentiometer::init() {
    Encoder::init(potentiometer);
}

float Potentiometer::readAngular(uint16_t adcValue) 
{
    return 180.0 * static_cast<float>(adcValue) / 4096.0;
}

void Quadrature::set(uint16_t curr, sysinterval_t interval)
{
    pos += (long long)(curr - prev);
    speed = static_cast<float>(curr - prev)/static_cast<float>(interval);
    prev = curr;
}

float Quadrature::readAngular()
{
    return static_cast<float>(pos)/anglePerRev;
}

void Quadrature::init(int gearRatio)
{
    anglePerRev = 360.0/(static_cast<float>(gearRatio) * 4.0);
    Encoder::init(linear_actuator);
    palSetPadMode(GPIOB, GPIOB_PIN5, PAL_MODE_ALTERNATE(2));
    palSetPadMode(GPIOB, GPIOB_PIN4, PAL_MODE_ALTERNATE(2));    
    qeiStart(&QEID3, &qeicfg);
    chVTObjectInit(&check_vt);
    pos = 0;
    this->prev = 0;
    qeiEnable(&QEID3);
    chVTSet(&check_vt, QEI_DELAY, updateValue, this);
}

