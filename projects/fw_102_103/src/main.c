/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for fw_102_103
 *
 * @date   2025-08-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "queues.h"

/* FW102 Onboarding module */
#include "ads1115.h"

/* Intra-component Headers */
#include "fw_102_103.h"

static GpioAddress blinky_gpio = {
  /* --------------------- TODO: FW102 --------------------- */
};

static I2CSettings i2c_settings = {
  /* --------------------- TODO: FW102 --------------------- */
};

static ADS1115_Config ads1115_cfg = {
  /* --------------------- TODO: FW102 --------------------- */
};

static Queue ads1115_data_queue = {
  /* --------------------- TODO: FW103 --------------------- */
  /* Hint: You will need to define an array to be used as the storage */
};

TASK(blinky, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will blinky an LED and log the state of the pin */
  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_writer, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the ADS1115 external chip and push its data to a queue */
  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_reader, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the queue containing ADS1115 data and process it */
  /* --------------------- FW103 END --------------------- */
}

#if defined(MS_PLATFORM_X86)
TASK(ads1115_data_simulator, TASK_STACK_256) {
  /* This task simulates the I2C data for simulated off-target testing */

  uint16_t simulated_voltage = 14414; /* Rouguhly 1.8V */

  while (true) {
    i2c_set_rx_data(ADS1115_I2C_PORT, (uint8_t *)&simulated_voltage, sizeof(simulated_voltage));
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}
#endif

int main() {
  /* Initialize printing module */
  log_init();

  /* Initialize RTOS tasks */
  tasks_init();

#if defined(MS_PLATFORM_X86)
  tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL);
#endif

  /* Start RTOS scheduler */
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
