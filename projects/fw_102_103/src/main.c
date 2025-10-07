/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for fw_102_103
 *
 * @date   2025-08-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "ads1115.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "queues.h"

/* Intra-component Headers */
#include "fw_102_103.h"

/* TODO: FW103 Add reader task period. Feel free to play around with these values! */
#define BLINKY_PERIOD_MS 1000U
#define ADS1115_SAMPLING_PERIOD_MS 1000U
#define QUEUE_NUM_ITEMS 5
#define QUEUE_ITEM_SIZE sizeof(uint32_t)
static GpioAddress blinky_gpio = {
  /* --------------------- TODO: FW102 --------------------- */
  .port = GPIO_PORT_B,
  .pin = 3
};
static uint8_t s_queue_buf[QUEUE_NUM_ITEMS*QUEUE_ITEM_SIZE];
static Queue ads1115_data_queue = {
  /* --------------------- TODO: FW103 --------------------- */
  /* Hint: You will need to define an array to be used as the storage */
  .num_items = QUEUE_NUM_ITEMS,
  .item_size = QUEUE_ITEM_SIZE,
  .storage_buf = s_queue_buf
};
static I2CSettings i2c_settings = {
  .scl = { .port = GPIO_PORT_B, .pin = 7U },
  .sda = { .port = GPIO_PORT_B, .pin = 6U },
  .speed = I2C_SPEED_STANDARD
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

TASK(blinky, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will blinky an LED and log the state of the pin */
  while (true) {
    gpio_toggle_state(&blinky_gpio);
    GpioState state = gpio_get_state(&blinky_gpio);
    if (state == GPIO_STATE_LOW) {
      LOG_DEBUG("Blinky State: OFF\n");
    } else {
      LOG_DEBUG("Blinky State: ON\n");
    }
    delay_ms(BLINKY_PERIOD_MS);
  }
  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_writer, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the ADS1115 external chip and push its data to a queue */
  float reading;
  
  while (true) {
    StatusCode ads_read_status = ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &reading);
    if (ads_read_status != STATUS_CODE_OK) {
      LOG_DEBUG("read ads failed\n");
    } else {
      StatusCode queue_send_status = queue_send(&ads1115_data_queue, &reading, 1000);
      if (queue_send_status != STATUS_CODE_OK) {
        LOG_DEBUG("write to ads queue failed\n");
      } else {
        LOG_DEBUG("writing to ads queue: %f\n", reading);
      }
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_reader, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the queue containing ADS1115 data and process it */
  float reading;
  
  while (true) {
    StatusCode queue_receive_status = queue_receive(&ads1115_data_queue, &reading, 1000);
    if (queue_receive_status != STATUS_CODE_OK) {
      LOG_DEBUG("queue read failed\n");
    } else {
      LOG_DEBUG("received from ads queue: %f\n", reading);
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
    /* Initialize printing module */
  }
  /* ------------------- FW103 END --------------------- */
}

#if defined(MS_PLATFORM_X86)
TASK(ads1115_data_simulator, TASK_STACK_256) {
  /* This task simulates the I2C data for simulated off-target testing */

  unsigned int noise_rand_seed = 0xDEADBEEF;
  uint16_t simulated_voltage = 22400; /* 1.4V */
  
  uint16_t dummy_cfg_reg_data = 0x123;
  
  while (true) {
    /* Simulate noise +- 500 */
    int16_t noise = (rand_r(&noise_rand_seed) % 1001) - 500;
    uint16_t noisy_voltage = simulated_voltage + noise;
    i2c_set_rx_data(ADS1115_I2C_PORT, (uint8_t *)&dummy_cfg_reg_data, sizeof(dummy_cfg_reg_data));
    i2c_set_rx_data(ADS1115_I2C_PORT, (uint8_t *)&noisy_voltage, sizeof(noisy_voltage));
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}
#endif

int main() {
  /* --------------------- FW102 START --------------------- */
  /* Initialize the MCU, I2C, ADS1115 and blinky GPIO */
  mcu_init();
  gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  i2c_init(ADS1115_I2C_PORT, &i2c_settings);
  ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin);
  /* --------------------- FW102 END --------------------- */
  
  log_init();
  
  /* Initialize RTOS tasks */
  tasks_init();
  
  /* --------------------- FW103 START --------------------- */
  /* Initialize the RTOS tasks and data queue */
  queue_init(&ads1115_data_queue);
  /* --------------------- FW103 END --------------------- */
  
  #if defined(MS_PLATFORM_X86)
  tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL);
  tasks_init_task(ads1115_writer, TASK_PRIORITY(3U), NULL);
  tasks_init_task(ads1115_reader, TASK_PRIORITY(2U), NULL);
  tasks_init_task(blinky, TASK_PRIORITY(1U), NULL);
  #endif
  
  
  /* Start RTOS scheduler */
  tasks_start();
  
  return 0;
}
