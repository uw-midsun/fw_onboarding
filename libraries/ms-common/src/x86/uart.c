/************************************************************************************************
 * @file   uart.c
 *
 * @brief  UART Library Source Code
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "uart.h"

/* Intra-component Headers */
#include "status.h"

StatusCode uart_init(UartPort uart, UartSettings *settings) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode uart_rx(UartPort uart, uint8_t *data, size_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode uart_tx(UartPort uart, uint8_t *data, size_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}
