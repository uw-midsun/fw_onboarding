/************************************************************************************************
 * @file    ads1115.c
 *
 * @brief   ADS1115 driver source file
 *
 * @date    2025-07-30
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

/* ADS1115 register transfers send the high byte first */
static StatusCode prv_write_register(ADS1115_Config *config, ADS1115_Reg reg, uint16_t value) {
  uint8_t data[] = { (uint8_t)(value >> 8), (uint8_t)value };
  return i2c_write_reg(config->i2c_port, config->i2c_addr, reg, data, sizeof(data));
}

static bool prv_valid_config(const ADS1115_Config *config) {
  return config != NULL && (unsigned int)config->i2c_port < NUM_I2C_PORTS && config->i2c_addr >= ADS1115_ADDR_GND && config->i2c_addr <= ADS1115_ADDR_SCL;
}

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  if (config == NULL || (unsigned int)config->i2c_port >= NUM_I2C_PORTS || i2c_addr < ADS1115_ADDR_GND || i2c_addr > ADS1115_ADDR_SCL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  config->i2c_addr = i2c_addr;
  config->ready_pin = ready_pin;
  /* Reset defaults with OS and MODE cleared for continuous conversion at 128 SPS */
  status_ok_or_return(prv_write_register(config, ADS1115_REG_CONFIG, 0x0483U));
  status_ok_or_return(prv_write_register(config, ADS1115_REG_LO_THRESH, 0x0000U));
  /* 1.5 V / 62.5 uV = 24000 */
  status_ok_or_return(prv_write_register(config, ADS1115_REG_HI_THRESH, 24000U));
  return STATUS_CODE_OK;
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (!prv_valid_config(config) || (unsigned int)channel > ADS1115_CHANNEL_3) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t data[2];
  status_ok_or_return(i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, data, sizeof(data)));
  uint16_t cmd = ((uint16_t)data[0] << 8) | data[1];
  /* MUX 100 through 111 selects AIN0 through AIN3 relative to ground */
  cmd = (cmd & ~0x7000U) | (0x4000U + ((uint16_t)channel << 12));
  return prv_write_register(config, ADS1115_REG_CONFIG, cmd);
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  if (reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  status_ok_or_return(ads1115_select_channel(config, channel));
#if !defined(MS_PLATFORM_X86)
  /* Allow the in-flight conversion and a new channel conversion at the configured 128 SPS */
  delay_ms(20U);
#endif
  uint8_t data[2];
  status_ok_or_return(i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, data, sizeof(data)));
  int32_t value = ((uint16_t)data[0] << 8) | data[1];
  if (value & 0x8000U) {
    value -= 65536;
  }
  *reading = (int16_t)value;
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  if (reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  int16_t raw;
  status_ok_or_return(ads1115_read_raw(config, channel, &raw));
  *reading = raw * (2.048f / 32768.0f);
  return STATUS_CODE_OK;
}
