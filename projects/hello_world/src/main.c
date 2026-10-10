/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2026-10-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "hello_world.h"

void pre_loop_init() {}

void run_100hz_cycle() {}

void run_10hz_cycle() {}

void run_1hz_cycle() {
  int counter = 0;
  while (true) {
    counter++;
    LOG_DEBUG("%d\n", counter);
    delay_ms(1000);
  }
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