/************************************************************************************************
 * @file    ads1115.c
 *
 * @brief   ADS1115 continuous-mode driver for the FW102/FW103 exercises.
 *
 * @date    2025-07-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "delay.h"
#include "i2c.h"

/* Intra-component Headers */
#include "ads1115.h"

/* Datasheet table 8-3: defaults with OS and MODE cleared, PGA = +/-2.048 V,
 * 128 samples/s, comparator disabled. MUX is selected before each raw read. */
#define ADS1115_CONTINUOUS_CONFIG 0x0483U
#define ADS1115_MUX_MASK 0x7000U
#define ADS1115_LOW_THRESHOLD 0U
/* 1.5 V * 32768 / 2.048 V = 24000 counts. */
#define ADS1115_HIGH_THRESHOLD 24000U
#define ADS1115_VOLTS_PER_COUNT (2.048f / 32768.0f)

static bool prv_valid_config(const ADS1115_Config *config) {
  if (config == NULL || (unsigned int)config->i2c_port >= NUM_I2C_PORTS) {
    return false;
  }
  return config->i2c_addr >= ADS1115_ADDR_GND && config->i2c_addr <= ADS1115_ADDR_SCL;
}

/* The ADS1115 transfers the most significant byte first, regardless of CPU byte order. */
static StatusCode prv_write_reg(ADS1115_Config *config, ADS1115_Reg reg, uint16_t value) {
  uint8_t bytes[2] = { (uint8_t)(value >> 8U), (uint8_t)value };
  return i2c_write_reg(config->i2c_port, config->i2c_addr, reg, bytes, sizeof(bytes));
}

static StatusCode prv_read_reg(ADS1115_Config *config, ADS1115_Reg reg, uint16_t *value) {
  uint8_t bytes[2];
  status_ok_or_return(i2c_read_reg(config->i2c_port, config->i2c_addr, reg, bytes, sizeof(bytes)));
  *value = ((uint16_t)bytes[0] << 8U) | bytes[1];
  return STATUS_CODE_OK;
}

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  if (config == NULL || (unsigned int)config->i2c_port >= NUM_I2C_PORTS || i2c_addr < ADS1115_ADDR_GND || i2c_addr > ADS1115_ADDR_SCL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  config->i2c_addr = i2c_addr;
  config->ready_pin = ready_pin; /* Optional; the comparator/ALERT interrupt is not enabled. */
  status_ok_or_return(prv_write_reg(config, ADS1115_REG_CONFIG, ADS1115_CONTINUOUS_CONFIG));
  status_ok_or_return(prv_write_reg(config, ADS1115_REG_LO_THRESH, ADS1115_LOW_THRESHOLD));
  return prv_write_reg(config, ADS1115_REG_HI_THRESH, ADS1115_HIGH_THRESHOLD);
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (!prv_valid_config(config) || (unsigned int)channel > ADS1115_CHANNEL_3) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t previous;
  status_ok_or_return(prv_read_reg(config, ADS1115_REG_CONFIG, &previous));
  /* MUX 100, 101, 110, 111 select AIN0..AIN3 relative to GND. */
  uint16_t channel_bits = (0x4U + (uint16_t)channel) << 12U;
  uint16_t selected = (previous & ~ADS1115_MUX_MASK) | channel_bits;
  status_ok_or_return(prv_write_reg(config, ADS1115_REG_CONFIG, selected));

  if ((previous & ADS1115_MUX_MASK) != (selected & ADS1115_MUX_MASK)) {
    /* An in-progress conversion still uses the old channel. Allow that and a
     * complete new conversion, with oscillator/tick margin. Call from a task. */
    static const uint16_t samples_per_second[] = { 8U, 16U, 32U, 64U, 128U, 250U, 475U, 860U };
    uint16_t rate = samples_per_second[(previous >> 5U) & 0x7U];
    /* Two conversion periods plus 20% margin, rounded up, and one RTOS tick. */
    uint32_t settling_time_ms = (2400U + rate - 1U) / rate + 1U;
    delay_ms(settling_time_ms);
  }
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  if (reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  status_ok_or_return(ads1115_select_channel(config, channel));
  uint16_t raw;
  status_ok_or_return(prv_read_reg(config, ADS1115_REG_CONVERSION, &raw));
  /* Decode two's complement without relying on an out-of-range unsigned cast. */
  int32_t signed_raw = raw;
  if (raw & 0x8000U) {
    signed_raw -= 65536;
  }
  *reading = (int16_t)signed_raw;
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  if (reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  int16_t raw;
  status_ok_or_return(ads1115_read_raw(config, channel, &raw));
  *reading = (float)raw * ADS1115_VOLTS_PER_COUNT;
  return STATUS_CODE_OK;
}
