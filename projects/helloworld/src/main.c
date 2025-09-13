/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for helloworld
 *
 * @date   2025-09-11
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
#include "helloworld.h"

int var = 0;

void increment(int *var)
{
    (*var)++;
}

void pre_loop_init() {}

void run_100hz_cycle()
{
  increment(&var);
  LOG_DEBUG("HELLO WORLD %d\n", var);
}

void run_10hz_cycle() {}

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
