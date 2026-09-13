/************************************************************************************************
 * @file   main.c
 *
 * @brief  FW102/FW103: read ADS1115 voltages through a queue while blinking PB3.
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
/* Use the explicit FW103 task specification (1000 ms). */
#define ADS1115_SAMPLING_PERIOD_MS 1000U
#define ADS1115_READER_PERIOD_MS 1000U
#define ADS1115_QUEUE_TIMEOUT_MS 2000U
#define ADS1115_QUEUE_LENGTH 5U

static GpioAddress blinky_gpio = { .port = GPIO_PORT_B, .pin = 3U };

/* Use I2C1's PA9/PA10 mapping documented in i2c.h. PB6/PB7 are used by log.h. */
static I2CSettings i2c_settings = {
  .scl = { .port = GPIO_PORT_A, .pin = 9U },
  .sda = { .port = GPIO_PORT_A, .pin = 10U },
  .speed = I2C_SPEED_STANDARD,
};
static GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 0U };
static ADS1115_Config ads1115_cfg = {
  .i2c_port = ADS1115_I2C_PORT,
  .i2c_addr = ADS1115_ADDR_GND,
  .ready_pin = &ready_pin,
};

static uint8_t ads1115_queue_storage[ADS1115_QUEUE_LENGTH * sizeof(float)];
static Queue ads1115_data_queue = {
  .num_items = ADS1115_QUEUE_LENGTH,
  .item_size = sizeof(float),
  .storage_buf = ads1115_queue_storage,
};

TASK(blinky, TASK_STACK_512) {
  while (true) {
    StatusCode status = gpio_toggle_state(&blinky_gpio);
    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("blinky - State: %s\n", gpio_get_state(&blinky_gpio) == GPIO_STATE_HIGH ? "ON" : "OFF");
    } else {
      LOG_DEBUG("blinky toggle failed: %d\n", status);
    }
    delay_ms(BLINKY_PERIOD_MS);
  }
}

TASK(ads1115_writer, TASK_STACK_512) {
  /* The ARM I2C implementation waits on interrupt-driven RTOS semaphores, so
   * register writes must run after tasks_start(), not before the scheduler. */
  StatusCode status;
  do {
    status = ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ADS1115 initialization failed: %d\n", status);
      delay_ms(ADS1115_SAMPLING_PERIOD_MS);
    }
  } while (status != STATUS_CODE_OK);

  while (true) {
    float reading;
    status = ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &reading);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ADC read failed: %d\n", status);
      delay_ms(ADS1115_SAMPLING_PERIOD_MS);
      continue;
    }

    status = queue_send(&ads1115_data_queue, &reading, ADS1115_QUEUE_TIMEOUT_MS);
    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("Writing to ADC queue: %f\n", (double)reading);
    } else {
      LOG_DEBUG("write to queue failed: %d\n", status);
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}

TASK(ads1115_reader, TASK_STACK_512) {
  while (true) {
    float reading;
    if (queue_receive(&ads1115_data_queue, &reading, ADS1115_QUEUE_TIMEOUT_MS) == STATUS_CODE_OK) {
      LOG_DEBUG("Reading from ADC queue: %f\n", (double)reading);
    } else {
      LOG_DEBUG("read from queue failed\n");
    }
    delay_ms(ADS1115_READER_PERIOD_MS);
  }
}

#if defined(MS_PLATFORM_X86)
TASK(ads1115_data_simulator, TASK_STACK_512) {
  unsigned int noise_rand_seed = 0xDEADBEEFU;
  while (true) {
    /* The simulated I2C backend queues transmitted bytes. Consume them as the
     * device would; otherwise its 32-byte TX queue eventually overflows. */
    uint8_t discarded;
    while (i2c_get_tx_data(ADS1115_I2C_PORT, &discarded, sizeof(discarded)) == STATUS_CODE_OK) {
    }

    int noise = (int)(rand_r(&noise_rand_seed) % 1001U) - 500;
    uint16_t raw = (uint16_t)(22400 + noise); /* 1.4 V +/- 31.25 mV. */
    /* read_raw consumes CONFIG then CONVERSION, both MSB first. This is an
     * AIN0-only sample producer, not a full register-level device emulator. */
    uint8_t response[] = { 0x44U, 0x83U, (uint8_t)(raw >> 8U), (uint8_t)raw };
    if (i2c_set_rx_data(ADS1115_I2C_PORT, response, sizeof(response)) != STATUS_CODE_OK) {
      LOG_DEBUG("ADC simulator receive buffer full\n");
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}
#endif

int main(void) {
#if defined(MS_PLATFORM_X86)
  /* The desktop mcu_init is a stub; initialize simulated GPIO explicitly. */
  mcu_init();
  status_ok_or_return(gpio_init());
#else
  status_ok_or_return(mcu_init());
#endif
  log_init();
  status_ok_or_return(tasks_init());
  status_ok_or_return(i2c_init(ADS1115_I2C_PORT, &i2c_settings));
  status_ok_or_return(gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  status_ok_or_return(queue_init(&ads1115_data_queue));

  status_ok_or_return(tasks_init_task(blinky, TASK_PRIORITY(1U), NULL));
  status_ok_or_return(tasks_init_task(ads1115_writer, TASK_PRIORITY(3U), NULL));
  status_ok_or_return(tasks_init_task(ads1115_reader, TASK_PRIORITY(2U), NULL));
#if defined(MS_PLATFORM_X86)
  status_ok_or_return(tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL));
#endif
  tasks_start();
  return 0;
}
