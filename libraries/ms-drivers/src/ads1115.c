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
  cmd = 0b0000010010000011 ; // we edit this command, everything else is done reading table 8-3

  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* Configure lower threshold to be 0V */
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* Configure higher threshold to be 1.5V */
  cmd = 24000; // 1.5V converted to a range in 0-32768
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

  uint16_t cmd; // storing the result frm i2c_read_reg

  /* Read the current configuration register value */
  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)&cmd, sizeof(cmd));

  /* Mask out the current channel bits (MUX bits are 12-14) */
  cmd &= ~0x7000; // masking the bits here, multiplexer bits = MUX found on datasheet homie

  /* --------------------- FW103 START --------------------- */
  /* Configure command to select the requested channel (Channel N should be default GND) */
  
  if (channel == ADS1115_CHANNEL_0) {
    cmd |= 0b0100000000000000;
  } else if (channel == ADS1115_CHANNEL_1) {
    cmd |= 0b0101000000000000;
  } else if (channel == ADS1115_CHANNEL_2) {
    cmd |= 0b0110000000000000;
  } else if (channel == ADS1115_CHANNEL_3) {
    cmd |= 0b0111000000000000;
  }

  /* ---------------------- FW103 END ---------------------- */

  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  /* --------------------- FW103 START --------------------- */
  /* TODO: complete ADS1115 read raw function */

  uint16_t buffer;

  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION /*0x00 */, (uint8_t *)&buffer, 2);

  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* --------------------- FW103 START --------------------- */
  /* TODO: complete ADS1115 read converted function */
  int16_t raw_adc_val; 

  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)&raw_adc_val, 2);

  *reading = ((float)raw_adc_val/32768)*2.048; // Yo the reading is passed by reference so we don't even
  // need to return it its like what we wanna read
  // ece150 moment thank you harder

  /* ---------------------- FW103 END ---------------------- */
  return STATUS_CODE_OK;
}
