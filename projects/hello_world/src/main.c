/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2025-09-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* ---------------- Module-local state & helpers ---------------- */

static int s_counter = 0;

/* Increment an integer by reference and return the new value. */
static int increment(int *value) {
  (*value)++;
  return *value;
}

/* ---------------- Scheduler hooks ---------------- */

void pre_loop_init() {
  /* Optional one-time setup before periodic cycles begin. */
  LOG_DEBUG("Starting Hello World Deliverable\n");
}

void run_100hz_cycle() {
  /* Not used for this deliverable. */
}

void run_10hz_cycle() {
  /* Not used for this deliverable. */
}

void run_1hz_cycle() {
  /* Runs once per second. Increment via function and print. */
  int current = increment(&s_counter);
  LOG_DEBUG("Hello World %d\n", current);
}

/* ---------------- Entry point ---------------- */

int main(void) {
  mcu_init();
  tasks_init();
  log_init();

  /* Wire run_100hz_cycle / run_10hz_cycle / run_1hz_cycle into the scheduler */
  init_master_tasks();

  /* Starts the periodic scheduler (does not normally return) */
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
