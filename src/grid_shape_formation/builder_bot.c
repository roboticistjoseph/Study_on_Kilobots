/**
 * @file builder_bot.c
 * @author Joseph Katakam (jkatak73@terpmail.umd.edu)
 *
 * @brief Code to Broadcast a fixed message as a builder, and act as a reference for grid formation
 * @version 0.1
 * @date 2023-03-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// kilolib library
#include "../../kilolib/kilolib.h"

// variable declaration

// message to be transmitted.
message_t transmit_msg;

// flag to keep track of message status.
int message_sent = 0;

// flag to keep track of total messages sent.
int total_messages_sent = 0;

/**
 * @brief transmitter callback function.
 * 
 * @return message_t* 
 */
message_t *message_tx() {
  return &transmit_msg;
}

/**
 * @brief message success callback (called *after* a message is sent)
 * 
 */
void message_tx_success() {
  message_sent = 1;
  total_messages_sent +=1;
}

/**
 * @brief Kilobot Setup
 * 
 */
void setup() {
    // Initialize message:

    // message Identifier
    transmit_msg.type = NORMAL;

    // kilobot UID - message Payload
    transmit_msg.data[0] = 0;

    // It's important that the CRC is computed after the data has been set;
    // otherwise it would be wrong.
    transmit_msg.crc = message_crc(&transmit_msg);
}

/**
 * @brief Kilobot Loop
 * 
 */
void loop() {
    // LED blinks Megenta when received a new message.
    if (message_sent ==1) {
        // Reset flag so LED is only blinked once per message.
        message_sent = 0;

        set_color(RGB(1, 0, 1));
        delay(1000);
        set_color(RGB(0, 0, 0));
    }
}

int main() {
    kilo_init();

    // Register the message_tx callback function.
    kilo_message_tx = message_tx;
    // Register the message_tx_success callback function.
    kilo_message_tx_success = message_tx_success;

    kilo_start(setup, loop);

    return 0;
}
