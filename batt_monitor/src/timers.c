#include "timers.h"

TIM_HandleTypeDef update_timer;

void timers_init(void) {
    update_timer.Instance           = TIM4;
    update_timer.Init.Prescaler     = 63999; // 1kHz timer
    update_timer.Init.Period        = 10;
    update_timer.Init.CounterMode   = TIM_COUNTERMODE_UP;
    update_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    HAL_TIM_Base_Init(&update_timer);

    NVIC_SetPriority(TIM4_IRQn, 2);
    NVIC_EnableIRQ(TIM4_IRQn);

    HAL_TIM_Base_Start_IT(&update_timer);
}

void TIM4_IRQHandler(void) {
    static uint8_t counter = 0;

    flag_take_measurement = true;

    // Taking measurements at 100Hz, but only want to publish at 1Hz
    if (++counter == 10) {
        flag_publish_battery = true;
        counter = 0;
    }
}