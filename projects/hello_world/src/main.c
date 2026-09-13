/************************************************************************************************
 * @file   main.c
 *
 * @brief  FW101: increment an integer through a function and log it at 1 Hz.
 *
 * @date   2026-09-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <limits.h>

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "hello_world.h"

static int s_count = 0;
static int s_100hz_count = 0;
static int s_10hz_count = 0;

static int prv_increment(int *value) {
  /* Keep the indefinitely running example from overflowing a signed integer. */
  if (*value == INT_MAX) {
    *value = 0;
  }
  ++(*value);
  return *value;
}

void pre_loop_init(void) {
  s_count = 0;
  s_100hz_count = 0;
  s_10hz_count = 0;
}

void run_100hz_cycle(void) {
  int count = prv_increment(&s_100hz_count);
  LOG_DEBUG("100 Hz cycle %d\n", count);
}

void run_10hz_cycle(void) {
  int count = prv_increment(&s_10hz_count);
  LOG_DEBUG("10 Hz cycle %d\n", count);
}

void run_1hz_cycle(void) {
  int count = prv_increment(&s_count);
  LOG_DEBUG("Hello World %d\n", count);
}

int main(void) {
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
