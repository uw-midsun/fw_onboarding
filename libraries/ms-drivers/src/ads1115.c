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

#include "gpio_interrupts.h"
#include "i2c.h"

/* Intra-component Headers */
#include "status.h"

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  if (config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  config->i2c_addr = i2c_addr;
  uint16_t cmd;
  StatusCode status;
  /* --------------------- FW103 START --------------------- */
  /* Configure for continuous mode (MODE bit = 0) */
  cmd = 0x0483;

  status = i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  /* Configure lower threshold to be 0V */
  cmd = 0;
  status = i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  /* Configure higher threshold to be 1.5V */
  cmd = 24000;  // raw = V * 32768/2.048
  status = i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_HI_THRESH, (uint8_t *)(&cmd), 2);
  if (status != STATUS_CODE_OK) {
    return status;
  }
  /* ---------------------- FW103 END ---------------------- */

  // Register the ALRT pin
  /* TODO (optional) */

  return STATUS_CODE_OK;
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t cmd;

  /* Read the current configuration register value */
  StatusCode status = i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)&cmd, sizeof(cmd));

  if (status != STATUS_CODE_OK) return status;
  /* Mask out the current channel bits (MUX bits are 12-14) */
  cmd &= ~0x7000;

  /* --------------------- FW103 START --------------------- */
  /* Configure command to select the requested channel (Channel N should be default GND) */
  cmd |= (uint16_t)((0x4 | channel) << 12);  // move the bits to 14 - 12 on the config reg
  /* ---------------------- FW103 END ---------------------- */

  status = i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return status;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  /* --------------------- FW103 START --------------------- */
  if (config == NULL || reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  StatusCode status = ads1115_select_channel(config, channel);
  if (status != STATUS_CODE_OK) {
    return status;
  }
  return i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)reading, sizeof(uint16_t));
  /* TODO: complete ADS1115 read raw function */
  /* ---------------------- FW103 END ---------------------- */
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* --------------------- FW103 START --------------------- */
  int16_t raw = 0;
  StatusCode status = ads1115_read_raw(config, channel, &raw);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  const float maxReading = 32768.0f;
  const float maxVoltage = 2.048f;

  *reading = ((float)raw / maxReading) * maxVoltage;
  return STATUS_CODE_OK;
  /* ---------------------- FW103 END ---------------------- */
}
