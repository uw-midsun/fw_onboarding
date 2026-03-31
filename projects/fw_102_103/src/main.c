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
#include "task.h"

/* Intra-component Headers */
#include "fw_102_103.h"

/* TODO: FW103 Add reader task period. Feel free to play around with these values! */
#define BLINKY_PERIOD_MS 1000U  // 1 second
#define ADS1115_SAMPLING_PERIOD_MS 1000U
#define NUM_ITEMS 5
#define ITEM_SIZE sizeof(float)

static GpioAddress blinky_gpio = {
  /* --------------------- TODO: FW102 --------------------- */
  .port = GPIO_PORT_B,
  .pin = 3,
};

static I2CSettings i2c_settings = { .scl = { .port = GPIO_PORT_B, .pin = 7U }, .sda = { .port = GPIO_PORT_B, .pin = 6U }, .speed = I2C_SPEED_STANDARD };
static GpioAddress ready_pin = {
  .port = GPIO_PORT_B,
  .pin = 0U,
};
static ADS1115_Config ads1115_cfg = {
  .i2c_addr = ADS1115_ADDR_GND,
  .i2c_port = ADS1115_I2C_PORT,
  .ready_pin = &ready_pin,
};

static uint8_t ads1115_queue_buf[NUM_ITEMS * ITEM_SIZE];

static Queue ads1115_data_queue = {
  /* --------------------- TODO: FW103 --------------------- */
  /* Hint: You will need to define an array to be used as the storage */
  .num_items = NUM_ITEMS,
  .item_size = ITEM_SIZE,
  .storage_buf = ads1115_queue_buf,
};

TASK(blinky, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will blinky an LED and log the state of the pin */

  // Initialization of any variables before task operation begins
  int my_int = 0;
  // Track LED State
  bool led_on = false;
  // Main operation of the task occurs in the while true loop
  // The task always stays in this loop
  // The scheduler will pause/resume this while loop for each task depending on priority
  while (true) {
    // toggle LED on and off
    led_on = !led_on;
    if (led_on) {
      LOG_DEBUG("Blinky - State: 1\n");
    } else {
      LOG_DEBUG("Blinky - State: 0\n");
    }
    gpio_toggle_state(&blinky_gpio);  // toggles the state
    delay_ms(BLINKY_PERIOD_MS);       // wait that one second
    // do_task_functionality();  // This function executes in the context of the calling task
  }
}
// The main function is still the entry point of the program

TASK(ads1115_writer, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the ADS1115 external chip and push its data to a queue */
  float voltage = 0.0f;

  while (true) {
    // Read Converted voltage from ADS1115
    StatusCode status = ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &voltage);

    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("Writing to ADC QUEUE: %f\n", voltage);
      // Pushing Voltage to queue
      StatusCode queue_status = queue_send(&ads1115_data_queue, &voltage, 1000);

      if (queue_status != STATUS_CODE_OK) {
        LOG_DEBUG("Write to queue failed");
      }
    }
    // Delay for sampling period
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_reader, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the queue containing ADS1115 data and process it */
  float received_voltage = 0.0f;

  while (true) {
    // Receive data frome queue (blocks until data availaible)
    StatusCode status = queue_receive(&ads1115_data_queue, &received_voltage, 1000);

    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("Reading from ADC Queue: %f\n", received_voltage);
    } else {
      LOG_DEBUG("Read from queue failed \n");
    }
    delay_ms(1000);
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
  mcu_init();
  gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  i2c_init(ADS1115_I2C_PORT, &i2c_settings);
  ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin);
  /* --------------------- FW102 END --------------------- */

  /* Initialize printing module */
  log_init();

  /* Initialize RTOS tasks */
  tasks_init();

  /* --------------------- FW103 START --------------------- */
  /* Initialize the RTOS tasks and data queue */
  queue_init(&ads1115_data_queue);
  tasks_init_task(blinky, TASK_PRIORITY(2), NULL);
  tasks_init_task(ads1115_writer, TASK_PRIORITY(3), NULL);
  tasks_init_task(ads1115_reader, TASK_PRIORITY(3), NULL);
  /* --------------------- FW103 END --------------------- */

#if defined(MS_PLATFORM_X86)
  tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL);
#endif

  /* Start RTOS scheduler */
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
