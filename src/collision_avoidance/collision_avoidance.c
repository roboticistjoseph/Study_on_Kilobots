/**
 * @file collision_avoidance.c
 * @author Joseph Katakam
 *
 * @brief Code to avoid collision and perform intended task
 * @version 0.1
 * @date 2023-04-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// standard libraries
// #include <stdbool.h>

// kilolib library
#include "../../kilolib/kilolib.h"
#define DEBUG
#include "../../kilolib/debug.h"

// ---- CONSTANTS ----
// Define the threshold distance to detect other kilobots.
#define THRESHOLD_DISTANCE 30
// Define the distance to move away from another kilobot.
#define AVOID_DISTANCE 40

// variable to track the kilobot's previous motion task.
int previous_motion = -1;

// flag to indicate whether the kilobot is avoiding a collision.
int collision_flag = 0;

// flag to store distance
uint8_t distance = -1;

// LED colors (Used as debugging mechanism)
#define LED_OFF          RGB(0, 0, 0)  // [INDICATION]: kilobot not in communication range
#define LED_WHITE        RGB(1, 1, 1)  // [INDICATION]: Circle formed
#define LED_BLUE         RGB(0, 0, 2)  // [INDICATION]: kilobot has reached it's destination
#define LED_CYAN         RGB(0, 2, 2)  // [INDICATION]: planet is about to enter orbit
#define LED_GREEN        RGB(0, 2, 0)  // [INDICATION]: kilobot is in communication range with others
#define LED_YELLOW       RGB(2, 2, 0)  // [INDICATION]: kilobot very far from communication range
#define LED_RED          RGB(2, 0, 0)  // [INDICATION]: planet is close to colliding with Seed robot
#define LED_MAGENTA      RGB(2, 0, 2)  // [INDICATION]: Star or Seed kilobot

// Define Move Set states
#define FORWARD  0
#define LEFT     1
#define RIGHT    2
#define STOP     3

/**
 * @brief Function to handle Kilobot Move Set: [MOTOR ACTION]
 * 
 * @param new_motion 
 * @param duration 
 */
void move(int new_motion, int duration = 500) {
    switch (new_motion) {
        case FORWARD:
            previous_motion = FORWARD;
            // Go STRAIGHT
            spinup_motors();
            set_motors(kilo_straight_left, kilo_straight_right);
            delay(duration);
            set_motors(0, 0);
            delay(duration);
            break;
        case LEFT:
            previous_motion = LEFT;
            // Turn LEFT
            spinup_motors();
            set_motors(kilo_straight_left, 0);
            delay(duration);
            set_motors(0, 0);
            delay(duration);
            break;
        case RIGHT:
            previous_motion = RIGHT;
            // Turn RIGHT
            spinup_motors();
            set_motors(0, kilo_straight_right);
            delay(duration);
            set_motors(0, 0);
            delay(duration);
            break;
        case STOP:
            previous_motion = STOP;
            // STOP
            set_motors(0, 0);
            delay(duration);
            break;
    }
}

// Define a function to handle the kilobot's collision avoidance.
void avoid_collision() {
  move(LEFT, 500);
  collision_flag = 1;
}

// Define a function to handle the kilobot's previous motion task.
void continue_previous_motion() {
  move(previous_motion, 500);
  collision_flag = 0;
}

/**
 * @brief Kilobot Setup (will be run once at the beginning)
 * 
 */
void setup() {
    // Nothing for now
}

/**
 * @brief Kilobot Loop (runs continuosly)
 * 
 */
void loop() {
    // Check if the kilobot is avoiding a collision.
    if (collision_flag) {
        // Check if the kilobot has moved away from the other kilobot.
        if (distance > AVOID_DISTANCE) {
            // Continue with the previous motion task.
            continue_previous_motion();
        }
    }
    // Delay for 10 milliseconds.
    delay(10);

    // basic movement
    set_color(LED_GREEN);  // [INDICATION]: kilobot moving straight
    move(FORWARD, 1000);
    set_color(LED_YELLOW);  // [INDICATION]: kilobot turning left
    move(LEFT, 1000);
}

/**
 * @brief receiver callback function.
 *        Define a function to handle the kilobot's message reception.
 * 
 * @param m  // message 
 * @param d  // distance 
 */
void message_rx(message_t *m, distance_measurement_t *d) {
  // Check if the message is from another planet kilobot.
  if (m->data[0] == 1) {
    // Calculate the distance from that kilobot
    distance = estimate_distance(d);

    // Check if the distance to the other kilobot is less than the threshold distance.
    if (distance < THRESHOLD_DISTANCE) {
      // Avoid the collision.
      avoid_collision();
    }
  }
}

// Main function.
int main() {
    // Initialize the kilobot.
    kilo_init();

    // Register the message_rx callback function.
    kilo_message_rx = message_rx;

    kilo_start(setup, loop);
    return 0;
}
