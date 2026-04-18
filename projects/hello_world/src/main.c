/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"


static int counter = 0;
void pre_loop_init() {}
static int prv_my_func(int *input) {
  (*input)++;
  return *input;
}
void run_100hz_cycle() {
  /* This task runs at 100hz (10ms period) */
}

void run_10hz_cycle() {

}

void run_1hz_cycle() {
  int val = prv_my_func(&counter);
  LOG_DEBUG("Hello World %d\n", val);
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