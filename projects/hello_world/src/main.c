/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2026-09-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "hello_world.h"

static int counter = 0;

static int increment(int value) {
  return value + 1;
}

void pre_loop_init() {}

void run_100hz_cycle() {}

void run_10hz_cycle() {}

void run_1hz_cycle() {
  counter = increment(counter);
  LOG_DEBUG("Hello World %d\n", counter);
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
