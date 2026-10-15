#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "ads1115.h"

static uint16_t registers[4];
static StatusCode read_failure, write_failure;
static uint32_t delay_total;
void delay_ms(uint32_t ms) { delay_total += ms; }

StatusCode i2c_read_reg(I2CPort port, I2CAddress addr, uint8_t reg, uint8_t *data, size_t length) {
  assert(port == I2C_PORT_1 && addr == ADS1115_ADDR_GND && length == 2 && reg < 4);
  if (read_failure) return read_failure;
  data[0] = (uint8_t)(registers[reg] >> 8);
  data[1] = (uint8_t)registers[reg];
  return STATUS_CODE_OK;
}
StatusCode i2c_write_reg(I2CPort port, I2CAddress addr, uint8_t reg, uint8_t *data, size_t length) {
  assert(port == I2C_PORT_1 && addr == ADS1115_ADDR_GND && length == 2 && reg < 4);
  if (write_failure) return write_failure;
  registers[reg] = ((uint16_t)data[0] << 8) | data[1];
  return STATUS_CODE_OK;
}

int main(void) {
  ADS1115_Config cfg = { .i2c_port = I2C_PORT_1 };
  assert(ads1115_init(&cfg, ADS1115_ADDR_GND, NULL) == STATUS_CODE_OK);
  assert(registers[1] == 0x0483 && registers[2] == 0 && registers[3] == 24000);
  puts("PASS: initialization and MSB-first configuration/threshold writes");
  for (int channel = 0; channel < 4; ++channel) {
    registers[1] = 0x8DAB;
    assert(ads1115_select_channel(&cfg, channel) == STATUS_CODE_OK);
    assert(registers[1] == ((0x8DAB & ~0x7000) | ((4 + channel) << 12)));
  }
  puts("PASS: all four channel selections preserve other configuration bits");
  const uint16_t raw_values[] = { 0, 22400, 32767, 0x8000, 0xFFFF };
  const float expected[] = { 0.0f, 1.4f, 2.0479375f, -2.048f, -0.0000625f };
  for (unsigned int i = 0; i < sizeof(raw_values)/sizeof(raw_values[0]); ++i) {
    registers[0] = raw_values[i];
    float voltage = 123.0f;
    assert(ads1115_read_converted(&cfg, ADS1115_CHANNEL_0, &voltage) == STATUS_CODE_OK);
    assert(fabsf(voltage - expected[i]) < 0.000001f);
  }
  puts("PASS: zero, 1.4 V, positive full scale, negative full scale and -1 count");
  int16_t raw = 123;
  assert(ads1115_init(NULL, ADS1115_ADDR_GND, NULL) == STATUS_CODE_INVALID_ARGS);
  assert(ads1115_init(&cfg, (ADS1115_Address)0x47, NULL) == STATUS_CODE_INVALID_ARGS);
  assert(ads1115_select_channel(&cfg, (ADS1115_Channel)-1) == STATUS_CODE_INVALID_ARGS);
  assert(ads1115_select_channel(&cfg, (ADS1115_Channel)4) == STATUS_CODE_INVALID_ARGS);
  assert(ads1115_read_raw(NULL, 0, &raw) == STATUS_CODE_INVALID_ARGS);
  assert(ads1115_read_raw(&cfg, 0, NULL) == STATUS_CODE_INVALID_ARGS);
  assert(ads1115_read_converted(&cfg, 0, NULL) == STATUS_CODE_INVALID_ARGS);
  puts("PASS: invalid pointers, addresses and channels rejected");
  read_failure = STATUS_CODE_TIMEOUT;
  assert(ads1115_read_raw(&cfg, 0, &raw) == STATUS_CODE_TIMEOUT && raw == 123);
  read_failure = STATUS_CODE_OK;
  write_failure = STATUS_CODE_UNREACHABLE;
  assert(ads1115_init(&cfg, ADS1115_ADDR_GND, NULL) == STATUS_CODE_UNREACHABLE);
  assert(ads1115_read_raw(&cfg, 0, &raw) == STATUS_CODE_UNREACHABLE && raw == 123);
  puts("PASS: I2C read/write failures propagate without changing output");
#ifndef MS_PLATFORM_X86
  assert(delay_total == 5U * 20U);
  puts("PASS: hardware path waits two 128-SPS conversion periods per read");
#endif
  puts("All ADC driver checks passed (mock I2C, no physical hardware).");
  return 0;
}
