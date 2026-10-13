/************************************************************************************************
 * @file    main.c
 *
 * @brief   Main
 *
 * @date    2026-10-13
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "ads1115.h"
#include "delay.h"
#include "fw_102_103.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "queues.h"

/* Intra-component Headers */

#define BLINKY_PERIOD_MS 1000U
#define ADS1115_SAMPLING_PERIOD_MS 1000U
#define ADS1115_READER_TIMEOUT_MS 2000U
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

static GpioAddress ready_pin = {
  .port = GPIO_PORT_B,
  .pin = 0U,
};

static ADS1115_Config ads1115_cfg = {
  .i2c_addr = ADS1115_ADDR_GND,
  .i2c_port = ADS1115_I2C_PORT,
  .ready_pin = &ready_pin,
};

static uint8_t s_queue_storage[ADS1115_QUEUE_SIZE * sizeof(float)];

static Queue ads1115_data_queue = {
  .num_items = ADS1115_QUEUE_SIZE,
  .item_size = sizeof(float),
  .storage_buf = s_queue_storage,
};

TASK(blinky, TASK_STACK_256) {
  while (true) {
    StatusCode status = gpio_toggle_state(&blinky_gpio);
    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("%s - LED %s\n", blinky->name, gpio_get_state(&blinky_gpio) == GPIO_STATE_HIGH ? "ON" : "OFF");
    } else {
      LOG_DEBUG("blinky GPIO toggle failed: %d\n", status);
    }
    delay_ms(BLINKY_PERIOD_MS);
  }
}

TASK(ads1115_writer, TASK_STACK_256) {
  TickType_t last_wake_time = xTaskGetTickCount();
  while (true) {
    float voltage;
    StatusCode status = ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &voltage);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ADC read failed: %d\n", status);
    } else {
      status = queue_send(&ads1115_data_queue, &voltage, ADS1115_SAMPLING_PERIOD_MS);
      if (status != STATUS_CODE_OK) {
        LOG_DEBUG("write to queue failed: %d\n", status);
      } else {
        LOG_DEBUG("Writing to ADC queue: %.6f\n", (double)voltage);
      }
    }
    xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(ADS1115_SAMPLING_PERIOD_MS));
  }
}

TASK(ads1115_reader, TASK_STACK_256) {
  while (true) {
    float voltage;
    StatusCode status = queue_receive(&ads1115_data_queue, &voltage, ADS1115_READER_TIMEOUT_MS);
    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("Reading from ADC queue: %.6f\n", (double)voltage);
    } else {
      LOG_DEBUG("read from queue failed: %d\n", status);
    }
  }
}

#if defined(MS_PLATFORM_X86)
TASK(ads1115_data_simulator, TASK_STACK_256) {
  unsigned int noise_rand_seed = 0xDEADBEEF;
  uint16_t simulated_voltage = 22400;

  uint8_t config_data[2] = { 0x04U, 0x83U };

  while (true) {
    uint8_t transmitted;
    while (i2c_get_tx_data(ADS1115_I2C_PORT, &transmitted, sizeof(transmitted)) == STATUS_CODE_OK) {
    }

    int16_t noise = (rand_r(&noise_rand_seed) % 1001) - 500;
    uint16_t noisy_voltage = simulated_voltage + noise;

    uint8_t conversion_data[2] = { (uint8_t)(noisy_voltage >> 8U), (uint8_t)noisy_voltage };
    StatusCode status = i2c_set_rx_data(ADS1115_I2C_PORT, config_data, sizeof(config_data));
    if (status == STATUS_CODE_OK) {
      status = i2c_set_rx_data(ADS1115_I2C_PORT, conversion_data, sizeof(conversion_data));
    }
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ADC simulation failed: %d\n", status);
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}
#endif

int main(void) {
  mcu_init();
  status_ok_or_return(tasks_init());
  log_init();
  status_ok_or_return(i2c_init(ADS1115_I2C_PORT, &i2c_settings));
  status_ok_or_return(ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin));
  status_ok_or_return(gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));

  status_ok_or_return(queue_init(&ads1115_data_queue));
  status_ok_or_return(tasks_init_task(blinky, TASK_PRIORITY(2U), NULL));
  status_ok_or_return(tasks_init_task(ads1115_writer, TASK_PRIORITY(3U), NULL));
  status_ok_or_return(tasks_init_task(ads1115_reader, TASK_PRIORITY(2U), NULL));

#if defined(MS_PLATFORM_X86)
  status_ok_or_return(tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL));
#endif

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
