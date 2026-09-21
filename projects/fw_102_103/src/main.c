/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for fw_102_103
 *
 * @date   2025-08-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdlib.h>

/* Inter-component Headers */
#include "ads1115.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "queues.h"
#include "tasks.h"

/* Intra-component Headers */
#include "fw_102_103.h"

#define BLINKY_PERIOD_MS 1000U
#define ADS1115_SAMPLING_PERIOD_MS 1000U
#define ADS1115_READER_PERIOD_MS 500U
#define ADS1115_QUEUE_SIZE 5U

static GpioAddress blinky_gpio = {
  .port = GPIO_PORT_B,
  .pin = 3U,
};

static I2CSettings i2c_settings = {
  .scl = { .port = GPIO_PORT_B, .pin = 6U },
  .sda = { .port = GPIO_PORT_B, .pin = 7U },
  .speed = I2C_SPEED_STANDARD,
};

static GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 0U };
static ADS1115_Config ads1115_cfg = {
  .i2c_port = ADS1115_I2C_PORT,
  .i2c_addr = ADS1115_ADDR_GND,
  .ready_pin = &ready_pin,
};

static uint8_t ads1115_queue_storage[ADS1115_QUEUE_SIZE * sizeof(float)];
static Queue ads1115_data_queue = {
  .num_items = ADS1115_QUEUE_SIZE,
  .item_size = sizeof(float),
  .storage_buf = ads1115_queue_storage,
};

TASK(blinky, TASK_STACK_256) {
  while (true) {
    if (gpio_toggle_state(&blinky_gpio) == STATUS_CODE_OK) {
      LOG_DEBUG("blinky LED %s\n", gpio_get_state(&blinky_gpio) == GPIO_STATE_HIGH ? "ON" : "OFF");
    } else {
      LOG_DEBUG("blinky GPIO toggle failed\n");
    }
    delay_ms(BLINKY_PERIOD_MS);
  }
}

TASK(ads1115_writer, TASK_STACK_256) {
  while (true) {
    float voltage;
    StatusCode status = ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &voltage);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ADC read failed %d\n", status);
    } else if (queue_send(&ads1115_data_queue, &voltage, 0U) != STATUS_CODE_OK) {
      LOG_DEBUG("write to queue failed\n");
    } else {
      LOG_DEBUG("Writing to ADC queue %f V\n", (double)voltage);
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}

TASK(ads1115_reader, TASK_STACK_256) {
  while (true) {
    float voltage;
    if (queue_receive(&ads1115_data_queue, &voltage, 2U * ADS1115_SAMPLING_PERIOD_MS) == STATUS_CODE_OK) {
      LOG_DEBUG("Reading from ADC queue %f V\n", (double)voltage);
    } else {
      LOG_DEBUG("read from queue failed\n");
    }
    delay_ms(ADS1115_READER_PERIOD_MS);
  }
}

#if defined(MS_PLATFORM_X86)
TASK(ads1115_data_simulator, TASK_STACK_256) {
  /* This task simulates the I2C data for simulated off-target testing */

  unsigned int noise_rand_seed = 0xDEADBEEF;
  uint16_t simulated_voltage = 22400; /* 1.4V */

  while (true) {
    /* Simulate noise +- 500 */
    int16_t noise = (rand_r(&noise_rand_seed) % 1001) - 500;
    uint16_t noisy_voltage = simulated_voltage + noise;

    /* Consume simulated writes so the bounded TX queue does not fill */
    uint8_t transmitted;
    while (i2c_get_tx_data(ADS1115_I2C_PORT, &transmitted, sizeof(transmitted)) == STATUS_CODE_OK) {
    }
    /* Config then conversion register in the same byte order as the ADC */
    uint8_t data[] = { 0x44U, 0x83U, (uint8_t)(noisy_voltage >> 8), (uint8_t)noisy_voltage };
    if (i2c_set_rx_data(ADS1115_I2C_PORT, data, sizeof(data)) != STATUS_CODE_OK) {
      LOG_DEBUG("ADC simulator queue failed\n");
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}
#endif

int main(void) {
#if defined(MS_PLATFORM_X86)
  /* The off-target MCU stub returns INCOMPLETE without initializing GPIO */
  mcu_init();
  status_ok_or_return(gpio_init());
#else
  status_ok_or_return(mcu_init());
#endif
  status_ok_or_return(gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  status_ok_or_return(i2c_init(ADS1115_I2C_PORT, &i2c_settings));
  status_ok_or_return(ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin));

  /* Initialize printing module */
  log_init();

  /* Initialize RTOS tasks */
  status_ok_or_return(tasks_init());

  status_ok_or_return(queue_init(&ads1115_data_queue));
  status_ok_or_return(tasks_init_task(blinky, TASK_PRIORITY(2U), NULL));
  status_ok_or_return(tasks_init_task(ads1115_writer, TASK_PRIORITY(2U), NULL));
  status_ok_or_return(tasks_init_task(ads1115_reader, TASK_PRIORITY(1U), NULL));

#if defined(MS_PLATFORM_X86)
  status_ok_or_return(tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL));
#endif

  /* Start RTOS scheduler */
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
