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
#include "ads1115.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "mcu.h"
#include "queues.h"
#include "tasks.h"

/* Intra-component Headers */
#include "fw_102_103.h"

#define BLINKY_PERIOD_MS 1000U
#define ADS1115_SAMPLING_PERIOD_MS 1000U
#define ADS1115_READER_WAIT_MS 1000U
#define ADS1115_QUEUE_LENGTH 5U

static GpioAddress blinky_gpio = {
  .port = GPIO_PORT_B,
  .pin = 3U,
};

static I2CSettings i2c_settings = {
  .scl = { .port = GPIO_PORT_B, .pin = 7U },
  .sda = { .port = GPIO_PORT_B, .pin = 6U },
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

/* The queue stores copies of five voltage readings. */
static uint8_t ads1115_queue_storage[ADS1115_QUEUE_LENGTH * sizeof(float)];
static Queue ads1115_data_queue = {
  .num_items = ADS1115_QUEUE_LENGTH,
  .item_size = sizeof(float),
  .storage_buf = ads1115_queue_storage,
};

TASK(blinky, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  while (true) {
    StatusCode status = gpio_toggle_state(&blinky_gpio);
    if (status == STATUS_CODE_OK) {
      GpioState state = gpio_get_state(&blinky_gpio);
      LOG_DEBUG("blinky: LED %s\n", state == GPIO_STATE_HIGH ? "ON" : "OFF");
    } else {
      LOG_DEBUG("blinky: GPIO toggle failed (%d)\n", status);
    }
    delay_ms(BLINKY_PERIOD_MS);
  }
  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_writer, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  while (true) {
    float voltage;
    StatusCode status = ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &voltage);
    if (status == STATUS_CODE_OK) {
      status = queue_send(&ads1115_data_queue, &voltage, ADS1115_SAMPLING_PERIOD_MS);
      if (status == STATUS_CODE_OK) {
        LOG_DEBUG("Writing to ADC queue: %.6f V\n", voltage);
      } else {
        LOG_DEBUG("write to queue failed (%d)\n", status);
      }
    } else {
      LOG_DEBUG("ADS1115 read failed (%d)\n", status);
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_reader, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  while (true) {
    float voltage;
    StatusCode status = queue_receive(&ads1115_data_queue, &voltage, ADS1115_READER_WAIT_MS);
    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("Reading from ADC queue: %.6f V\n", voltage);
    } else {
      LOG_DEBUG("read from queue failed (%d)\n", status);
    }
  }
  /* --------------------- FW103 END --------------------- */
}

#if defined(MS_PLATFORM_X86)
TASK(ads1115_data_simulator, TASK_STACK_256) {
  /* This task simulates the I2C data for simulated off-target testing */

  unsigned int noise_rand_seed = 0xDEADBEEF;
  uint16_t simulated_voltage = 22400; /* 1.4V */

  uint8_t dummy_cfg_reg_data[2] = { 0x04U, 0x83U };

  while (true) {
    /* Consume simulated outgoing transactions so repeated readings do not fill the TX queue. */
    uint8_t tx_byte;
    while (i2c_get_tx_data(ADS1115_I2C_PORT, &tx_byte, sizeof(tx_byte)) == STATUS_CODE_OK) {
    }

    /* Simulate noise +- 500 */
    int16_t noise = (rand_r(&noise_rand_seed) % 1001) - 500;
    uint16_t noisy_voltage = simulated_voltage + noise;
    uint8_t noisy_voltage_data[2] = { (uint8_t)(noisy_voltage >> 8U), (uint8_t)noisy_voltage };

    i2c_set_rx_data(ADS1115_I2C_PORT, dummy_cfg_reg_data, sizeof(dummy_cfg_reg_data));
    i2c_set_rx_data(ADS1115_I2C_PORT, noisy_voltage_data, sizeof(noisy_voltage_data));
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}
#endif

int main() {
  /* --------------------- FW102 START --------------------- */
  mcu_init();
  gpio_init();
  gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  i2c_init(ADS1115_I2C_PORT, &i2c_settings);
  ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin);
  /* --------------------- FW102 END --------------------- */

  /* Initialize printing module */
  log_init();

  /* Initialize RTOS tasks */
  tasks_init();

  /* --------------------- FW103 START --------------------- */
  if (queue_init(&ads1115_data_queue) != STATUS_CODE_OK) {
    LOG_CRITICAL("ADC queue initialization failed\n");
    return 1;
  }
  if (tasks_init_task(blinky, TASK_PRIORITY(2U), NULL) != STATUS_CODE_OK || tasks_init_task(ads1115_writer, TASK_PRIORITY(3U), NULL) != STATUS_CODE_OK ||
      tasks_init_task(ads1115_reader, TASK_PRIORITY(2U), NULL) != STATUS_CODE_OK) {
    LOG_CRITICAL("FW103 task initialization failed\n");
    return 1;
  }
  /* --------------------- FW103 END --------------------- */

#if defined(MS_PLATFORM_X86)
  tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL);
#endif

  /* Start RTOS scheduler */
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
