/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"
#include "unistd.h"

void pre_loop_init() {}

void run_100hz_cycle() {
  /* This task runs at 100hz (10ms period) */
}

void run_10hz_cycle() {
  /* This task runs at 10hz (100ms period) */
}

void run_1hz_cycle(int counter) {
  /* This task runs at 1hz (1s period) */
  sleep(1);
  LOG_DEBUG("Hello World %d\n", counter);
}

int main() {
  int counter = 0;
  while (true)
  {
    counter++;
    run_1hz_cycle(counter);
  }
  return 0;
}