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

// FLAGS FOR CONFIG REGISTER
// Continuous Flags
#define CONFIG_PGA_DEFAULT 0x400
#define CONFIG_DR_DEFAULT 0x80
#define CONFIG_COMP_QUE_DEFAULT 0x3

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  if (config == NULL || ready_pin == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  config->i2c_addr = i2c_addr;
  uint16_t cmd;

  /* --------------------- FW103 START --------------------- */
  /* Configure for continuous mode (MODE bit = 0) */
  // in binary, the default continuous mode is 0000 0100 1000 0011
  cmd = CONFIG_PGA_DEFAULT | CONFIG_DR_DEFAULT | CONFIG_COMP_QUE_DEFAULT;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* Configure lower threshold to be 0V */
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* Configure higher threshold to be 1.5V */
  /*const float TARGET_VOLT = 1.5;
  //const float MAX_VOLT = 2.048;
  const int MAX_READING = 32768; ----- i decided to just put in the hex-conversion because of floating-point precision being off*/

  cmd = 0x5DC0;
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

  /* Read the current configuration register value */
  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)&cmd, sizeof(cmd));

  /* Mask out the current channel bits (MUX bits are 12-14) */
  cmd &= ~0x7000;

  /* --------------------- FW103 START --------------------- */
  /* Configure command to select the requested channel (Channel N should be default GND) */
  cmd |= 0x7000;
  /* ---------------------- FW103 END ---------------------- */

  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  /* --------------------- FW103 START --------------------- */
  /* TODO: complete ADS1115 read raw function */
  if (config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Select Channel to Read from
  ads1115_select_channel(config, channel);

  // Read Config From Conversion Register (NOTE uint16_t = 16 bits = 2 bytes)
  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)&reading, sizeof(*reading));

  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* --------------------- FW103 START --------------------- */
  /* TODO: complete ADS1115 read converted function */
  if (config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Get Raw Reading
  int16_t raw_reading;
  ads1115_read_raw(config, channel, &raw_reading);

  // Calculate Converted Voltage
  const int16_t MAX_READING = 32768;  // 32768 bits
  const float MAX_VOLTAGE_READING = 2.048;

  *reading = (raw_reading / MAX_READING) * MAX_VOLTAGE_READING;

  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}
