/**
 * @file move_to_light.c
 * @author Joseph Katakam (jkatak73@terpmail.umd.edu)
 *
 * @brief Code for each robot to move towards the direction of the brightest light source.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// kilolib library
#include "../../kilolib/kilolib.h"

// Constants for light following.
#define THRESH_LO 300
#define THRESH_HI 600

// Constants for motion handling function.
#define STOP 0
#define FORWARD 1
#define LEFT 2
#define RIGHT 3

// Initiate motion to Stop.
int current_motion = STOP;
// Initiate sensed light intensity to 0.
int current_light = 0;

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
 * @brief Function to sample light.
 * 
 */
void sample_light() {
    // variables to calculate Average
    int number_of_samples = 1;
    int sum = 0;

    // The ambient light sensor gives noisy readings.
    // To mitigate this, we take the average of 300 samples in quick succession.
    while (number_of_samples < 300) {
        // read value of ambient light from sensor.
        int sample = get_ambientlight();

        // -1 indicates a failed sample, which should be discarded.
        if (sample != -1) {
            sum = sum + sample;
            number_of_samples = number_of_samples + 1;
        }
    }

    // Update light reading with the computed average.
    current_light = sum / number_of_samples;
}

/**
 * @brief Kilobot Setup
 * 
 */
void setup() {
    // This ensures that the robot starts moving.
    set_motion(LEFT);
}

/**
 * @brief Kilobot Loop
 * 
 */
void loop() {
    sample_light();

    if (current_light < THRESH_LO) {
        set_motion(RIGHT);
    } else if (current_light > THRESH_HI) {
        set_motion(LEFT);
    }
}

int main() {
    kilo_init();

    kilo_start(setup, loop);

    return 0;
}
