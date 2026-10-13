/************************************************************************************************
 * @file    ads1115.c
 *
 * @brief   Ads1115
 *
 * @date    2026-10-13
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "ads1115.h"
#include "delay.h"
#include "i2c.h"

/* Intra-component Headers */
#include "status.h"

#define ADS1115_CONFIG_CONTINUOUS 0x0483U
#define ADS1115_MUX_MASK 0x7000U
#define ADS1115_MUX_SINGLE_ENDED 0x4000U
#define ADS1115_CHANNEL_SETTLE_MS 20U
#define ADS1115_VOLTS_PER_COUNT (2.048F / 32768.0F)

static bool prv_valid_config(const ADS1115_Config *config) {
  return config != NULL && (unsigned int)config->i2c_port < NUM_I2C_PORTS && config->i2c_addr >= ADS1115_ADDR_GND && config->i2c_addr <= ADS1115_ADDR_SCL;
}

static StatusCode prv_write_register(const ADS1115_Config *config, ADS1115_Reg reg, uint16_t value) {
  uint8_t data[2] = { (uint8_t)(value >> 8U), (uint8_t)value };
  return i2c_write_reg(config->i2c_port, config->i2c_addr, reg, data, sizeof(data));
}

static StatusCode prv_read_register(const ADS1115_Config *config, ADS1115_Reg reg, uint16_t *value) {
  uint8_t data[2];
  status_ok_or_return(i2c_read_reg(config->i2c_port, config->i2c_addr, reg, data, sizeof(data)));
  *value = ((uint16_t)data[0] << 8U) | data[1];
  return STATUS_CODE_OK;
}

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  if (config == NULL || (unsigned int)config->i2c_port >= NUM_I2C_PORTS || i2c_addr < ADS1115_ADDR_GND || i2c_addr > ADS1115_ADDR_SCL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  config->i2c_addr = i2c_addr;
  config->ready_pin = ready_pin;

  status_ok_or_return(prv_write_register(config, ADS1115_REG_CONFIG, ADS1115_CONFIG_CONTINUOUS));
  status_ok_or_return(prv_write_register(config, ADS1115_REG_LO_THRESH, 0U));
  status_ok_or_return(prv_write_register(config, ADS1115_REG_HI_THRESH, (uint16_t)(1.5F / ADS1115_VOLTS_PER_COUNT + 0.5F)));

  return STATUS_CODE_OK;
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (!prv_valid_config(config) || (unsigned int)channel > ADS1115_CHANNEL_3) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t cmd;
  status_ok_or_return(prv_read_register(config, ADS1115_REG_CONFIG, &cmd));
  uint16_t selected = (cmd & (uint16_t)~ADS1115_MUX_MASK) | ADS1115_MUX_SINGLE_ENDED | ((uint16_t)channel << 12U);

  if (selected != cmd) {
    status_ok_or_return(prv_write_register(config, ADS1115_REG_CONFIG, selected));
    delay_ms(ADS1115_CHANNEL_SETTLE_MS);
  }

  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  if (reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  status_ok_or_return(ads1115_select_channel(config, channel));
  uint16_t raw;
  status_ok_or_return(prv_read_register(config, ADS1115_REG_CONVERSION, &raw));
  *reading = (int16_t)raw;
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  if (reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  int16_t raw;
  status_ok_or_return(ads1115_read_raw(config, channel, &raw));
  *reading = raw * ADS1115_VOLTS_PER_COUNT;
  return STATUS_CODE_OK;
}
