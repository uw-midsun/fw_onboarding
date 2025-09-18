#include "hello_world.h"
#include "log.h"

static int s_counter = 0;

void hello_world_init(void) {
  s_counter = 0;
}

void hello_world_increment_and_print(void) {
  s_counter++;
  LOG_DEBUG("Hello World %d\n", s_counter);
}