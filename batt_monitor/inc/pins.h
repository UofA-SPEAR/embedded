#ifndef PINS_H_
#define PINS_H_

#define USR_BTN_PIN         GPIO_PIN_1
#define USR_BTN_PORT        GPIOA

#define OUT_EN_PIN          GPIO_PIN_15
#define OUT_EN_PORT         GPIOA

#define CURRENT_MSR_PIN     GPIO_PIN_0
#define CURRENT_MSR_PORT    GPIOA
#define VOLTAGE_MSR_PIN     GPIO_PIN_2
#define VOLTAGE_MSR_PORT    GPIOA

#define NODE_SELECT_PIN0    GPIO_PIN_6
#define NODE_SELECT_PIN1    GPIO_PIN_7
#define NODE_SELECT_PIN2    GPIO_PIN_8
#define NODE_SELECT_PIN3    GPIO_PIN_9
#define NODE_SELECT_PORT    GPIOB

// TODO: Check the order of RX/TX
#define CAN_RX_PIN          GPIO_PIN_11
#define CAN_TX_PIN          GPIO_PIN_12
#define CAN_PORT            GPIOA

#endif