#pragma once

/************************************************************************************************
 * @file   ads1115.h
 *
 * @brief  ADS1115 driver header file
 *
 * @date   2025-07-30
 * @author Midnight Sun Team
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"

/* Intra-component Headers */
#include "status.h"

/* Standard Library Headers */

/**
 * @defgroup ADS1115
 * @brief    ADS1115 ADC driver
 * @{
 */

/**
 * @brief Alert event ID used for soft interrupt handling.
 */
#define ALRT_EVENT 0U

/**
 * @brief Default I2C port used by the ADS1115.
 */
#define ADS1115_I2C_PORT I2C_PORT_1

/**
 * @brief I2C Address selection based on ADDR pin connection.
 */
typedef enum {
  ADS1115_ADDR_GND = 0x48U, /**< ADDR pin connected to GND */
  ADS1115_ADDR_VDD = 0x49U, /**< ADDR pin connected to VDD */
  ADS1115_ADDR_SDA = 0x4AU, /**< ADDR pin connected to SDA */
  ADS1115_ADDR_SCL = 0x4BU, /**< ADDR pin connected to SCL */
} ADS1115_Address;

/**
 * @brief ADS1115 single-ended input channels.
 */
typedef enum {
  ADS1115_CHANNEL_0 = 0, /**< Input channel AIN0 */
  ADS1115_CHANNEL_1,     /**< Input channel AIN1 */
  ADS1115_CHANNEL_2,     /**< Input channel AIN2 */
  ADS1115_CHANNEL_3,     /**< Input channel AIN3 */
} ADS1115_Channel;

/**
 * @brief Register map for the ADS1115.
 */
typedef enum {
  ADS1115_REG_CONVERSION = 0x00, /**< Conversion result register */
  ADS1115_REG_CONFIG,            /**< Configuration register */
  ADS1115_REG_LO_THRESH,         /**< Low threshold register for comparator */
  ADS1115_REG_HI_THRESH,         /**< High threshold register for comparator */
} ADS1115_Reg;

/**
 * @brief Configuration structure for the ADS1115 driver.
 */
typedef struct {
  I2CPort i2c_port;       /**< I2C port used for communication */
  I2CAddress i2c_addr;    /**< I2C address of the ADS1115 */
  Task *handler_task;     /**< Task to notify when conversion is ready */
  GpioAddress *ready_pin; /**< GPIO pin used for the ALERT/RDY signal */
} ADS1115_Config;

/**
 * @brief   Initializes the ADS1115 on the I2C bus with an optional ALERT/RDY pin.
 *
 * @param   config Pointer to the ADS1115_Config to populate
 * @param   i2c_addr I2C address of the device
 * @param   ready_pin Pointer to GPIO address of the ALERT/RDY pin (can be NULL)
 * @return  STATUS_CODE_OK if initialization succeeds
 *          STATUS_CODE_INVALID_ARGS if config or address is invalid
 *          STATUS_CODE_INTERNAL_ERROR on I2C failure
 */
StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin);

/**
 * @brief   Selects the active ADC channel on the ADS1115.
 * @details Reconfigures the ADC for single-ended input on the selected channel.
 *
 * @param   config Pointer to the ADS1115 configuration
 * @param   channel Input channel to read from (AIN0–AIN3)
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if channel or config is invalid
 */
StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel);

/**
 * @brief   Reads the raw ADC value from the specified channel.
 *
 * @param   config Pointer to the ADS1115 configuration
 * @param   channel Input channel to read from
 * @param   reading Pointer to store the raw 16-bit ADC result
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if any parameter is invalid
 *          STATUS_CODE_INTERNAL_ERROR on I2C failure
 */
StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, uint16_t *reading);

/**
 * @brief   Reads and converts the ADC value from the specified channel into a voltage.
 * @details Conversion depends on the ADS1115's internal reference and configuration.
 *
 * @param   config Pointer to the ADS1115 configuration
 * @param   channel Input channel to read from
 * @param   reading Pointer to store the converted voltage (in volts)
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if any parameter is invalid
 *          STATUS_CODE_INTERNAL_ERROR on I2C failure
 */
StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading);

/** @} */
