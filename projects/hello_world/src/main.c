/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"
int i = 0;
void pre_loop_init() {}
void run_100hz_cycle() {
  LOG_DEBUG("Hello World%d\n", i);
  i++;
  /* This task runs at 100hz (10ms period) */
}
void run_10hz_cycle() {
  /* This task runs at 10hz (100ms period) */
}
void run_1hz_cycle() {
  /* This task runs at 1hz (1s period) */
}
int main() {
/*   int i = 0;
  while (1){
    LOG_DEBUG("Hello World %d\n", i);
    i++;
  } */

  mcu_init();
  tasks_init();
  log_init();
  init_master_tasks();
  tasks_start();

  return 0;
}