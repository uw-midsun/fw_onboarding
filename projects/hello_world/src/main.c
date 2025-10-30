/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2025-10-23
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

// Increment the counter
void increment_counter(void) {
  counter++;
}

void pre_loop_init() {
  // Initialize Counter
  counter = 0;
  LOG_DEBUG("Hello World\n");
}

void run_100hz_cycle() {}

void run_10hz_cycle() {
  increment_counter();
  LOG_DEBUG("Hello World %d\n", counter);
}

void run_1hz_cycle() {}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}