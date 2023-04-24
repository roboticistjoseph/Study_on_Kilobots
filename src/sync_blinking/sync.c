/**
 * @file sync.c
 * @author Joseph Katakam (jkatak73@terpmail.umd.edu)
 *
 * @brief Code to Create a logical synchronous clock between different robots 
 *        to allow two or more robots to blink an LED in unison roughly every 4 seconds.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// kilolib library
#include "../../kilolib/kilolib.h"

// Nominal period with which to blink; approximately 2 seconds. This will be
// be the period of every robot once the swarm has synchronized.
#define PERIOD 50
// Affects the size of the reset time adjustment for every discrepancy
// with a neighbor. A larger value means smaller adjustments. As a rule of thumb
// this value should increase with the average number of neighbors each robot has.
#define RESET_TIME_ADJUSTMENT_DIVIDER 120
// A cap on the absolute value of the reset time adjustment.
#define RESET_TIME_ADJUSTMENT_MAX 30

// Flags
uint32_t reset_time = 0;
uint32_t last_reset = 0;
int reset_time_adjustment = 0;

// kilobot message structure
message_t message;

/**
 * @brief transmitter callback function.
 * 
 * @return message_t* 
 */
message_t *message_tx() {
    return &message;
}

/**
 * @brief receiver callback function.
 * 
 * @param m 
 * @param d 
 */
void message_rx(message_t *m, distance_measurement_t *d) {
    // current timer count
    int my_timer = kilo_ticks - last_reset;
    // read the timer count for neighbouring kilobot
    int rx_timer = m->data[0];
    // lag in timer count
    int timer_discrepancy = my_timer - rx_timer;

    // Reset time adjustment due to this message - to be combined with the
    // overall reset time adjustment.
    int rx_reset_time_adjustment = 0;

    if (timer_discrepancy > 0) {
        // The neighbor is trailing behind: move the reset time forward
        // (reset later).
        if (timer_discrepancy < (PERIOD / 2)) {
            rx_reset_time_adjustment = timer_discrepancy;
        } else {
            // The neighbor is running ahead: move the reset time backward
            // (reset sooner).
            rx_reset_time_adjustment = - (PERIOD - timer_discrepancy) % PERIOD;
        }
    } else if (timer_discrepancy < 0) {
        // The neighbor is running ahead: move the reset time backward
        // (reset sooner).
        if (- timer_discrepancy < (PERIOD / 2)) {
            rx_reset_time_adjustment = timer_discrepancy;
        } else {
            // The neighbor is trailing behind: move the reset time forward
            // (reset later).
            rx_reset_time_adjustment = (PERIOD + timer_discrepancy) % PERIOD;
        }
    }

    // Combine the reset time adjustment due to this message with the overall
    // reset time adjustment.
    reset_time_adjustment = reset_time_adjustment + rx_reset_time_adjustment;
}

/**
 * @brief Kilobot Setup
 * 
 */
void setup() {
    // Set the message.
    message.type = NORMAL;
    message.data[0] = 0;
    message.crc = message_crc(&message);

    // Introduce a random delay so the robots don't become instantly
    // synchronized by the run signal.
    set_color(RGB(1, 0, 0));
    delay(10 * rand_hard());
    set_color(RGB(0, 0, 0));
}

/**
 * @brief Kilobot Loop
 * 
 */
void loop() {
    if (kilo_ticks >= reset_time) {
        reset_time_adjustment = (reset_time_adjustment / RESET_TIME_ADJUSTMENT_DIVIDER);

        // Apply a cap to the absolute value of the reset time adjustment.
        if (reset_time_adjustment < - RESET_TIME_ADJUSTMENT_MAX) {
            reset_time_adjustment = - RESET_TIME_ADJUSTMENT_MAX;
        } else if (reset_time_adjustment > RESET_TIME_ADJUSTMENT_MAX) {
            reset_time_adjustment = RESET_TIME_ADJUSTMENT_MAX;
        }

        last_reset = kilo_ticks;
        reset_time = kilo_ticks + PERIOD + reset_time_adjustment;

        reset_time_adjustment = 0;

        // Set the LED white and turn the motors on.
        set_color(RGB(1, 1, 1));
        set_motors(150, 150);
    } else if (kilo_ticks > (last_reset + 1)) {
        // After 1 clock tick, turn the LED and motors off.
        set_color(RGB(0, 0, 0));
        set_motors(0, 0);
    }

    // Only send the current time if it can fit in 1 byte (8 bits), which
    // corresponds to a maximum of 2^8 - 1 = 255. Otherwise, set the message
    // CRC to 0 so neighbors ignore the message.
    if ((kilo_ticks - last_reset) < 255) {
        message.data[0] = kilo_ticks - last_reset;
        message.crc = message_crc(&message);
    } else {
        message.crc = 0;
    }
}

int main() {
    kilo_init();

    // Register the message_rx callback function.
    kilo_message_rx = message_rx;
    // Register the message_tx callback function.
    kilo_message_tx = message_tx;


    kilo_start(setup, loop);

    return 0;
}
