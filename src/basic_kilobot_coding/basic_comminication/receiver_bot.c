/**
 * @file receiver_bot.c
 * @author Joseph Katakam (jkatak73@terpmail.umd.edu)
 *
 * @brief Code to Blink yellow when a new message is recieved.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// kilolib library
#include "../../../kilolib/kilolib.h"

// variable declaration

// store new incoming messages.
message_t rcvd_message;

// Flag to keep track of new messages.
int new_message = 0;

// flag to keep track of total messages received.
int total_messages_received = 0;

/**
 * @brief receiver callback function.
 * 
 * @param message 
 * @param distance 
 */
void message_rx(message_t *message, distance_measurement_t *distance) {
    // store the incoming message
    rcvd_message = *message;

    // set the flag to 1 to indicate that a new message arrived
    new_message = 1;

    // flag to keep track of total messages received.
    total_messages_received += 1;
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
    // Blink led yellow when you get a message
    if (new_message == 1) {
        new_message = 0;
        set_color(RGB(1, 1, 0));
        delay(100);
        set_color(RGB(0, 0, 0));
    }
}

int main() {
    kilo_init();

    // Register the message_rx callback function.
    kilo_message_rx = message_rx;

    kilo_start(setup, loop);

    return 0;
}
