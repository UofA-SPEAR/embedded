#include "main.h"

/**
 * TODO:
 * - CAN parameters
 * - Improve battery info reporting
 * - Add user button interface
 * - Add better external battery management
 */

#define MAX_CURRENT		50
#define MIN_VOLTAGE		3.4

#define NODE_ID_OFFSET	40

// Control flags
bool flag_take_measurement 	= false;
bool flag_publish_battery	= false;

/*** @brief General GPIO Initialization
 * 
 */
void gpio_init() {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* ------ Inputs ------ */
	// Node select
	GPIO_InitTypeDef gpio;
	gpio.Pin = NODE_SELECT_PIN0 | NODE_SELECT_PIN1 | NODE_SELECT_PIN2 | NODE_SELECT_PIN3;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &gpio);

	// User Button
	gpio.Pin = USR_BTN_PIN;
	gpio.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(USR_BTN_PORT, &gpio);

	// Analog Inputs
	gpio.Pin = CURRENT_MSR_PIN | VOLTAGE_MSR_PIN;
	gpio.Pull = GPIO_NOPULL;
	gpio.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(CURRENT_MSR_PORT, &gpio);

	/* ------- Outputs ------ */
	// Out Enable
	gpio.Pin = OUT_EN_PIN;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(OUT_EN_PORT, &gpio);

	// CAN Bus
	gpio.Pin = CAN_RX_PIN | CAN_TX_PIN;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate = GPIO_AF9_CAN;
	HAL_GPIO_Init(CAN_PORT, &gpio);
}

/*
 * Theoretically a working blinky example.
 * Untested though.
 */
int main(void) {
	adc_measurement_t measurement;
	uint16_t battery_status = 0;

	// Not sure if this is strictly necessary after a reset
	// It's probably not necessary but it's a single instruction
	// so whatever
	__enable_irq();

	// Initialise peripherals
	clocks_init();
	gpio_init();
	adc_init();
	timers_init();

	// Get node ID from node selection pins
	uint8_t node_id = HAL_GPIO_ReadPin(NODE_SELECT_PORT, NODE_SELECT_PIN0);
	node_id |= HAL_GPIO_ReadPin(NODE_SELECT_PORT, NODE_SELECT_PIN1) << 1;
	node_id |= HAL_GPIO_ReadPin(NODE_SELECT_PORT, NODE_SELECT_PIN2) << 2;
	node_id |= HAL_GPIO_ReadPin(NODE_SELECT_PORT, NODE_SELECT_PIN3) << 3;
	node_id += NODE_ID_OFFSET;
	coms_init(node_id);

	// Turn on battery
	HAL_GPIO_WritePin(OUT_EN_PORT, OUT_EN_PIN, 1);

	// Detect battery number of cells.
	// Can fail if battery is severely under- or over-voltage
	// Works in the range of 3.5V-4.2V
	measurement = adc_measure();
	//uint8_t num_cells = floor(measurement.bat_voltage / 3.5);


	for (;;) {
		if (flag_take_measurement) {
			measurement = adc_measure();

            /*
			if (measurement.current > MAX_CURRENT) {
				// Current overload, shut off battery.
				battery_status = UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_OVERLOAD;
				HAL_GPIO_WritePin(OUT_EN_PORT, OUT_EN_PIN, 0);
			}

			if ((measurement.bat_voltage / num_cells) < MIN_VOLTAGE) {
				// Battery voltage too low, shut off battery
				battery_status = UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_OVERLOAD;
				HAL_GPIO_WritePin(OUT_EN_PORT, OUT_EN_PIN, 0);
			}
            */

			// Handle overcurrent/undervoltage here
			flag_take_measurement = false;
		}

		if (flag_publish_battery) {
			publish_batteryInfo(measurement, battery_status);

			flag_publish_battery = false;
		}

		// Handle CAN messages in downtime
		tx_once();
		rx_once();
	}
}
