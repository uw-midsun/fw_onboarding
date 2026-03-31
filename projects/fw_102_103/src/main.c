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

/* FW103 Task Periods */
#define BLINKY_PERIOD_MS 1000U
#define ADS1115_SAMPLING_PERIOD_MS 1000U

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

static GpioAddress blinky_gpio = {
    .port = GPIO_PORT_B,
    .pin  = 3,
};

#define ADS1115_QUEUE_LENGTH 4
static uint8_t ads1115_queue_storage[ADS1115_QUEUE_LENGTH * sizeof(float)];
static Queue ads1115_data_queue = {
    .num_items   = ADS1115_QUEUE_LENGTH,
    .item_size   = sizeof(float),
    .storage_buf = ads1115_queue_storage,
};

TASK(blinky, TASK_STACK_256) {
    while (true) {
        gpio_toggle_state(&blinky_gpio);
        GpioState state = gpio_get_state(&blinky_gpio);
        LOG_DEBUG("Blinky State = %d\n", state);
        delay_ms(BLINKY_PERIOD_MS);
    }
}

TASK(ads1115_writer, TASK_STACK_256) {
    float voltage = 0.0f;
    while (true) {
        if (ads1115_read_converted(&ads1115_cfg, ADS1115_CHANNEL_0, &voltage) != STATUS_CODE_OK) {
            LOG_DEBUG("ADS1115 read failed\n");
        } else {
            if (queue_send(&ads1115_data_queue, &voltage, 1000) != STATUS_CODE_OK) {
                LOG_DEBUG("Write to queue failed\n");
            }
        }
        delay_ms(ADS1115_SAMPLING_PERIOD_MS);
    }
}

TASK(ads1115_reader, TASK_STACK_256) {
    float voltage = 0.0f;
    while (true) {
        if (queue_receive(&ads1115_data_queue, &voltage, 1000) == STATUS_CODE_OK) {
            LOG_DEBUG("ADC Voltage = %f\n", voltage);
        } else {
            LOG_DEBUG("Read from queue failed\n");
        }
        delay_ms(1000);
    }
}

#if defined(MS_PLATFORM_X86)
TASK(ads1115_data_simulator, TASK_STACK_256) {
    unsigned int noise_rand_seed = 0xDEADBEEF;
    uint16_t simulated_voltage = 22400; /* 1.4V */
    uint16_t dummy_cfg_reg_data = 0x123;

    while (true) {
        int16_t noise = (rand_r(&noise_rand_seed) % 1001) - 500;
        uint16_t noisy_voltage = simulated_voltage + noise;

        i2c_set_rx_data(ADS1115_I2C_PORT, (uint8_t *)&dummy_cfg_reg_data, sizeof(dummy_cfg_reg_data));
        i2c_set_rx_data(ADS1115_I2C_PORT, (uint8_t *)&noisy_voltage, sizeof(noisy_voltage));
        delay_ms(ADS1115_SAMPLING_PERIOD_MS);
    }
}
#endif

int main() {
    /* Initialize MCU, I2C, ADS1115, and Blinky GPIO */
    mcu_init();
    i2c_init(ADS1115_I2C_PORT, &i2c_settings);
    ads1115_init(&ads1115_cfg, ADS1115_ADDR_GND, &ready_pin);
    gpio_init_pin(&blinky_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

    /* Initialize log system */
    log_init();

    /* Initialize RTOS tasks and queue */
    tasks_init();
    queue_init(&ads1115_data_queue);

    /* Task initialization with priorities */
    tasks_init_task(blinky, TASK_PRIORITY(1), NULL);
    tasks_init_task(ads1115_reader, TASK_PRIORITY(2), NULL);
    tasks_init_task(ads1115_writer, TASK_PRIORITY(2), NULL);

#if defined(MS_PLATFORM_X86)
    tasks_init_task(ads1115_data_simulator, TASK_PRIORITY(4), NULL);
#endif

    /* Start RTOS scheduler */
    tasks_start();

    return 0;
}