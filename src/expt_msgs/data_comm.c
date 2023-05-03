/**
 * @file data_comm.c
 * @author Joseph
 * @brief Experiment to test data communication
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
int looper = -1;  // keep track of loop

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

  // indicate we are a star
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

    if (looper == 0 || looper == 1) {
        set_color(LED_RED);
        printf("Initial Loop\n");  // REMOVE LATER
        message.data[1] = 0;
        delay(1000);
    }

    if (looper == 2) {
        set_color(LED_BLUE);
        printf("Data[1] Updating...\n");  // REMOVE LATER
        message.data[1] = 1;
        delay(1000);
    }

    if (looper == 10) {
        set_color(LED_GREEN);
        printf("Data[1] Updating...\n");  // REMOVE LATER
        message.data[1] = 2;
        delay(1000);
        // looper = -1;
    }

    looper++;
}

/**
 * @brief receiver callback function.
 * 
 * @param m  // message 
 * @param d  // distance 
 */
void message_rx(message_t *m, distance_measurement_t *d) {
  // getting msg from planet robot
  if (m->data[0] == 1) {
      printf("Message received from planet robot %u\n", m->data[2]);  // REMOVE LATER
    // Check for data[1] update
    // if (m->data[1] == 1 || m->data[1] == 2) {
    //   printf("Data[1] Updated\n");  // REMOVE LATER
    //   // REMOVE LATER
    //   set_color(LED_CYAN);
    //   delay(1000);
    //   set_color(LED_OFF);
    // } else if (m->data[1] == 0) {
    //   printf("Data[1] not updated\n");  // REMOVE LATER
    // }
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

    // Register the message_rx callback function.
    kilo_message_rx = message_rx;

    // Register the message_tx callback function.
    kilo_message_tx = message_tx;

    // Register the message_tx_success callback function.
    kilo_message_tx_success = message_tx_success;

    #ifdef DEBUG
    debug_init();
    #endif

    kilo_start(setup, loop);

    return 0;
}
