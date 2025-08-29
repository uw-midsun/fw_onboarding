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
  cmd = 0x0483;

  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /*
   * Formula: (threshold_voltage) / (4.096V) * 2^15
   */

  /* Configure lower threshold to be 0V */
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* Configure higher threshold to be 1V */
  cmd = 7992;
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

  /* --------------------- FW103 START --------------------- */
  /* Configure command to select the requested channel */
  cmd = 0x0000;
  /* ---------------------- FW103 END ---------------------- */

  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, uint16_t *reading) {
  /* --------------------- FW103 START --------------------- */
  /* TODO: complete ADS1115 read raw function */

  if (NULL != config && NULL != reading) {
    ads1115_select_channel(config, channel);
    i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)reading, sizeof(uint16_t));
  }

  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* --------------------- FW103 START --------------------- */
  /* TODO: complete ADS1115 read converted function */

  uint16_t raw_reading = 0;
  ads1115_read_raw(config, channel, &raw_reading);

  if (NULL != reading) {
    /*
     * Formula: ((raw_reading) / (2^15)) * 4.096V
     */
    *reading = (((float)raw_reading) / (32767.0f)) * 4.096f;
  }

  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}
