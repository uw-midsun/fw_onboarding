#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"

static int s_counter = 0;

static void prv_increment(int *val) {
  (*val)++;
}

void pre_loop_init() {}

void run_100hz_cycle() {}

void run_10hz_cycle() {}

void run_1hz_cycle() {
  prv_increment(&s_counter);
  LOG_DEBUG("Hello World %d\n", s_counter);
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