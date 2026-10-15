#include <stdlib.h>

#include "ads1115.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "queues.h"
#include "tasks.h"

#define BLINKY_PERIOD_MS 1000U
#define ADS1115_SAMPLING_PERIOD_MS 1000U
#define READER_TIMEOUT_MS 1500U
#define QUEUE_LENGTH 8U

static GpioAddress blinky_gpio = { .port = GPIO_PORT_B, .pin = 3U };
static GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 0U };
static I2CSettings i2c_settings = {
  .scl = { .port = GPIO_PORT_B, .pin = 7U },
  .sda = { .port = GPIO_PORT_B, .pin = 6U },
  .speed = I2C_SPEED_STANDARD,
};
static ADS1115_Config ads1115_cfg = { .i2c_port = ADS1115_I2C_PORT };
static uint8_t queue_storage[QUEUE_LENGTH * sizeof(float)];
static Queue ads1115_data_queue = {
  .num_items = QUEUE_LENGTH,
  .item_size = sizeof(float),
  .storage_buf = queue_storage,
};

TASK(blinky, TASK_STACK_256) {
  (void)context;
  while (true) {
    gpio_toggle_state(&blinky_gpio);
    LOG_DEBUG("blinky: %s\n", gpio_get_state(&blinky_gpio) == GPIO_STATE_HIGH ? "ON" : "OFF");
    delay_ms(BLINKY_PERIOD_MS);
  }
}

TASK(ads1115_writer, TASK_STACK_256) {
  (void)context;
#if defined(MS_PLATFORM_X86)
  /* Keep reads behind the simulator's input updates. */
  delay_ms(10U);
#endif
  while (true) {
    float voltage;
    StatusCode status = ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &voltage);
    if (status != STATUS_CODE_OK) {
      LOG_WARN("ads1115_writer: read failed (%d)\n", status);
    } else if (queue_send(&ads1115_data_queue, &voltage, 0U) != STATUS_CODE_OK) {
      LOG_WARN("ads1115_writer: queue full\n");
    } else {
      LOG_DEBUG("ads1115_writer: queued reading\n");
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}

TASK(ads1115_reader, TASK_STACK_256) {
  (void)context;
  while (true) {
    float voltage;
    if (queue_receive(&ads1115_data_queue, &voltage, READER_TIMEOUT_MS) == STATUS_CODE_OK) {
      LOG_DEBUG("ads1115_reader: %.4f V\n", (double)voltage);
    } else {
      LOG_WARN("ads1115_reader: timed out waiting for a reading\n");
    }
  }
}

#if defined(MS_PLATFORM_X86)
TASK(ads1115_data_simulator, TASK_STACK_256) {
  (void)context;
  unsigned int noise_rand_seed = 0xDEADBEEFU;
  while (true) {
    /* The mock I2C implementation has a bounded TX FIFO. Drain each cycle. */
    uint8_t discarded;
    while (i2c_get_tx_data(ADS1115_I2C_PORT, &discarded, 1U) == STATUS_CODE_OK) {
    }
    int16_t noise = (int16_t)((int)(rand_r(&noise_rand_seed) % 1001U) - 500);
    uint16_t raw = (uint16_t)(22400 + noise);
    /* Config and conversion responses, in the real device's MSB-first order. */
    uint8_t data[] = { 0x44U, 0x83U, (uint8_t)(raw >> 8U), (uint8_t)raw };
    if (i2c_set_rx_data(ADS1115_I2C_PORT, data, sizeof(data)) != STATUS_CODE_OK) {
      LOG_WARN("ads1115_data_simulator: RX FIFO full\n");
    }
    delay_ms(ADS1115_SAMPLING_PERIOD_MS);
  }
}
#endif

int main(void) {
  mcu_init();
#if defined(MS_PLATFORM_X86)
  /* The x86 mcu_init is a stub and does not initialize GPIO. */
  status_ok_or_return(gpio_init());
#endif
  status_ok_or_return(tasks_init());
  log_init();
  status_ok_or_return(gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  status_ok_or_return(i2c_init(ADS1115_I2C_PORT, &i2c_settings));
  status_ok_or_return(ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin));
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
