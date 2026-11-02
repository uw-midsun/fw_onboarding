/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2026-10-26
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"

/* Intra-component Headers */
#include "hello_world.h"

GpioAddress pa0_led = { .pin = 0U, .port = GPIO_PORT_A };
static int my_int = 0;

TASK(hello_world, TASK_STACK_512) {
  gpio_init_pin(&pa0_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  while (true) {
    LOG_DEBUG("Hello World %d\n", my_int++);
    gpio_toggle_state(&pa0_led);
    delay_ms(500);
  }
}

void pre_loop_init() {}

void run_100hz_cycle() {}

void run_10hz_cycle() {}

void run_1hz_cycle() {}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_init_task(hello_world, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}