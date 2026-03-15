/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"

static int count = 0;

static void increment(int *n) {
  (*n)++;
}

void pre_loop_init() {}

void run_100hz_cycle() {
  /* This task runs at 100hz (10ms period) */
}

void run_10hz_cycle() {
  /* This task runs at 10hz (100ms period) */
}

void run_1hz_cycle() {
  /* This task runs at 1hz (1s period) */
  increment(&count);
  LOG_DEBUG("Hello World %d\n", count);
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