#include <string.h>

#include "ads1115.h"
#include "test_helpers.h"

static ADS1115_Config s_config;
static uint8_t s_config_bytes[2];
static uint8_t s_sample_bytes[2];
static uint8_t s_write_regs[3];
static uint8_t s_write_bytes[3][2];
static unsigned int s_reads;
static unsigned int s_writes;
static unsigned int s_fail_read;
static unsigned int s_fail_write;

StatusCode TEST_MOCK(i2c_read_reg)(I2CPort port, I2CAddress address, uint8_t reg, uint8_t *data, size_t len) {
  TEST_ASSERT_EQUAL(I2C_PORT_2, port);
  TEST_ASSERT_EQUAL(ADS1115_ADDR_SCL, address);
  TEST_ASSERT_EQUAL(2U, len);
  s_reads++;
  if (s_reads == s_fail_read) {
    return STATUS_CODE_TIMEOUT;
  }
  if (reg == ADS1115_REG_CONFIG) {
    memcpy(data, s_config_bytes, len);
  } else {
    TEST_ASSERT_EQUAL(ADS1115_REG_CONVERSION, reg);
    memcpy(data, s_sample_bytes, len);
  }
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(i2c_write_reg)(I2CPort port, I2CAddress address, uint8_t reg, uint8_t *data, size_t len) {
  TEST_ASSERT_EQUAL(I2C_PORT_2, port);
  TEST_ASSERT_EQUAL(ADS1115_ADDR_SCL, address);
  TEST_ASSERT_EQUAL(2U, len);
  TEST_ASSERT_LESS_THAN(3U, s_writes);
  s_write_regs[s_writes] = reg;
  memcpy(s_write_bytes[s_writes], data, len);
  s_writes++;
  return s_writes == s_fail_write ? STATUS_CODE_TIMEOUT : STATUS_CODE_OK;
}

void setup_test(void) {
  s_config = (ADS1115_Config){ .i2c_port = I2C_PORT_2, .i2c_addr = ADS1115_ADDR_SCL };
  s_config_bytes[0] = 0x04U;
  s_config_bytes[1] = 0x83U;
  memset(s_sample_bytes, 0, sizeof(s_sample_bytes));
  memset(s_write_regs, 0, sizeof(s_write_regs));
  memset(s_write_bytes, 0, sizeof(s_write_bytes));
  s_reads = s_writes = s_fail_read = s_fail_write = 0U;
}

void teardown_test(void) {}

void test_initialization_registers_and_byte_order(void) {
  GpioAddress ready = { .port = GPIO_PORT_B, .pin = 0U };
  TEST_ASSERT_OK(ads1115_init(&s_config, ADS1115_ADDR_SCL, &ready));
  const uint8_t expected_regs[] = { 1U, 2U, 3U };
  const uint8_t expected_bytes[] = { 0x04U, 0x83U, 0x00U, 0x00U, 0x5DU, 0xC0U };
  TEST_ASSERT_EQUAL(3U, s_writes);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_regs, s_write_regs, 3U);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_bytes, s_write_bytes, 6U);
  TEST_ASSERT_EQUAL_PTR(&ready, s_config.ready_pin);
}

void test_optional_ready_pin(void) {
  TEST_ASSERT_OK(ads1115_init(&s_config, ADS1115_ADDR_SCL, NULL));
  TEST_ASSERT_NULL(s_config.ready_pin);
}

void test_all_channels_preserve_other_config_bits(void) {
  for (unsigned int channel = 0; channel < 4U; channel++) {
    setup_test();
    s_config_bytes[0] = 0xB4U;
    s_config_bytes[1] = 0x9BU;
    TEST_ASSERT_OK(ads1115_select_channel(&s_config, (ADS1115_Channel)channel));
    TEST_ASSERT_EQUAL(ADS1115_REG_CONFIG, s_write_regs[0]);
    TEST_ASSERT_EQUAL_HEX8(0xC4U + (channel << 4), s_write_bytes[0][0]);
    TEST_ASSERT_EQUAL_HEX8(0x9BU, s_write_bytes[0][1]);
    TEST_ASSERT_EQUAL(1U, s_reads);
    TEST_ASSERT_EQUAL(1U, s_writes);
  }
}

void test_signed_raw_and_voltage_conversion(void) {
  const uint16_t codes[] = { 0x0000U, 0x0001U, 0x5780U, 0x7FFFU, 0x8000U, 0xC000U, 0xFFFFU };
  const int16_t raw_values[] = { 0, 1, 22400, 32767, -32768, -16384, -1 };
  const float voltages[] = { 0.0f, 0.0000625f, 1.4f, 2.0479375f, -2.048f, -1.024f, -0.0000625f };
  for (unsigned int i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
    setup_test();
    s_sample_bytes[0] = codes[i] >> 8;
    s_sample_bytes[1] = codes[i] & 0xFFU;
    int16_t raw = 123;
    TEST_ASSERT_OK(ads1115_read_raw(&s_config, ADS1115_CHANNEL_0, &raw));
    TEST_ASSERT_EQUAL_INT16(raw_values[i], raw);
    float voltage = 10.0f;
    TEST_ASSERT_OK(ads1115_read_converted(&s_config, ADS1115_CHANNEL_0, &voltage));
    TEST_ASSERT_FLOAT_WITHIN(0.000001f, voltages[i], voltage);
    TEST_ASSERT_EQUAL(4U, s_reads);
    TEST_ASSERT_EQUAL(2U, s_writes);
  }
}

void test_initialization_stops_at_each_failed_write(void) {
  for (unsigned int step = 1U; step <= 3U; step++) {
    setup_test();
    s_fail_write = step;
    TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, ads1115_init(&s_config, ADS1115_ADDR_SCL, NULL));
    TEST_ASSERT_EQUAL(step, s_writes);
  }
}

void test_read_errors_leave_output_unchanged(void) {
  for (unsigned int step = 1U; step <= 3U; step++) {
    setup_test();
    if (step == 2U) {
      s_fail_write = 1U;
    } else {
      s_fail_read = step == 1U ? 1U : 2U;
    }
    float voltage = 9.0f;
    TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, ads1115_read_converted(&s_config, ADS1115_CHANNEL_0, &voltage));
    TEST_ASSERT_EQUAL_FLOAT(9.0f, voltage);
    TEST_ASSERT_EQUAL(step == 3U ? 2U : 1U, s_reads);
    TEST_ASSERT_EQUAL(step == 1U ? 0U : 1U, s_writes);
  }
}

void test_invalid_arguments_do_not_access_i2c(void) {
  int16_t raw = 0;
  float voltage = 0;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_init(NULL, ADS1115_ADDR_SCL, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_init(&s_config, (ADS1115_Address)0x47U, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_raw(NULL, ADS1115_CHANNEL_0, &raw));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_raw(&s_config, (ADS1115_Channel)4U, &raw));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_raw(&s_config, (ADS1115_Channel)-1, &raw));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_raw(&s_config, ADS1115_CHANNEL_0, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_converted(&s_config, ADS1115_CHANNEL_0, NULL));
  s_config.i2c_port = NUM_I2C_PORTS;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_read_converted(&s_config, ADS1115_CHANNEL_0, &voltage));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1115_init(&s_config, ADS1115_ADDR_SCL, NULL));
  TEST_ASSERT_EQUAL(0U, s_reads);
  TEST_ASSERT_EQUAL(0U, s_writes);
}
