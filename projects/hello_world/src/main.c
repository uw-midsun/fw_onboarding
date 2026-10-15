/************************************************************************************************
 * @file    main.c
 *
 * @brief   Main
 *
 * @date    2026-10-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <inttypes.h>

/* Inter-component Headers */
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */

static uint32_t counter;
static uint32_t increment(uint32_t value) {
  return value + 1U;
}
void pre_loop_init(void) {}
void run_100hz_cycle(void) {}
void run_10hz_cycle(void) {}
void run_1hz_cycle(void) {
  counter = increment(counter);
  LOG_DEBUG("Hello World: %" PRIu32 "\n", counter);
}
int main(void) {
  mcu_init();
  status_ok_or_return(tasks_init());
  log_init();
  status_ok_or_return(init_master_tasks());
  tasks_start();
  return 0;
}
