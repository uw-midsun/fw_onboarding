/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for hello_world
 *
 * @date   2026-09-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"
#include "interrupt.h" // interrupts are required for soft timers
#include "soft_timer.h"

/* Intra-component Headers */
#include "hello_world.h"

static int my_int = 0; // Any static/global declarationstes

#define COIN_FLIP_PERIOD_MS 1000 // milliseconds between coin flips
typedef struct CoinFlipStorage {
  uint16_t num_heads;
  uint16_t num_tails;
} CoinFlipStorage;
void prv_timer_callback(SoftTimerId timer_id, void *context) {
  CoinFlipStorage *storage = context; // cast void* to our struct so we can use it
  uint8_t coinflip = rand() % 2;
  if (coinflip == 1) storage->num_heads++;
  else if (coinflip == 0) storage->num_tails++;
  // log output
  LOG_DEBUG("Num heads: %i, num tails: %i\n", storage->num_heads, storage->num_tails);
  // start the timer again, so it keeps periodically flipping coins
  soft_timer_start_millis(COIN_FLIP_PERIOD_MS,
                          prv_timer_callback, 
                          storage, 
                          NULL);


void pre_loop_init() {}

void run_100hz_cycle() {}

void run_10hz_cycle() {
}

void run_1hz_cycle() {}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_start();

  interrupt_init(); // interrupts must be initialized for soft timers to work
  soft_timer_init(); // soft timers must be initialized before using them
  CoinFlipStorage storage = { 0 }; // we use this to initialize a struct to be all 0
  soft_timer_start_millis(COIN_FLIP_PERIOD_MS, // timer duration
                          prv_timer_callback, // function to call after timer
                          &storage, // automatically gets cast to void*
                          NULL); // timer id - not needed here
  while (true) {
    wait(); // waits until an interrupt is triggered rather than endlessly spinning
  }
  
  //run_10hz_cycle();

  LOG_DEBUG("exiting main?");
  return 0;
}