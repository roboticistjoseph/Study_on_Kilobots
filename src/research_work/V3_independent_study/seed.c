/**
 * @file seed.c
 * @author Jared and Joseph
 *
 * @brief Single's and DUO codebase: Seed kilobot code for hardware experimentation. [Independent Study Team]
 * @version 0.9
 * @date 2023-04-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// standard libraries
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// kilolib library
#include "../../../kilolib/kilolib.h"
#define DEBUG
#include "../../../kilolib/debug.h"

// define number of kilobots used in the experiment
/**
 * @brief Number of robots in the simulation: 46
 *        Number of robots in the experiment:
 *                                   Ideally: 8
 *                                   Minimum: 0-1
 *                                   Maximum: 10 or 12
 * 
 */
#define NUMBER_OF_ROBOTS 8


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
 * @brief Global structure to hold the variable used by algorithm.
 * 
 */
struct GLOBALS {
  // global variables
  // NOTE: the use of a GLOBALS struct will also work on a normal kilobot,
  //       but is not required on a normal kilobot.
  //       It is, however, required by the simulator.

  message_t message;

  // flags
  int message_sent;  // keep track of message transmission.

  // container to track the number of kilobots in first circle
  int stopped_robots_array[NUMBER_OF_ROBOTS];

  // container to track the time of arrival of kilobots in first circle
  int time_array[NUMBER_OF_ROBOTS];
}* g;  // there should only be one GLOBAL, this is it, remember to register it in main()


/**
 * @brief Kilobot Setup
 * 
 */
void setup() {
  // The type is always NORMAL.
  g->message.type = NORMAL;

  // indicate we are a star
  g->message.data[0] = 0;

  // track & communicate the number of kilobots present in first circle
  g->message.data[1] = 0;

  // It's important that the CRC is computed after the data has been set;
  // otherwise it would be wrong.
  g->message.crc = message_crc(&g->message);

  // set all the flags to uninitialized state
  int i;
  for (i = 0; i <= NUMBER_OF_ROBOTS-1; i++) {
    // g->stopped_robots_array[i] = -1;  // modified initialization  // REMOVE LATER
    g->stopped_robots_array[i] = 0;  // init to a large number  // REMOVE LATER
    g->time_array[i] = 0;
  }
}

/**
 * @brief Kilobot Loop
 * 
 */
void loop() {
//   set_color(RGB(1, 0, 1));

  // Blink LED magenta whenever a message is sent.
  if (g->message_sent == 1) {
      // Reset flag so LED is only blinked once per message.
      g->message_sent = 0;

      set_color(LED_MAGENTA);
      delay(1000);
      set_color(LED_OFF);
      printf("NEW Message sent\n");  // REMOVE LATER
  }

  int n_counter = 0;  // counts the kilobots in the circle

  for (int i = 0; i <= NUMBER_OF_ROBOTS-1; i++) {
    // here we are checking if the we heard from a robot in the last 40 ticks
    // if not heard then that robot must not be in the stopped radius
    // hence set its value in the array = 0 so we can only count the number of
    // robots which are stopped.
    if ((kilo_ticks - (g->time_array[i])) > 40) {
      g->stopped_robots_array[i] = 0;
    }

    // update counter when kilobot is present in first circle
    if (g->stopped_robots_array[i] == 1) {
      n_counter++;
      printf("Kilobot %d is in first circle\n", i);  // REMOVE LATER
    }
  }

  // communicate to other kilobots, the number of kilobots in first circle
  g->message.data[1] = n_counter;
  printf("Number of kilobots: %d\n", g->message.data[1]);  // REMOVE LATER
  delay(1000);

  // debugging mechanism to count planets in first circle  // REMOVE LATER
  switch (g->message.data[1]) {
      case 0:
          set_color(LED_OFF);
          printf("No kilobots in first circle\n");  // REMOVE LATER
          break;
      case 1:
          set_color(LED_GREEN);
          printf("1 kilobot in first circle\n");  // REMOVE LATER
          break;
      case 2:
          set_color(LED_YELLOW);
          printf("2 kilobots in first circle\n");  // REMOVE LATER
          break;
      case 3:
          set_color(LED_RED);
          printf("3 kilobots in first circle\n");  // REMOVE LATER
          break;
      case 4:
          set_color(LED_MAGENTA);
          printf("4 kilobots in first circle\n");  // REMOVE LATER
          break;
      case 5:
          set_color(LED_CYAN);
          printf("5 kilobots in first circle\n");  // REMOVE LATER
          break;
      case 6:
          set_color(LED_BLUE);
          printf("6 kilobots in first circle\n");  // REMOVE LATER
          break;
      case 7:
          set_color(LED_WHITE);
          printf("7 kilobots in first circle\n");  // REMOVE LATER
          break;
      default:
          set_color(LED_OFF);
          printf("Default or More than 7 kilobots in first circle\n");  // REMOVE LATER
          break;
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
  if (m->data[0] == 1) {
    // NOTE: m->data[2] has the kilobot uid that is in communication
    printf("ID received from planet robot in %u\n", m->data[2]);  // REMOVE LATER
    // if the planet robot is part of first ring
    if (m->data[1] == 1) {
      printf("First circle updated!! :)\n");  // REMOVE LATER
      g->time_array[m->data[2]] = kilo_ticks;  // set the time when the planet robot stopped
      g->stopped_robots_array[m->data[2]] = 1;  // setting value of corresponging uid robot in array to 1
    } else if (m->data[1] == 0) {
      // if the planet robot is not part of first ring
      g->stopped_robots_array[m->data[2]] = 0;
      printf("Received robot is not yet part of first circle. :(\n");  // REMOVE LATER
    }
  }
}

/**
 * @brief transmitter callback function.
 * 
 * @return message_t* 
 */
message_t *message_tx() {
  // perform CRC on the message, as the values are updated
  g->message.crc = message_crc(&g->message);

  // message is transmitted roughly twice per sec
  return &(g->message);
}

/**
 * @brief message success callback (called *after* a message is sent)
 * 
 */
void message_tx_success() {
  // set flag on message transmission.
  g->message_sent = 1;
}


int main() {
  // Create user defined globals
  struct GLOBALS* g_safe =  (struct GLOBALS*)malloc(sizeof(struct GLOBALS));

  // Initialize kilobot.
  kilo_init();

  // Debugging mechanism
  #ifdef DEBUG
    debug_init();
  #endif

  // Register the message_rx callback function.
  kilo_message_rx = message_rx;

  // Register the message_tx callback function.
  kilo_message_tx = message_tx;

  // Register the message_tx_success callback function.
  kilo_message_tx_success = message_tx_success;

  // Register user defined global structure.
  g = g_safe;

  // Start kilobot.
  kilo_start(setup, loop);

  // free user defined globals
  free(g_safe);

  return 0;
}
