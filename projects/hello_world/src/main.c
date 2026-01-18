/************************************************************************************************
 * @file    main.c
 *
 * @brief   Main
 *
 * @date    2026-01-18
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */

void pre_loop_init() {}

void run_100hz_cycle() {
  /* This task runs at 100hz (10ms period) */
}

void run_10hz_cycle() {
  /* This task runs at 10hz (100ms period) */
}

void run_1hz_cycle() {
  static int count = 67;
  printf("Hello, World! %d\n", count++);
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
