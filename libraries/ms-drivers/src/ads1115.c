/************************************************************************************************
 * @file    ads1115.c
 *
 * @brief   ADS1115 driver source file
 *
 * @date    2025-07-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "ads1115.h"

#include "delay.h"
#include "gpio_interrupts.h"
#include "i2c.h"

/* Intra-component Headers */
#include "status.h"

/* ADS1115 register transfers send the high byte first. */
static StatusCode prv_write_register(ADS1115_Config *config, ADS1115_Reg reg, uint16_t value) {
  uint8_t data[2] = { (uint8_t)(value >> 8U), (uint8_t)value };
  return i2c_write_reg(config->i2c_port, config->i2c_addr, reg, data, sizeof(data));
}

static StatusCode prv_read_register(ADS1115_Config *config, ADS1115_Reg reg, uint16_t *value) {
  uint8_t data[2];
  status_ok_or_return(i2c_read_reg(config->i2c_port, config->i2c_addr, reg, data, sizeof(data)));
  *value = (uint16_t)(((uint16_t)data[0] << 8U) | data[1]);
  return STATUS_CODE_OK;
}

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  if (config == NULL || (unsigned int)config->i2c_port >= NUM_I2C_PORTS || i2c_addr < ADS1115_ADDR_GND || i2c_addr > ADS1115_ADDR_SCL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  config->i2c_addr = i2c_addr;
  config->ready_pin = ready_pin;
  uint16_t cmd;

  /* --------------------- FW103 START --------------------- */
  /* Configure for continuous mode (MODE bit = 0) */
  cmd = 0x0483U;

  status_ok_or_return(prv_write_register(config, ADS1115_REG_CONFIG, cmd));

  /* Configure lower threshold to be 0V */
  cmd = 0U;
  status_ok_or_return(prv_write_register(config, ADS1115_REG_LO_THRESH, cmd));

  /* Configure higher threshold to be 1.5V */
  cmd = 24000U;
  status_ok_or_return(prv_write_register(config, ADS1115_REG_HI_THRESH, cmd));
  /* ---------------------- FW103 END ---------------------- */

  // Register the ALRT pin
  /* TODO (optional) */

  return STATUS_CODE_OK;
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (config == NULL || (unsigned int)channel > ADS1115_CHANNEL_3) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t cmd;

  /* Read the current configuration register value */
  status_ok_or_return(prv_read_register(config, ADS1115_REG_CONFIG, &cmd));

  uint16_t channel_bits = (uint16_t)(0x4000U | ((uint16_t)channel << 12U));
  if ((cmd & 0x7000U) == channel_bits) {
    return STATUS_CODE_OK;
  }

  /* Mask out the current channel bits (MUX bits are 12-14) */
  cmd &= (uint16_t)~0x7000U;

  /* --------------------- FW103 START --------------------- */
  /* Configure command to select the requested channel (Channel N should be default GND) */
  cmd |= channel_bits;
  /* ---------------------- FW103 END ---------------------- */

  status_ok_or_return(prv_write_register(config, ADS1115_REG_CONFIG, cmd));

#if defined(MS_PLATFORM_ARM)
  /* At 128 samples/s, allow the current and next conversion to finish after a channel change. */
  delay_ms(20U);
#endif
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  /* --------------------- FW103 START --------------------- */
  if (config == NULL || reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  status_ok_or_return(ads1115_select_channel(config, channel));
  uint16_t raw;
  status_ok_or_return(prv_read_register(config, ADS1115_REG_CONVERSION, &raw));
  *reading = (int16_t)raw;
  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* --------------------- FW103 START --------------------- */
  if (config == NULL || reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  int16_t raw;
  status_ok_or_return(ads1115_read_raw(config, channel, &raw));
  *reading = (float)raw * (2.048f / 32768.0f);
  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}
