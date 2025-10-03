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
  if (config == NULL || ready_pin == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  config->i2c_addr = i2c_addr;
  uint16_t cmd;

  /* --------------------- FW103 START --------------------- */
  /* Configure for continuous mode (MODE bit = 0) */
  cmd = (uint16_t)((0x4u << 12) | 0x0400 | 0x0003);

  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* Configure lower threshold to be 0V */
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* Configure higher threshold to be 1.5V */
  cmd = (uint16_t)(1.5f * 16000.0f);   // 1.5 V -> 24000 (0x5DC0)
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_HI_THRESH, (uint8_t *)(&cmd), 2);
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
  cmd = (uint16_t)((0x4u | (uint16_t)channel) << 12);

  /* Read the current configuration register value */
  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)&cmd, sizeof(cmd));

  /* Mask out the current channel bits (MUX bits are 12-14) */
  cmd &= ~0x7000;

  /* --------------------- FW103 START --------------------- */
  /* Configure command to select the requested channel (Channel N should be default GND) */
  cmd |= 0x0000U;
  /* ---------------------- FW103 END ---------------------- */

  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
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

  // Read 16-bit two's complement conversion value
  status = i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION,
                        (uint8_t *)reading, sizeof(*reading));
  if (status != STATUS_CODE_OK) {
    return status;
  }

  return STATUS_CODE_OK;
  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* --------------------- FW103 START --------------------- */
    if (config == NULL || reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  int16_t raw = 0;
  StatusCode status = ads1115_read_raw(config, channel, &raw);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  // ±2.048 V full-scale → LSB = 2.048/32768 V
  *reading = ((float)raw / 32768.0f) * 2.048f;
  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}
