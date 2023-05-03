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

// communication message
message_t message;

// put your setup code here, will be run once at the beginning
void setup() {
    // The type is always NORMAL.
    message.type = NORMAL;

    // kilo_uid
    message.data[2] = kilo_uid;

    // It's important that the CRC is computed after the data has been set;
    // otherwise it would be wrong.
    message.crc = message_crc(&g->message);
}

// put your main code here, will be run repeatedly
void loop() {
    // do nothing
}

/**
 * @brief receiver callback function.
 * 
 * @param m 
 * @param d 
 */
void message_rx(message_t *m, distance_measurement_t *d) {
    printf("Kilobot UID: %u\n", m->data[2]);  // REMOVE LATER
    printf("Distance: %d\n", estimate_distance(d));  // REMOVE LATER
}

/**
 * @brief transmitter callback function.
 * 
 * @return message_t* 
 */
message_t *message_tx() {
    // message is transmitted roughly twice per sec
    return &(message);
}

int main() {
    kilo_init();

    // Register the message_rx callback function.
    kilo_message_rx = message_rx;

    // Register the message_tx callback function.
    kilo_message_tx = message_tx;

    #ifdef DEBUG
    debug_init();
    #endif

    kilo_start(setup, loop);

    return 0;
}
