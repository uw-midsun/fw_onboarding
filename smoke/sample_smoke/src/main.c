/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for sample_smoke
 *
 * @date   2025-06-07
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

TASK(sample_smoke, TASK_STACK_1024) {
  while (true) {
    LOG_DEBUG("Sample smoke application:\r\n");
    delay_ms(100);
  }
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(sample_smoke, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
