#include "encoders.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

static void encoders_gpio_init() {
    GPIO_InitTypeDef gpio;

    // Configure for timer 2, PA0 and PA1
    gpio.Pin        = GPIO_PIN_0 | GPIO_PIN_1;
    gpio.Mode       = GPIO_MODE_AF_PP;
    gpio.Speed      = GPIO_SPEED_FREQ_HIGH; 
    gpio.Pull       = GPIO_PULLUP;
    gpio.Alternate  = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &gpio);

    // Configure for timer 1, PA8 and PA9
    gpio.Pin        = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Alternate  = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &gpio);
}

void encoders_init() {
    /*
     * Here we need to set up both TIM1 and TIM2 in encoder mode.
     * TIM2 will be motor 1, TIM1 will be motor 2    
     */

    encoders_gpio_init();

    htim1.Instance = TIM1;
    htim2.Instance = TIM2;

    // Base timer settings
    htim1.Init.Prescaler = 0;
    htim1.Init.Period = UINT16_MAX;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    // Both timers are configured the same
    htim2.Init = htim1.Init;

    TIM_Encoder_InitTypeDef encoder;
    encoder.EncoderMode     = TIM_ENCODERMODE_TI12;
    encoder.IC1Polarity     = TIM_ICPOLARITY_BOTHEDGE;
    encoder.IC1Selection    = TIM_ICSELECTION_DIRECTTI;
    encoder.IC1Filter       = 0;
    encoder.IC1Prescaler    = TIM_ICPSC_DIV1;
    encoder.IC2Polarity     = TIM_ICPOLARITY_BOTHEDGE;
    encoder.IC2Selection    = TIM_ICSELECTION_DIRECTTI;
    encoder.IC2Filter       = 0;
    encoder.IC1Prescaler    = TIM_ICPSC_DIV1;

    // Configure both timers the same
    HAL_TIM_Encoder_Init(&htim1, &encoder);
    HAL_TIM_Encoder_Init(&htim2, &encoder);

    // Start counter in middle so we can go negative initially
    TIM2->CNT = UINT16_MAX / 2;
    TIM1->CNT = UINT16_MAX / 2;

    // Start in encoder mode

    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
}