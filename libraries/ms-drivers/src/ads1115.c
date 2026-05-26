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
  /* Configure for continuous mode (MODE bit = 0)
   * Config register layout (Table 8-3 of ADS1115 datasheet):
   *   [15]    OS        = 0
   *   [14-12] MUX       = 000 (set per-channel by ads1115_select_channel)
   *   [11-9]  PGA       = 010 (+/-2.048V FSR)
   *   [8]     MODE      = 0   (continuous conversion)
   *   [7-5]   DR        = 100 (128 SPS, default)
   *   [4]     COMP_MODE = 0
   *   [3]     COMP_POL  = 0
   *   [2]     COMP_LAT  = 0
   *   [1-0]   COMP_QUE  = 11  (disable comparator)
   * Binary: 0 000 010 0 100 0 0 0 11 = 0000 0100 1000 0011 = 0x0483
   */
  cmd = 0x0483U;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* Lower threshold = 0V => raw 0 */
  cmd = 0x0000U;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* Higher threshold = 1.5V
   * raw = V / V_FSR * FullScale = 1.5 / 2.048 * 32768 = 24000 = 0x5DC0
   */
  cmd = 0x5DC0U;
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
  /* Single-ended input (AINn = GND) with channel N at AINp:
   *   MUX = 0b1cc where cc is the channel number (0..3)
   * Shift into bits 12-14:
   */
  cmd |= ((uint16_t)(0x4U | (uint16_t)channel) << 12U);
  /* ---------------------- FW103 END ---------------------- */

  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  /* --------------------- FW103 START --------------------- */
  if (config == NULL || reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Route the desired channel to the ADC's MUX before reading. */
  StatusCode status = ads1115_select_channel(config, channel);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  /* Read 2 bytes from the CONVERSION register (16-bit signed result). */
  return i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)reading, sizeof(*reading));
  /* ---------------------- FW103 END ---------------------- */
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

  /* PGA was configured for +/-2.048V FSR in ads1115_init.
   * Full-scale code = 32768 (2^15). Convert raw counts to volts:
   */
  *reading = ((float)raw / 32768.0f) * 2.048f;
  return STATUS_CODE_OK;
  /* ---------------------- FW103 END ---------------------- */
}
