/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2025-09-14
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"

/* Intra-component Headers */
#include "hello_world.h"

void pre_loop_init() {
  hello_world_init();
}
void run_100hz_cycle() {}

void run_10hz_cycle() {}

void run_1hz_cycle() {
  hello_world_increment_and_print();
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