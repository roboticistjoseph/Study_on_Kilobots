/**
 * @file single_bots.c
 * @author Jared and Joseph
 *
 * @brief Single's Codebase: Planet kilobot code for hardware experiment. [Independent study team]
 * @version 0.4
 * @date 2023-04-07
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

// preprocessor directives to assign BIT values
#define SetBit(A, k) (A[(k / 32)] |= (1 << (k % 32)))     // sets the kth bit in array A
#define ClearBit(A, k) (A[(k / 32)] &= ~(1 << (k % 32)))  // resets (clears) the kth bit in array A
#define TestBit(A, k) (A[(k / 32)] & (1u << (k % 32)))    // checks if we have 1 at kth position

// define number of robots in the experiment
/**
 * @brief Number of robots in the experiment:
 *                                   First Circle: 2
 *                                   Second Circle: 10
 *
 * The number of kilobots should be an even number, as the DUOs exist as pairs.
 * 
 */
#define ROBOTS_IN_FIRST_CIRCLE 4
#define ROBOTS_IN_SECOND_CIRCLE 10
#define TOTAL_KILOBOTS 4  // Value 1: 16robots, Value 2: 32robots, Value 3: 48robots, Value 4: 64robots

// Parameters for Circle formation
#define DESIRED_DISTANCE  55
#define EPSILON           15
// #define MIN_DISTANCE      35  // Not used
#define MAX_DISTANCE      90

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
void move(int new_motion, int duration) {
    switch (new_motion) {
        case FORWARD:
            // Go STRAIGHT
            spinup_motors();
            set_motors(kilo_straight_left, kilo_straight_right);
            delay(duration);
            set_motors(0, 0);
            delay(duration);
            break;
        case LEFT:
            // Turn LEFT
            spinup_motors();
            set_motors(kilo_straight_left, 0);
            delay(duration);
            set_motors(0, 0);
            delay(duration);
            break;
        case RIGHT:
            // Turn RIGHT
            spinup_motors();
            set_motors(0, kilo_straight_right);
            delay(duration);
            set_motors(0, 0);
            delay(duration);
            break;
        case STOP:
            // STOP
            set_motors(0, 0);
            delay(duration);
            break;
    }
}

/**
 * @brief GLobal structure to hold the variable used by algorithm.
 * 
 */
struct GLOBALS {
    // global variables
    // NOTE: the use of a GLOBALS struct will also work on a normal kilobot,
    //       but is not required on a normal kilobot.
    //       It is, however, required by the simulator.
    // int current_motion;
    int loop_count;
    int distance;
    int new_message;
    int timer;
    int last_changed;
    int ring_status_from_star_robot;
    int ring_status_from_planet_robot;
    int message_from_stopped_L1_robot;
    int new_message_from_L1_robot;
    int my_stop_status;
    int distance_from_L1_robot;
    int previous_dist_from_planet_robot;
    // int layer_2_robot_done;
    int new_message_from_L2_robot;
    int distance_from_L2_robot;

    //**************************************

    // Since kilobot uses 8bit AVR microcontroller, 'int' can only store 16bits of data.
    int my_array[TOTAL_KILOBOTS];    // holds bits for 64 robots 4kilobots*(2bytes*8bits)  int is 2bytes
    int rcvd_array[TOTAL_KILOBOTS];  // holds bits for 64 robots 4kilobots*(2bytes*8bits)  int is 2bytes

    // [FIX THIS LATER]---------- Below two lines cause error for outgoing message ----------
    // int array_bits = sizeof(my_array) * 8;  // holds the bit value of the array
    // int array_divisor = array_bits/TOTAL_KILOBOTS;  // holds the divisor for array_bits

    // int time_array[NUM_R];  // REMOVE LATER

    int y;
    int rcvd_count;
    int my_count;
    int my_ring_number;
    int previous_dist_from_L2;
    int heard_from_L2;

    //***************************************

    message_t outgoing_message;
    uint8_t B[4];
    uint32_t mask1;
    uint32_t mask2;
    uint32_t mask3;
    uint32_t mask4;
    uint8_t i1;
    uint8_t i2;
    uint8_t i3;
    uint8_t i4;
    int count;
} * g;  // there should only be one GLOBAL, this is it, remember to register it in main()

// /**
//  * @brief structure to hold the pair robot information.
//  *
//  */
// typedef struct DUO {
//     uint8_t uid;
//     uint8_t pairUid;
//     uint8_t hasIdentifiedPair;
// } DUO;

/**
 * @brief Kilobot Setup (will be run once at the beginning)
 * 
 */
void setup() {
    // Initializing Varibales
    //**********************************************************
    for (int n = 0; n < TOTAL_KILOBOTS; n++) {
        g->my_array[n] = 0;
        g->rcvd_array[n] = 0;
    }

    // REMOVE LATER
    // for (int r = 0; r <= NUM_R - 1; r++) {
    //     g->time_array[r] = 0;
    // }

    g->y = 0;
    g->rcvd_count = 0;
    g->my_count = 0;

    g->outgoing_message.data[4] = 0;  // for rcvd array
    g->outgoing_message.data[5] = 0;  // for rcvd array
    g->outgoing_message.data[6] = 0;  // for rcvd array
    g->outgoing_message.data[7] = 0;
    g->outgoing_message.data[8] = 0;  // for counter value
                                      // g->messgae.data[8]=0;  // layer number
    g->my_ring_number = 0;
    g->previous_dist_from_L2 = 0;
    g->heard_from_L2 = 0;
    //**********************************************************

    g->distance = 10000;
    g->loop_count = -1;
    g->new_message = 0;
    g->timer = kilo_ticks = 0;
    g->last_changed = 0;
    g->ring_status_from_star_robot = 0;
    g->ring_status_from_planet_robot = 0;
    g->message_from_stopped_L1_robot = 0;
    g->new_message_from_L1_robot = 0;
    g->my_stop_status = 0;
    g->distance_from_L1_robot = 10000;
    g->previous_dist_from_planet_robot = 0;
    // g->layer_2_robot_done = 0;  // REMOVE LATER
    g->new_message_from_L2_robot = 0;
    g->distance_from_L2_robot = 10000;

    g->mask1 = 0xFF000000;
    g->mask2 = 0x00FF0000;
    g->mask3 = 0x0000FF00;
    g->mask4 = 0x000000FF;

    g->i1 = 0;
    g->i2 = 0;
    g->i3 = 0;
    g->i4 = 0;

    g->count = 0;

    /** 
     * NOTE: Because we are never changing the data in the message in this example
     *       then we only need to do this once, otherwise we'd need to do it every time we
     *       wanted to send a new message.
     */

    // The type is always NORMAL.
    g->outgoing_message.type = NORMAL;

    // indicate we are a planet
    g->outgoing_message.data[0] = 1;

    // saying we stopped
    g->outgoing_message.data[1] = 0;  // my ring number

    // kilo_uid
    g->outgoing_message.data[2] = kilo_uid;

    // ring status to tell other robot planets
    g->outgoing_message.data[3] = 0;

    // It's important that the CRC is computed after the data has been set;
    // otherwise it would be wrong.
    g->outgoing_message.crc = message_crc(&g->outgoing_message);
}

void loop() {
    printf("Distance: %d\n", g->distance);  // REMOVE LATER
    // update looper which tracks the time since the kilobot started
    g->loop_count++;

    // pause when starting (to finish experimental setup)
    if (g->loop_count == 0) {
        delay(9000);
    }

    // ring status is incomplete (reset) for every new iteration
    // g->outgoing_message.data[1] = 0;  // is this needed? [FIX LATER]
    g->my_stop_status = 0;

    move(STOP, 200);  // [MOTOR ACTION]: stop previous motor action for every new iteration
    set_color(LED_OFF);  // [INDICATION]: Color indicates that kilobot not in communication range of others

    // [CASE]: when a new message is received
    if (g->new_message == 1) {
        g->new_message = 0;  // reset message flag

     // OTTE -------- START different distance based cases for first circle -----

        // [CASE]: Kilobot reached desired orbit (Define Stopping Criterion)
        if ((g->distance >= (DESIRED_DISTANCE - EPSILON)) && (g->distance <= (DESIRED_DISTANCE + EPSILON))) {
            // [UPDATE]: First circle is formed
            g->my_ring_number = 1;
            g->outgoing_message.data[1] = 1;  // Ring 1 complete
            g->my_stop_status = 1;  // stop algorithm

            set_color(LED_BLUE);  // [INDICATION]: kilobot has reached it's destination
            move(STOP, 1000);  // [MOTOR ACTION]: stop motors
            delay(2000);  // [DELAY]: wait for 2 second // dont rush
            // g->messgae.data[8]=1;  // I'm ring 1  // REMOVE LATER
        } else if ((g->distance < (DESIRED_DISTANCE - EPSILON)) &&
            // [CASE]: when kilobot is very close to the Seed robot
             g->ring_status_from_star_robot == 0 &&
             g->ring_status_from_planet_robot == 0) {
            set_color(LED_RED);  // [INDICATION]: planet is close to colliding with Seed robot
            g->my_stop_status = 0;  // continue algorithm

            // [MOTOR ACTION]: kilobot has to turn around and go backwards (away from collision range of Seed)
            if (kilo_uid < TOTAL_KILOBOTS / 2) {
                move(FORWARD, 2000);  // [MOTOR ACTION]: Left kilobot goes Straight
            } else if (kilo_uid >= TOTAL_KILOBOTS / 2) {
                move(STOP, 2000);  // [MOTOR ACTION]: Right kilobot stops
            }

            move(FORWARD, 650);
            delay(2000);  // [DELAY]: wait for 2 second // dont rush
        } else if ((g->distance > (DESIRED_DISTANCE + EPSILON)) && (g->distance <= MAX_DISTANCE)) {
            // [Case]: when planet is close to the orbit but not in the orbit
            set_color(LED_CYAN);  // [INDICATION]: planet is about to enter orbit
            g->my_stop_status = 0;  // REMOVE LATER?

            move(FORWARD, 150);  // [MOTOR ACTION]: move straight
        }

     // OTTE -------- END different distance based cases for first circle

        // [CHECK]: Circle 1 formation (OTTE)
        if (g->ring_status_from_star_robot == 1) {
            // [CASE]: if first circle formed, indicate other bots
            g->outgoing_message.data[3] = 1;
            set_color(LED_WHITE);  // [INDICATION]: Circle formed

            // REMOVE LATER
            // if (g->distance < (DESIRED_DISTANCE + EPSILON)) {
            //     set_color(LED_WHITE);  // [INDICATION]: Circle formed
            //     move(STOP, 1000);  // [MOTOR ACTION]: Stop
            // }
        } else {
            // [CASE]: first circle not formed
            g->outgoing_message.data[3] = 0;
        }
    }

// //------------------------------------------------------------------------------------------------------------
//     // [CASE]: Recieved message from robot on first circle (OTTE)
//     if (g->new_message_from_L1_robot == 1) {
//         g->new_message_from_L1_robot = 0;  // reset the status

//         // OTTE ------- START different cases for circle 1 and 2 formation given a message from a robot on circle 1

//         // [CASE]: Steering for formation of L1 (circle 1)
//         if (g->message_from_stopped_L1_robot == 1 &&
//             g->my_stop_status == 0 &&
//             g->ring_status_from_planet_robot == 0 &&
//             g->distance >= (DESIRED_DISTANCE + EPSILON)) {
//                 /**
//                 * @brief not stopped and ring not formed, and
//                 *        recieved message from a robot on circle 1,
//                 *        so move striaght and then turn. (OTTE)
//                 *
//                 */
//                 g->message_from_stopped_L1_robot = 0;  // Reset the flag

//                 set_color(LED_GREEN);  // [INDICATION]: in communication range
//                 move(FORWARD, 700);  // [MOTOR ACTION]: Go straight
//                 move(LEFT, 300);  // [MOTOR ACTION]: Turn Left
//         }

//         // [CASE]: L1 formed (circle 1)
//         if (g->message_from_stopped_L1_robot == 1 &&
//             g->my_stop_status == 0 &&
//             g->ring_status_from_planet_robot == 1 &&
//             g->distance > (DESIRED_DISTANCE + EPSILON)) {
//                 /**
//                  * @brief too far from circle 1, but we did hear from a circle 1
//                  *         robot, so just sleep? (OTTE)
//                  *
//                  */
//                 g->message_from_stopped_L1_robot = 0;  // Reset the flag


//                 // [CASE]: Ring 1 is formed, this robor is not stopped, and
//                 // it is far from the seed robot (OTTE)
//                 if (g->distance_from_L1_robot >= (DESIRED_DISTANCE)) {
//                     /**
//                     * @brief far from ring 1 robot, so aim to form ring 2 (OTTE)
//                     *
//                     */

//                     // [Update]: Stop status
//                     g->my_stop_status = 1;  // OTTE: not sure why we stop... perhaps to avoid scrweing things up?
//                     // [UPDATE]: indicating we this kilobot is part of L2
//                     g->my_ring_number = 2;
//                     g->outgoing_message.data[1] = g->my_ring_number;

//                     set_color(LED_YELLOW);  // [INDICATION]: Too far
//                     move(STOP, 1000);  // [MOTOR ACTION]: Stop
//                 }

//                 // [CASE]: Close to Ring 1 (L1)
//                 if (g->distance_from_L1_robot < (DESIRED_DISTANCE)) {
//                     /**
//                     * @brief close (< 28 away from circle 1) but we did hear from a circle 1 robot.
//                     *        Change this for "perfect-ness of the circle".
//                     *        Larger the value, better the circle but trades off with time
//                     *
//                     */

//                     // [CASE]: planet going away, it looks like we just move forward a bit
//                     if (g->distance_from_L1_robot > g->previous_dist_from_planet_robot) {
//                         // [UPDATE]: previous distance from kilobot
//                         g->previous_dist_from_planet_robot = g->distance_from_L1_robot;

//                         set_color(LED_CYAN);  // [INDICATION]: Atmost there, so move forward
//                         move(FORWARD, 1000);  // [MOTOR ACTION]: Move forward
//                         // g->my_stop_status=1;  // REMOVE LATER?
//                     }

//                     if (g->distance_from_L1_robot < g->previous_dist_from_planet_robot) {
//                         /**
//                         * @brief Going toward the circle 1 robot, we change stop status to stopped,
//                         *        and then pivot and then move straight (OTTE)
//                         *
//                         */

//                         // [UPDATE]: Stop status and previous distance Flag
//                         g->my_stop_status = 1;
//                         g->previous_dist_from_planet_robot = g->distance_from_L1_robot;

//                         set_color(LED_RED);  // [INDICATION]: Close to hitting other kilobots in L1
//                         move(LEFT, 1500);  // [MOTOR ACTION]: Turn left
//                         move(FORWARD, 500);  // [MOTOR ACTION]: Move forward
//                     }
//                 }
//         }
//     // OTTE ---------- END different cases for circle 1 and 2 formation given a message from a robot on circle 1
//     }

//     // [CASE]: got message from robot on circle 2 (OTTE)
//     if (g->new_message_from_L2_robot == 1) {
//         g->new_message_from_L2_robot = 0;  // reset flag status

//         // OTTE ---------- START different cases given a message from a robot on circle 1

//         // [CASE]: L1 has formed
//         if (g->my_stop_status == 1 &&
//             g->ring_status_from_planet_robot == 1 &&
//             g->distance > (DESIRED_DISTANCE + EPSILON)) {
//                 /**
//                  * @brief already stopped, and the circle 1 is already formed, and
//                  *        we're greater than 32 from, presumably, the robot on circle 2 (OTTE)
//                  *
//                  */
//                 // OR-ING (What the heck is this)  // REMOVE LATER?
//                 for (g->y = 0; g->y < 96; g->y++) {
//                     g->my_array[g->y / 32] = g->my_array[g->y / 32] | g->rcvd_array[g->y / 32];
//                 }

//                 g->i1 = (g->mask1&g->my_array[0])>>24;
//                 g->i2 = (g->mask2&g->my_array[0])>>16;
//                 g->i3 = (g->mask3&g->my_array[0])>>8;
//                 g->i4 = (g->mask4&g->my_array[0]);

//                 g->count++;
//                 if (g->count == 4) {
//                     g->count = 0;
//                 }


//                 if (g->rcvd_count > g->my_count) {
//                     g->my_count = g->rcvd_count;
//                 }
//                 // log_message("Red N %d", g->my_count);
//                 g->outgoing_message.data[8] = g->my_count;
//             }

//         // [CASE]: not stopped, and we've heard from 6 robots on the circle (or 3 this time) (OTTE)
//         if (g->my_stop_status == 0 && (g->my_count >= 10 || g->rcvd_count >= 10)) {
//             /**
//              * @brief not stopped, and we've heard from 6 robots on the circle (or 3 this time) (OTTE)
//              *
//              */

//             // [CASE]: If kilobot in communication range, but far from circle 2
//             if ((g->distance_from_L2_robot <= (DESIRED_DISTANCE + EPSILON)) &&
//                 g->distance_from_L1_robot > (DESIRED_DISTANCE + EPSILON) &&
//                 g->distance > (DESIRED_DISTANCE + EPSILON)) {
//                 /**
//                  * @brief if in appoperiate range of circle 2 and also greater than 30 from all
//                  *        other types of robots in circles, then stop
//                  *
//                  */

//                 // [UPDATE]: update stop status flag
//                 g->my_stop_status = 1;

//                 set_color(LED_GREEN);  // [INDICATION]: In communication range from L2
//                 move(STOP, 1000);  // [MOTOR ACTION]: Turn left
//             }

//             // [CASE]: Kilobot moving away (OTTE)
//             if (g->distance_from_L2_robot < (DESIRED_DISTANCE) &&
//                 g->distance > (DESIRED_DISTANCE)) {
//                 /**
//                  * @brief change this for "perfect-ness of the circle". Larger the value,
//                  *        better the circle but trades off with time
//                  *
//                  */

//                 // [CASE]: going away
//                 if (g->distance_from_L2_robot > g->previous_dist_from_L2) {
//                     // [UPDATE]: update the previous distance of planet from L2
//                     g->previous_dist_from_L2 = g->distance_from_L2_robot;

//                     set_color(LED_YELLOW);  // [INDICATION]: Kilobot moving out of range
//                     move(FORWARD, 1000);  // [MOTOR ACTION]: Move straight
//                     // g->my_stop_status=1;  // REMOVE LATER
//                 }

//                 // [CASE]: turn left, then move straight. Not sure what this is about (OTTE)
//                 if (g->distance_from_L2_robot < g->previous_dist_from_L2) {
//                     // [UPDATE]: update stop status flag and previous distance
//                     g->my_stop_status = 1;
//                     g->previous_dist_from_L2 = g->distance_from_L2_robot;

//                     set_color(LED_YELLOW);  // [INDICATION]: Kilobot moving out of range
//                     move(LEFT, 1500);  // [MOTOR ACTION]: Turn Left
//                     move(FORWARD, 500);  // [MOTOR ACTION]: Move straight
//                 }
//             }
//         }
//         // OTTE ---------- END different cases given a message from a robot on circle 1
//     }



//------------------------------------------------------------------------------------------------------------
    // [CASE]: when kilobot has not received message for a long period
    if ((kilo_ticks - g->timer > 32 * 10 || kilo_ticks == 32 * 15) && g->my_stop_status == 0) {
        set_color(LED_OFF);  // [INDICATION]: kilobot not in communication range
        move(FORWARD, 500);  // [MOTOR ACTION]: Move straight
        // g->my_stop_status=0;

        // [CASE]: when kilobot has not received message for a very long period
        if (kilo_ticks > g->last_changed + 32 * 30) {
            // [UPDATE]: kiloticks
            g->last_changed = kilo_ticks;

            if (kilo_uid < TOTAL_KILOBOTS / 2) {
                move(FORWARD, 3400);  // [MOTOR ACTION]: left kilobot goes straight
            } else if (kilo_uid >= TOTAL_KILOBOTS / 2) {
                move(STOP, 3400);  // [MOTOR ACTION]: right kilobot stops
            }
        }
    }
}

/**
 * @brief receiver callback function.
 * 
 * @param m  // message 
 * @param d  // distance 
 */
void message_rx(message_t *m, distance_measurement_t *d) {
    // [CASE]: getting message from star robot
    if (m->data[0] == 0) {
        g->timer = kilo_ticks;
        g->new_message = 1;
        g->distance = estimate_distance(d);

        // [CASE]: star robot saying that it has reached desired number of neighbors
        if (m->data[1] >= ROBOTS_IN_FIRST_CIRCLE) {
            // [UPDATE]: ring 1 has been formed
            g->ring_status_from_star_robot = 1;
        } else if (m->data[1] < ROBOTS_IN_FIRST_CIRCLE) {
            // [UPDATE]: ring 1 is not formed yet
            g->ring_status_from_star_robot = 0;
        }
    }

    // [CASE]: getting message from planet robot
    // if (m->data[0] == 1) {
    //     // [CASE]: getting message from planet robot
    //     g->new_message_from_L1_robot = 1;

    //     // [CASE]: saying that we got a message from a L1 robot
    //     if (m->data[1] == 1) {
    //         g->distance_from_L1_robot = estimate_distance(d);
    //         g->message_from_stopped_L1_robot = 1;
    //         g->timer = kilo_ticks;
    //     }

    //     // [CASE]: getting message from L2 robot (alrready stopped)
    //     if (m->data[1] == 2) {
    //         // g->timer=kilo_ticks;  // REMOVE LATER
    //         g->heard_from_L2 = kilo_ticks;
    //         g->new_message_from_L2_robot = 1;
    //         g->distance_from_L2_robot = estimate_distance(d);

    //         //************
    //         // g->time_array[m->data[2]]=kilo_ticks;
    //         SetBit(g->my_array, m->data[2]);
    //         // setting m->data[1]th bit equal 1 , m->data[1] is the kilouid of the robot we recieve message from

    //         // printf("bit : %d", SetBit(g->my_array, m->data[2]));
    //         g->rcvd_array[g->count] = ((uint32_t)m->data[4] << 24) +
    //                                   ((uint32_t)m->data[5] << 16) +
    //                                   ((uint32_t)m->data[6] << 8) +
    //                                   ((uint32_t)m->data[7]);
    //         g->rcvd_count = m->data[8];
    //         //*************
    //     }

        // // [CASE]: a planet sent us message that the ring has formed
        // if (m->data[3] == 1) {
        //     g->ring_status_from_planet_robot = 1;
        // }
    // }
}

/**
 * @brief transmitter callback function.
 * 
 * @return message_t* 
 */
message_t *message_tx() {
    // // message is transmitted roughly twice per sec
    // memcpy(&g->outgoing_message.data[4], &g->i1, sizeof(uint8_t));
    // memcpy(&g->outgoing_message.data[5], &g->i2, sizeof(uint8_t));
    // memcpy(&g->outgoing_message.data[6], &g->i3, sizeof(uint8_t));
    // memcpy(&g->outgoing_message.data[7], &g->i4, sizeof(uint8_t));

    printf("Transmitting Kilobot: %u\n", g->outgoing_message.data[2]);  // REMOVE LATER
    printf("Transmitting Flag: %u\n", g->outgoing_message.data[1]);  // REMOVE LATER

    // perform CRC on the message, as the values are updated
    g->outgoing_message.crc = message_crc(&g->outgoing_message);

    // message is transmitted roughly twice per sec
    return &(g->outgoing_message);
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

  // Register user defined global structure.
  g = g_safe;

  // Start kilobot.
  kilo_start(setup, loop);

  // free user defined globals
  free(g_safe);

  return 0;
}
