#include <stdint.h>

#include "ads1115.h"
#include "test_helpers.h"
#include "unity.h"

static ADS1115_Config s_config;
static GpioAddress s_ready_pin = { .port = GPIO_PORT_B, .pin = 0U };
static uint16_t s_registers[4];
static uint32_t s_read_calls;
static uint32_t s_write_calls;
static uint32_t s_delay_calls;
static uint32_t s_fail_read;
static uint32_t s_fail_write;

void setup_test(void) {
  s_config = (ADS1115_Config){ .i2c_port = ADS1115_I2C_PORT, .i2c_addr = ADS1115_ADDR_GND };
  s_registers[ADS1115_REG_CONFIG] = 0x0483U;
  s_registers[ADS1115_REG_CONVERSION] = 0x5780U;
  s_registers[ADS1115_REG_LO_THRESH] = 0x8000U;
  s_registers[ADS1115_REG_HI_THRESH] = 0x7FFFU;
  s_read_calls = 0U;
  s_write_calls = 0U;
  s_delay_calls = 0U;
  s_fail_read = UINT32_MAX;
  s_fail_write = UINT32_MAX;
}

void teardown_test(void) {}

StatusCode TEST_MOCK(i2c_read_reg)(I2CPort port, I2CAddress addr, uint8_t reg, uint8_t *data, size_t len) {
  TEST_ASSERT_EQUAL(ADS1115_I2C_PORT, port);
  TEST_ASSERT_EQUAL_HEX8(s_config.i2c_addr, addr);
  TEST_ASSERT_LESS_THAN(4U, reg);
  TEST_ASSERT_EQUAL(2U, len);
  TEST_ASSERT_NOT_NULL(data);
  if (s_read_calls++ == s_fail_read) return STATUS_CODE_TIMEOUT;
  data[0] = (uint8_t)(s_registers[reg] >> 8U);
  data[1] = (uint8_t)s_registers[reg];
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(i2c_write_reg)(I2CPort port, I2CAddress addr, uint8_t reg, uint8_t *data, size_t len) {
  TEST_ASSERT_EQUAL(ADS1115_I2C_PORT, port);
  TEST_ASSERT_EQUAL_HEX8(s_config.i2c_addr, addr);
  TEST_ASSERT_LESS_THAN(4U, reg);
  TEST_ASSERT_EQUAL(2U, len);
  TEST_ASSERT_NOT_NULL(data);
  if (s_write_calls++ == s_fail_write) return STATUS_CODE_TIMEOUT;
  s_registers[reg] = ((uint16_t)data[0] << 8U) | data[1];
  return STATUS_CODE_OK;
}

void TEST_MOCK(delay_ms)(uint32_t time_ms) {
  TEST_ASSERT_EQUAL(20U, time_ms);
  ++s_delay_calls;
}

void test_init_register_values(void) {
  TEST_ASSERT_OK(ads1115_init(&s_config, ADS1115_ADDR_GND, &s_ready_pin));
  TEST_ASSERT_EQUAL_HEX16(0x0483U, s_registers[ADS1115_REG_CONFIG]);
  TEST_ASSERT_EQUAL_HEX16(0x0000U, s_registers[ADS1115_REG_LO_THRESH]);
  TEST_ASSERT_EQUAL_HEX16(0x5DC0U, s_registers[ADS1115_REG_HI_THRESH]);
  TEST_ASSERT_EQUAL_PTR(&s_ready_pin, s_config.ready_pin);
  TEST_ASSERT_EQUAL(3U, s_write_calls);
}

void test_init_optional_ready_pin(void) {
  TEST_ASSERT_OK(ads1115_init(&s_config, ADS1115_ADDR_SCL, NULL));
  TEST_ASSERT_EQUAL_HEX8(0x4BU, s_config.i2c_addr);
  TEST_ASSERT_NULL(s_config.ready_pin);
}

void test_invalid_arguments(void) {
  int16_t raw = 123;
  float voltage = 1.0F;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_init(NULL, ADS1115_ADDR_GND, &s_ready_pin));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_init(&s_config, (ADS1115_Address)0x47U, &s_ready_pin));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_select_channel(NULL, ADS1115_CHANNEL_0));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_select_channel(&s_config, (ADS1115_Channel)-1));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_select_channel(&s_config, (ADS1115_Channel)4U));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_raw(&s_config, ADS1115_CHANNEL_0, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_converted(&s_config, ADS1115_CHANNEL_0, NULL));
  s_config.i2c_port = NUM_I2C_PORTS;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_init(&s_config, ADS1115_ADDR_GND, &s_ready_pin));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_raw(&s_config, ADS1115_CHANNEL_0, &raw));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_converted(&s_config, ADS1115_CHANNEL_0, &voltage));
  TEST_ASSERT_EQUAL(123, raw);
  TEST_ASSERT_EQUAL_FLOAT(1.0F, voltage);
  TEST_ASSERT_EQUAL(0U, s_read_calls);
  TEST_ASSERT_EQUAL(0U, s_write_calls);
}

void test_init_stops_on_i2c_failure(void) {
  for (uint32_t i = 0U; i < 3U; ++i) {
    setup_test();
    s_fail_write = i;
    TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, ads1115_init(&s_config, ADS1115_ADDR_GND, &s_ready_pin));
    TEST_ASSERT_EQUAL(i + 1U, s_write_calls);
  }
}

void test_select_all_channels_preserves_other_bits(void) {
  const uint16_t expected[] = { 0xC58BU, 0xD58BU, 0xE58BU, 0xF58BU };
  for (uint32_t channel = 0U; channel < 4U; ++channel) {
    s_registers[ADS1115_REG_CONFIG] = 0x858BU;
    TEST_ASSERT_OK(ads1115_select_channel(&s_config, (ADS1115_Channel)channel));
    TEST_ASSERT_EQUAL_HEX16(expected[channel], s_registers[ADS1115_REG_CONFIG]);
  }
  TEST_ASSERT_EQUAL(4U, s_delay_calls);
}

void test_select_current_channel_avoids_rewrite(void) {
  s_registers[ADS1115_REG_CONFIG] = 0x4483U;
  TEST_ASSERT_OK(ads1115_select_channel(&s_config, ADS1115_CHANNEL_0));
  TEST_ASSERT_EQUAL(0U, s_write_calls);
  TEST_ASSERT_EQUAL(0U, s_delay_calls);
}

void test_read_raw_selects_channel_and_decodes_signed_bytes(void) {
  int16_t raw = 0;
  s_registers[ADS1115_REG_CONVERSION] = 0xFFC0U;
  TEST_ASSERT_OK(ads1115_read_raw(&s_config, ADS1115_CHANNEL_2, &raw));
  TEST_ASSERT_EQUAL_HEX16(0x6483U, s_registers[ADS1115_REG_CONFIG]);
  TEST_ASSERT_EQUAL(-64, raw);
  TEST_ASSERT_EQUAL(2U, s_read_calls);
}

void test_voltage_conversion_boundaries(void) {
  const uint16_t raw[] = { 0x8000U, 0xFFFFU, 0x0000U, 0x0001U, 0x5780U, 0x5DC0U, 0x7FFFU };
  const float expected[] = { -2.048F, -0.0000625F, 0.0F, 0.0000625F, 1.4F, 1.5F, 2.0479375F };
  s_registers[ADS1115_REG_CONFIG] = 0x4483U;
  for (uint32_t i = 0U; i < sizeof(raw) / sizeof(raw[0]); ++i) {
    float voltage = 9.0F;
    s_registers[ADS1115_REG_CONVERSION] = raw[i];
    TEST_ASSERT_OK(ads1115_read_converted(&s_config, ADS1115_CHANNEL_0, &voltage));
    TEST_ASSERT_FLOAT_WITHIN(0.000001F, expected[i], voltage);
  }
}

void test_read_failures_preserve_output(void) {
  for (uint32_t i = 0U; i < 2U; ++i) {
    setup_test();
    s_fail_read = i;
    float voltage = 1.0F;
    TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, ads1115_read_converted(&s_config, ADS1115_CHANNEL_0, &voltage));
    TEST_ASSERT_EQUAL_FLOAT(1.0F, voltage);
  }
}

void test_select_write_failure_stops_read(void) {
  int16_t raw = 123;
  s_fail_write = 0U;
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, ads1115_read_raw(&s_config, ADS1115_CHANNEL_1, &raw));
  TEST_ASSERT_EQUAL(123, raw);
  TEST_ASSERT_EQUAL(1U, s_read_calls);
  TEST_ASSERT_EQUAL(0U, s_delay_calls);
}
