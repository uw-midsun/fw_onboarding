<!--
General guidelines
These are just guidelines, not strict rules - document however seems best.
A README for a firmware-only project (e.g. Babydriver, MPXE, bootloader, CAN explorer) should answer the following questions:
    - What is it?
    - What problem does it solve?
    - How do I use it? (with usage examples / example commands, etc)
    - How does it work? (architectural overview)
A README for a board project (powering a hardware board, e.g. power distribution, centre console, charger, BMS carrier) should answer the following questions:
    - What is the purpose of the board?
    - What are all the things that the firmware needs to do?
    - How does it fit into the overall system?
    - How does it work? (architectural overview, e.g. what each module's purpose is or how data flows through the firmware)
-->
# fw_102_103

```
/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for fw_102_103
 *
 * @date   2025-08-23
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
#include "fw_102_103.h"

TASK(blinky, TASK_STACK_512) {

  while (true) {

  }
}

TASK(ads1115_writer, TASK_STACK_256) {
  /* This task will read from the ADS1115 external chip and push its data to a queue */

  while (true) {

  }
}

TASK(ads1115_reader, TASK_STACK_256) {
  /* This task will read from the queue containing ADS1115 data and process it */

  while (true) {

  }
}

#if defined(MS_PLATFORM_X86)
TASK(ads1115_data_simulator, TASK_STACK_256) {

  while (true) [
    
  ]
}
#endif

int main() {
  log_init();

  LOG_DEBUG("exiting main?");
  return 0;
}
```