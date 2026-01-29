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
#include "log.h"
#include "mcu.h"
#include "queues.h"

/* Intra-component Headers */
#include "fw_102_103.h"

/* TODO: FW103 Add reader task period. Feel free to play around with these values! */
#define BLINKY_PERIOD_MS 1000U
#define ADS1115_SAMPLING_PERIOD_MS 1000U
#define NUM_ITEMS 5
#define ITEM_SIZE sizeof(uint32_t)

static uint8_t s_queue_buf[NUM_ITEMS*ITEM_SIZE];

static GpioAddress blinky_gpio = {
  /* --------------------- TODO: FW102 --------------------- */
  .port = GPIO_PORT_B, // blinky is on gpio_b3
  .pin = 3,
};

static Queue ads1115_data_queue = {
  /* --------------------- TODO: FW103 --------------------- */
  /* Hint: You will need to define an array to be used as the storage */
  .num_items = NUM_ITEMS, 
  .item_size = ITEM_SIZE,
  .storage_buf = s_queue_buf
};

static I2CSettings i2c_settings = {
  .scl = {.port = GPIO_PORT_B, .pin = 7U}, // clock line
  .sda = { .port = GPIO_PORT_B, .pin = 6U }, // data line
  .speed = I2C_SPEED_STANDARD // speed at which it goes?
};

static GpioAddress ready_pin = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

static ADS1115_Config ads1115_cfg = {
  .i2c_addr = ADS1115_ADDR_GND,
  .i2c_port = ADS1115_I2C_PORT, 
  .ready_pin = &ready_pin, // where the ready pin is..?
};

TASK(blinky, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will blinky an LED and log the state of the pin */
  while (true) {
    LOG_DEBUG("Blinky! State: %d\n", gpio_get_state(&blinky_gpio));
    gpio_toggle_state(&blinky_gpio);
    delay_ms(BLINKY_PERIOD_MS);
  }

  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_writer, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the ADS1115 external chip and push its data to a queue */
  float reading = 0.0; 
  while(true){
    ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &reading);
    StatusCode status = queue_send(&ads1115_data_queue, &reading, 1000);

    if(status != STATUS_CODE_OK){
      LOG_DEBUG("Write to queue failed");
    } else {
      LOG_DEBUG("Writing to the ADC queue: %f\n", (double)reading);
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_reader, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the queue containing ADS1115 data and process it */
  float received = 0;

  while(true){
    StatusCode status = queue_receive(&ads1115_data_queue, &received, 1000);
    delay_ms(500);
    if(status != STATUS_CODE_OK){
      LOG_DEBUG("Write to queue failed");
    } else {
      LOG_DEBUG("Reading from the ADC queue: %f\n", (double)received);
    }
  }
  /* --------------------- FW103 END --------------------- */
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
  /* --------------------- FW102 END --------------------- */
  mcu_init();
  gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  /* Initialize printing module */
  log_init();

  /* Initialize RTOS tasks */
  queue_init(&ads1115_data_queue); // send queue data in here
  tasks_init();
  tasks_init_task(blinky, TASK_PRIORITY(4U), NULL);
  tasks_init_task(ads1115_reader, TASK_PRIORITY(3U), NULL);
  tasks_init_task(ads1115_writer, TASK_PRIORITY(2U), NULL);

  /* Initialize I2C and ADS1115 */
  i2c_init(ADS1115_I2C_PORT, &i2c_settings);
  ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin);

  /* --------------------- FW103 START --------------------- */
  /* Initialize the RTOS tasks and data queue */
  /* --------------------- FW103 END --------------------- */

#if defined(MS_PLATFORM_X86)
  tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL);
#endif

  /* Start RTOS scheduler */
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
