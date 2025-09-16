/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2025-09-15
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

static int my_int=0 ;// Any defined constants or preprocessor directives


void pre_loop_init() {
  /* Initialize any components before starting the main loop */
}

void run_100hz_cycle() {
  LOG_DEBUG("Hello World %d\n", my_int);
  my_int++;

}

void run_10hz_cycle() {
  /* This task runs at 10hz (100ms period) */

}

void run_1hz_cycle() {


}



int main() {
  mcu_init();
  tasks_init();
  log_init();

  LOG_DEBUG("Hello World - Starting master tasks\n");

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
