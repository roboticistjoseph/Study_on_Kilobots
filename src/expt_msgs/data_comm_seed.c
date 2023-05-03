/**
 * @file data_comm_seed.c
 * @author Joseph
 * @brief Experiment to test data communication with actual seed
 * @version 0.1
 * @date 2023-05-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// kilolib library
#include "../../kilolib/kilolib.h"
#define DEBUG
#include "../../kilolib/debug.h"

// communication message
message_t message;

// flags
int message_sent;  // keep track of message transmission
int ring_status_from_star_robot = 0;  // 0 = no ring, 1 = ring
int distance = 10000;  // distance from star robot

#define ROBOTS_IN_FIRST_CIRCLE 3

// Parameters for Circle formation
#define DESIRED_DISTANCE  65
#define EPSILON           20

// LED colors
#define LED_OFF          RGB(0, 0, 0)
#define LED_WHITE        RGB(1, 1, 1)
#define LED_BLUE         RGB(0, 0, 2)
#define LED_CYAN         RGB(0, 2, 2)
#define LED_GREEN        RGB(0, 2, 0)
#define LED_YELLOW       RGB(2, 2, 0)
#define LED_RED          RGB(2, 0, 0)
#define LED_MAGENTA      RGB(2, 0, 2)

/**
 * @brief Kilobot Setup
 * 
 */
void setup() {
  // The type is always NORMAL.
  message.type = NORMAL;

  // indicate we are a planet
  message.data[0] = 1;

  // track & communicate the number of kilobots present in first circle
  message.data[1] = 0;

  // kilo_uid
  message.data[2] = kilo_uid;

  // It's important that the CRC is computed after the data has been set;
  // otherwise it would be wrong.
  message.crc = message_crc(&g->message);
}

/**
 * @brief Kilobot Loop
 * 
 */
void loop() {
    // -------------- Uncomment and comment out below if needed ----------------
    message.data[1] = 0;  // reset message data to 0

    // Blink LED magenta whenever a message is sent.
    if (message_sent == 1) {
        // Reset flag so LED is only blinked once per message.
        message_sent = 0;
        printf("Message Transmitted\n");  // REMOVE LATER
    }

    if ((distance >= (DESIRED_DISTANCE - EPSILON)) && (distance <= (DESIRED_DISTANCE + EPSILON))) {
            message.data[1] = 1;  // Ring 1 complete
            set_color(LED_BLUE);  // [INDICATION]: kilobot has reached it's destination
        }

    if (ring_status_from_star_robot == 1) {
        set_color(LED_WHITE);  // [INDICATION]: ring 1 has been formed
    } else if (ring_status_from_star_robot == 0) {
        set_color(LED_RED);  // [INDICATION]: ring 1 is not formed yet
    }
}

/**
 * @brief receiver callback function.
 * 
 * @param m  // message 
 * @param d  // distance 
 */
void message_rx(message_t *m, distance_measurement_t *d) {
  // getting msg from planet robot
  if (m->data[0] == 0) {
      printf("Message received from SEED robot...\n");  // REMOVE LATER
      distance = estimate_distance(d);
      // [CASE]: star robot saying that it has reached desired number of neighbors
        if (m->data[1] >= ROBOTS_IN_FIRST_CIRCLE) {
            // [UPDATE]: ring 1 has been formed
            ring_status_from_star_robot = 1;
        } else if (m->data[1] < ROBOTS_IN_FIRST_CIRCLE) {
            // [UPDATE]: ring 1 is not formed yet
            ring_status_from_star_robot = 0;
        }
  }
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

/**
 * @brief message success callback (called *after* a message is sent)
 * 
 */
void message_tx_success() {
    // set flag on message transmission.
    message_sent = 1;
}

int main() {
    kilo_init();
    debug_init();

    // Register the message_rx callback function.
    kilo_message_rx = message_rx;

    // Register the message_tx callback function.
    kilo_message_tx = message_tx;

    // Register the message_tx_success callback function.
    kilo_message_tx_success = message_tx_success;

    kilo_start(setup, loop);

    return 0;
}
