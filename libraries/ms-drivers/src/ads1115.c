#include "ads1115.h"
#if !defined(MS_PLATFORM_X86)
#include "delay.h"
#endif

static StatusCode write_register(ADS1115_Config *config, uint8_t reg, uint16_t value) {
  uint8_t data[] = { (uint8_t)(value >> 8U), (uint8_t)value };
  return i2c_write_reg(config->i2c_port, config->i2c_addr, reg, data, sizeof(data));
}

static StatusCode read_register(ADS1115_Config *config, uint8_t reg, uint16_t *value) {
  uint8_t data[2];
  status_ok_or_return(i2c_read_reg(config->i2c_port, config->i2c_addr, reg, data, sizeof(data)));
  *value = ((uint16_t)data[0] << 8U) | data[1];
  return STATUS_CODE_OK;
}

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address address, GpioAddress *ready_pin) {
  if (config == NULL || address < ADS1115_ADDR_GND || address > ADS1115_ADDR_SCL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  config->i2c_addr = address;
  config->ready_pin = ready_pin;
  /* Continuous mode, +/-2.048 V, 128 SPS; comparator disabled. */
  status_ok_or_return(write_register(config, ADS1115_REG_CONFIG, 0x0483U));
  status_ok_or_return(write_register(config, ADS1115_REG_LO_THRESH, 0U));
  /* 1.5 V / 62.5 uV per count = 24000. */
  return write_register(config, ADS1115_REG_HI_THRESH, 0x5DC0U);
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (config == NULL || (unsigned int)channel > ADS1115_CHANNEL_3) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint16_t value;
  status_ok_or_return(read_register(config, ADS1115_REG_CONFIG, &value));
  value = (value & ~0x7000U) | ((4U + (uint16_t)channel) << 12U);
  return write_register(config, ADS1115_REG_CONFIG, value);
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  if (reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  status_ok_or_return(ads1115_select_channel(config, channel));
#if !defined(MS_PLATFORM_X86)
  /* Allow two conversion periods plus oscillator tolerance after a MUX switch. */
  delay_ms(20U);
#endif
  uint16_t value;
  status_ok_or_return(read_register(config, ADS1115_REG_CONVERSION, &value));
  *reading = (int16_t)((int32_t)value - ((value & 0x8000U) ? 65536 : 0));
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
