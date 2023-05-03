/**
 * @file distance.c
 * @author Joseph
 * @brief Calculate Distance and debug
 * @version 0.1
 * @date 2023-05-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "../../kilolib/kilolib.h"
#define DEBUG
#include "../../kilolib/debug.h"

// global variables
uint16_t wait_time = 300;

// put your setup code here, will be run once at the beginning
void setup() {
    // nothing
}

// put your main code here, will be run repeatedly
void loop() {
    // nothing
}

void message_rx(message_t *m, distance_measurement_t *d) {
    printf("Distance: %d\n", estimate_distance(d));  // REMOVE LATER
    printf("Kilobot UID: %u\n", kilo_uid);  // REMOVE LATER
}

int main() {
  kilo_init();
  debug_init();
  kilo_start(setup, loop);

  return 0;
}
