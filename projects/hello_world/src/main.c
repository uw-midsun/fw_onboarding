/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2026-01-08
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

// Function to Increment Integer
static int incr(int *input) {
	(*input)++;
	return *input;
}

// Increment Variable
static int i = 0;

void pre_loop_init() {

}

void run_100hz_cycle() {
}

void run_10hz_cycle() {}

void run_1hz_cycle() {
}
int main(void) {

  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  // Logic for Onboarding Task
  while (true) {
  	LOG_DEBUG("Hello World %d\n", incr(&i));
  }

  tasks_start();
  
  
  LOG_DEBUG("exiting main?");

  return 0;
}
