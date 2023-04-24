/**
 * @file orbit_planet.c
 * @author Joseph Katakam (jkatak73@terpmail.umd.edu)
 *
 * @brief Code to use distance sensing to have one robot orbit another stationary robot while keeping a fixed distance.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// kilolib library
#include "../../kilolib/kilolib.h"

// Constants for orbit control.
#define TOO_CLOSE_DISTANCE 40
#define DESIRED_DISTANCE 60

// Constants for motion handling function.
#define STOP 0
#define FORWARD 1
#define LEFT 2
#define RIGHT 3

// Initiate motion to Stop.
int current_motion = STOP;
// store distance measured.
int distance_calculated;
// Flag to keep track of new messages.
int new_message = 0;

/**
 * @brief Function to handle motion.
 * 
 * @param new_motion 
 */
void set_motion(int new_motion) {
    // Only take an action if the motion is being changed.
    if (current_motion != new_motion) {
        current_motion = new_motion;

        // Motion cases
        if (current_motion == STOP) {
            // STOP
            set_motors(0, 0);
        } else if (current_motion == FORWARD) {
            // STRAIGHT
            spinup_motors();
            set_motors(kilo_straight_left, kilo_straight_right);
        } else if (current_motion == LEFT) {
            // LEFT
            spinup_motors();
            set_motors(kilo_turn_left, 0);
        } else if (current_motion == RIGHT) {
            // RIGHT
            spinup_motors();
            set_motors(0, kilo_turn_right);
        }
    }
}

/**
 * @brief receiver callback function.
 * 
 * @param message 
 * @param distance 
 */
void message_rx(message_t *message, distance_measurement_t *distance) {
    // store the incoming message.
    message_t rcvd_message = *message;

    // set the flag to 1 to indicate that a new message arrived.
    new_message = 1;

    // store the distance measured for sending kilobot.
    distance_calculated = estimate_distance(distance);
}

/**
 * @brief Kilobot Setup
 * 
 */
void setup() {
}

/**
 * @brief Kilobot Loop
 * 
 */
void loop() {
    // Update the motion whenever a message is received.
    if (new_message == 1) {
        // Reset flag
        new_message = 0;

        // If too close, move forward to get back into orbit.
        if (distance_calculated < TOO_CLOSE_DISTANCE) {
            set_color(RGB(1, 0, 0));
            set_motion(FORWARD);
        } else {
            // If not too close, turn left or right depending on distance, to maintain orbit.
            if (distance_calculated < DESIRED_DISTANCE) {
                set_color(RGB(0, 1, 0));
                set_motion(LEFT);
            } else {
                set_color(RGB(0, 0, 1));
                set_motion(RIGHT);
            }
        }
    }
}

int main() {
    kilo_init();

    // Register the message_rx callback function.
    kilo_message_rx = message_rx;

    kilo_start(setup, loop);

    return 0;
}
