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
#define ITEM_SIZE sizeof(float)

static GpioAddress blinky_gpio = {
  /* --------------------- FW102 START --------------------- */
  /* This creates the struct for the gpio pin */
  /* --------------------- FW102 END --------------------- */
  .port = GPIO_PORT_B,
  .pin = 3,
};
static I2CSettings i2c_settings = {
  .scl = { .port = GPIO_PORT_B, .pin = 6U },
  .sda = { .port = GPIO_PORT_B, .pin = 7U },
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

static uint8_t ads1115_queue_storage[NUM_ITEMS * ITEM_SIZE];
static Queue ads1115_data_queue = {

  .num_items = NUM_ITEMS,
  .item_size = ITEM_SIZE,
  .storage_buf = ads1115_queue_storage,
  /* --------------------- FW103 START --------------------- */
  /* Hint: You will need to define an array to be used as the storage */
  /* --------------------- FW103 START --------------------- */
};

TASK(blinky, TASK_STACK_256) {
    GpioState led_state = GPIO_STATE_LOW;
    while(true)
    {
      gpio_toggle_state(&blinky_gpio);
      led_state = gpio_get_state(&blinky_gpio);

    if(led_state == GPIO_STATE_HIGH){
      LOG_DEBUG("Blinky: ON\n");
    }
    else{
      LOG_DEBUG("Blinky: OFF\n");
    }
      delay_ms(BLINKY_PERIOD_MS);
    }
  /* --------------------- FW103 START --------------------- */
  /* This task will blinky an LED and log the state of the pin */
  /* --------------------- FW103 END --------------------- */
}

TASK(ads1115_writer, TASK_STACK_256)
{
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the ADS1115 external chip and push its data to a queue */
  /* --------------------- FW103 END --------------------- */
  float voltage = 0.0f;
  while (true){
    ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &voltage);
    StatusCode status = queue_send(&ads1115_data_queue, &voltage, 1000);
    if(status != STATUS_CODE_OK){
      LOG_DEBUG("failed to write to queue\n");
    }
     
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}

TASK(ads1115_reader, TASK_STACK_256)
{
  float voltage = 0.0f;
  while(true){
    StatusCode status = queue_receive(&ads1115_data_queue, &voltage, 2000);
    if (status != STATUS_CODE_OK)
    {
      LOG_DEBUG("feiled to read from queue\n");
    }
    else{
      LOG_DEBUG("The ADC Voltage is: %f\n", voltage);
    }
  }
  /* --------------------- FW103 START --------------------- */
  /* This task will read from the queue containing ADS1115 data and process it */
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
 mcu_init(); /* Initialize the MCU, I2C, ADS1115 and blinky GPIO */
  /* --------------------- FW102 END --------------------- */
  gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  /* Initialize printing module */
  log_init();

  /* Initialize RTOS tasks */
  tasks_init();

  queue_init(&ads1115_data_queue);


  /* --------------------- FW103 START --------------------- */
  /* Initialize the RTOS tasks and data queue */
  /* --------------------- FW103 END --------------------- */
  tasks_init_task(blinky, TASK_PRIORITY(1U), NULL);
  tasks_init_task(ads1115_writer, TASK_PRIORITY(2U), NULL);
  tasks_init_task(ads1115_reader, TASK_PRIORITY(3U), NULL);

#if defined(MS_PLATFORM_X86)
  tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4U), NULL);
#endif

    i2c_init(ADS1115_I2C_PORT, &i2c_settings);
ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin);

  /* Start RTOS scheduler */
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
