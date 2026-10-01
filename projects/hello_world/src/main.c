/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2026-09-20
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

static int num = 0;

static void increment_func(int* a) {
  ++*a;
  LOG_DEBUG("Hello World %d\n", *a);
}

void pre_loop_init() {}

void run_100hz_cycle() {}

void run_10hz_cycle() {
  increment_func(&num);
}

void run_1hz_cycle() {}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_start();

  run_10hz_cycle();
  
  LOG_DEBUG("exiting main?");
  return 0;
}