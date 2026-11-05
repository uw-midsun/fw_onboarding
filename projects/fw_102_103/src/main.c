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

//adding a queue size for the ADS1115 data queue. This defines the number of items that can be in the queue at any given time.
#define ADS115_QUEUE_SIZE 5U

static GpioAddress blinky_gpio = {
  /* --------------------- TODO: FW102 --------------------- */
  .port = GPIO_PORT_B,
  .pin = 3,
};

// Adding given structs based on instructions
static I2CSettings i2c_settings = {
    .scl = {
        .port = GPIO_PORT_B,
        .pin = 7U,
    },
    .sda = {
        .port = GPIO_PORT_B,
        .pin = 6U,
    },
    .speed = I2C_SPEED_STANDARD
};

static GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = 0U,
};

static ADS1115_Config ads115_cfg = {
  .i2c_addr = ADS1115_ADDR_GND,
  .i2c_port = ADS1115_I2C_PORT,
  .ready_pin = &ready_pin,
};

//added a static buffer to be used as the storage for the ADS1115 data queue. The size of the buffer is defined as the number of items in the queue times the size of each item as shown in the line of code below.
static uint8_t ads1115_queue_buf[ADS115_QUEUE_SIZE * sizeof(float)];

static Queue ads1115_data_queue = {
  /* --------------------- TODO: FW103 --------------------- */
  /* Hint: You will need to define an array to be used as the storage */
  
  // adding code inside the queue struct to define the number of items, size of each item, and the storage buffer for the queue
  .num_items = ADS115_QUEUE_SIZE,
  .item_size = sizeof(float),
  .storage_buf = ads1115_queue_buf,
};

TASK(blinky, TASK_STACK_256) {
  /* --------------------- FW103 START --------------------- */
  /* This task will blinky an LED and log the state of the pin */

  // adding while loop to toggle the state of the blinky GPIO pin, log the state of the pin, and delay for the defined blinky period.
  while (true) {
    // toggling the state of the blinky GPIO pin
    gpio_toggle_pin(&blinky_gpio);

    // logging the state of the blinky GPIO pin
    LOG_DEBUG("Blinky - state: %d\n", gpio_get_state(&blinky_gpio));

    // delaying for the defined blinky period
    delay_ms(BLINKY_PERIOD_MS);
  }
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
  
  // adding a float variable to store the voltage reading from the ADS1115
  float voltage;
  
  //adding while loop to read the converted voltage from the ADS1115, send it to the queue, and log the result of the queue send.
  while (true) {
    // reading the converted voltage from the ADS1115 and storing it in the voltage variable
    ads1115_read_converted(&ads115_cfg, ADS1115_CHANNEL_0, &voltage);
    
    // adding the voltage reading to the ADS1115 data queue with no delay
    StatusCode status = queue_send(&ads1115_data_queue, &voltage, 0);

    //adding a check to see if the queue send was successful and loging the result
    if (status != STATUS_CODE_OK)
      LOG_DEBUG("Failed to send data to queue\n");
    else 
      LOG_DEBUG("Sent data to queue: %f\n", voltage);
      delay_ms(ADS1115_SAMPLING_PERIOD_MS);
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

  /* Initialize printing module */
  /* Initialize the MCU */
  mcu_init();
  gpio_init();

  gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  
  i2c_init(ADS1115_I2C_PORT, &i2c_settings);
  ads1115_init(&ads115_cfg, ADS1115_ADDR_GND, &ready_pin);

  log_init();

  /* Initialize RTOS tasks */
  tasks_init();

  /* --------------------- FW103 START --------------------- */
  /* Initialize the RTOS tasks and data queue */

  //adding code to initialize the ADS1115 data queue and the RTOS tasks for blinky, ads1115_writer, and ads1115_reader.
  queue_init(&ads1115_data_queue);
  task_init_task(blinky, TASK_PRIORITY(1), NULL);
  task_init_task(ads1115_writer, TASK_PRIORITY(2), NULL);
  task_init_task(ads1115_reader, TASK_PRIORITY(3), NULL);
  /* --------------------- FW103 END --------------------- */

#if defined(MS_PLATFORM_X86)
  tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL);
#endif

  /* Start RTOS scheduler */
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
