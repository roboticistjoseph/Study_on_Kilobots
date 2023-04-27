/**
 * @file chris_con_circ.c
 * @author Chris
 *
 * @brief 2nd version of the algorithm to form concentric circles using singles. (Planet)
 * @version 0.1
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

// preprocessor directives
#define SetBit(A, k) (A[(k / 32)] |= (1 << (k % 32)))    // sets the kth bit in array A
#define ClearBit(A, k) (A[(k / 32)] &= ~(1 << (k % 32)))
#define TestBit(A, k) (A[(k / 32)] & (1u << (k % 32)))    // checks if we have 1 at kth position
#define NUM_R 20

// motion
#define STOP 0
#define FORWARD 1
#define LEFT 2
#define RIGHT 3

// LED colors
#define LED_OFF          RGB(0, 0, 0)
#define LED_WHITE        RGB(1, 1, 1)

#define LED_BLUE         RGB(0, 0, 2)
#define LED_CYAN_BLUE    RGB(0, 1, 2)
#define LED_CYAN         RGB(0, 2, 2)
#define LED_GREEN_CYAN   RGB(0, 2, 1)
#define LED_GREEN        RGB(0, 2, 0)
#define LED_YELLOW_GREEN RGB(1, 2, 0)
#define LED_YELLOW       RGB(2, 2, 0)
#define LED_ORANGE       RGB(2, 1, 0)
#define LED_RED          RGB(2, 0, 0)
#define LED_MAGENTA_RED  RGB(2, 0, 1)
#define LED_MAGENTA      RGB(2, 0, 2)
#define LED_PURPLE       RGB(1, 0, 2)
#define MAX_NEIGHBORS 200

struct GLOBALS {
    // global variables
    // NOTE: the use of a GLOBALS struct will also work on a normal kilobot,
    //       but is not required on a normal kilobot.
    //       It is, however, required by the simulator.

    int current_motion;
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
    int layer_2_robot_done;
    int new_message_from_L2_robot;
    int distance_from_L2_robot;

    //**************************************
    int my_array[4];    // holds bits for 96 robots 3*(4*8bits)  int is 4bytes
    int rcvd_array[4];  // holds bits for 96 robots 3*(4*8bits)  int is 4bytes
    int time_array[NUM_R];
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

// put your setup code here, will be run once at the beginning
// this is a good place, for example, to define the intial values
// of global variables
void setup() {
    //**********************************************************
    for (int n = 0; n < 3; n++) {
        g->my_array[n] = 0;
        g->rcvd_array[n] = 0;
    }

    for (int r = 0; r <= NUM_R - 1; r++) {
        g->time_array[r] = 0;
    }

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
    g->layer_2_robot_done = 0;
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

    // (note that because we are never changing the data in the message in this example
    // then we only need to do this once, otherwise we'd need to do it every time we
    // wanted to send a new message)

    // The type is always NORMAL.
    g->outgoing_message.type = NORMAL;

    // indicate we are a planet
    g->outgoing_message.data[0] = 1;

    // saying we stopped
    g->outgoing_message.data[1] = 0;  // my ring number

    // ilo_uid
    g->outgoing_message.data[2] = kilo_uid;

    // ring status to tell other robot planets
    g->outgoing_message.data[3] = 0;

    // It's important that the CRC is computed after the data has been set;
    // otherwise it would be wrong.
    g->outgoing_message.crc = message_crc(&g->outgoing_message);
}

int count() {
    int counter = 0;
    for (int p = 0; p < 96; p++) {
        if (TestBit(g->my_array, p)) {
            counter++;
        }
    }
    return counter;
}
// int write(int q)

// {
//     int distance, error, time, read_distance, read_error, read_time, num;
//     num = 190;
//     distance = q;
//     error = 92;
//     time = 99;

//     FILE *fptr;
//     fptr = (fopen("C:\\Users\\anshu\\Documents\\UMD\\Thesis\\fatte\\robot_data.txt", "a"));

//     FILE *read_fptr;
//     read_fptr = (fopen("C:\\Users\\anshu\\Documents\\UMD\\Thesis\\fatte\\robot_data.txt", "r"));

//     for (int i = 0; i < num; ++i)
//     {
//         fprintf(fptr, "%d,%d,%d\n", q, error, time); // writing data to file
//     }

//     //    char ch1 = getc(read_fptr);
//     //    while(ch1!= EOF)
//     //    {

//     //       if (ch1 == '\n')
//     //       {
//     //          num_lines++; // counting total number of lines
//     //       }
//     //       ch1 = getc(read_fptr);
//     //    }

//     //    for (int d;d<num_lines;d++)
//     //    {
//     //       fgets(current_line, 10, read_fptr);
//     //    }
//     //    printf("Num lines %d",num_lines);

//     fclose(fptr);
//     return 0;
// }

void loop() {
    // printf("array: %d", (char)(g->outgoing_message.data[2]));

    // printf("bit : %d", (char)SetBit(g->my_array, g->outgoing_message.data[2]));
    g->loop_count++;
    if (g->loop_count == 0) {
        delay(9000);
    }

    // set_color(RGB(0, 0, 0));
    g->outgoing_message.data[1] = 0;
    // g->outgoing_message.data[4]=0;
    g->my_stop_status = 0;
    // log_message("Ring status from star %d",g->ring_status_from_star_robot);

    if (g->new_message == 1) {
        g->new_message = 0;
        set_color(RGB(0, 0, 0));

        // OTTE -------- START different distance based cases for first circle

        // g->distance==25||g->distance==26||g->distance==27||g->distance==28||
        if (g->distance == 25 || g->distance == 26 || g->distance == 27 ||
            g->distance == 28 || g->distance == 29 || g->distance == 30 || g->distance == 31) {
            // stopping criteria
            // OTTE: stop if within correct or slighly near-center part of goal region

            set_motors(0, 0);
            delay(1000);
            set_color(RGB(0, 0, 1));

            g->my_ring_number = 1;
            g->outgoing_message.data[1] = 1;  // Im ring 1
            g->my_stop_status = 1;
            // g->messgae.data[8]=1;  // I'm ring 1
        }

        if (g->distance < 25 && g->ring_status_from_star_robot == 0 &&
            g->ring_status_from_planet_robot == 0) {  // && g->distance>=20
            // OTTE: too close to center, so turn around


            // g->outgoing_message.data[1]=0;
            // g->message.data[3]=0;
            set_color(RGB(0, 1, 1));
            set_motors(kilo_straight_left, 0);
            delay(500);
            set_motors(0, 0);
            delay(500);
            set_motors(kilo_straight_left, kilo_straight_right);
            delay(650);
            g->my_stop_status = 0;
        }

        if (g->distance == 32 || g->distance == 33 || g->distance == 34 || g->distance == 35) {
            // OTTE: almost there so move straight

            // g->outgoing_message.data[1]=0;
            set_motors(kilo_straight_left, kilo_straight_right);
            delay(150);
            set_motors(0, 0);
            delay(100);
            // g->my_stop_status=0;
        }
        // OTTE -------- END different distance based cases for first circle

        // OTTE: check for ring status (for first circle from the seed robot
        if (g->ring_status_from_star_robot == 1) {\
            // if the ring is formed then I will tell other planets about it

            if (g->distance < 25) {
                set_color(RGB(1, 1, 1));
                set_motors(0, 0);
                delay(1000);
            }
            g->outgoing_message.data[3] = 1;  // ring status to tell other robot planets
        } else {
            g->outgoing_message.data[3] = 0;
        }
    }

    if (g->new_message_from_L1_robot == 1) {
        // OTTE: recieved message from robot on first circle


        // OTTE ---------- START different cases for circle 1 and 2 formation given a message from a robot on circle 1

        g->new_message_from_L1_robot = 0;

        if (g->message_from_stopped_L1_robot == 1 && g->my_stop_status == 0 &&
            g->ring_status_from_planet_robot == 0 && g->distance >= 31) {  // steering for formation of L1
            // OTTE: circle 1 not yet formed, so move forward then turn a bit (not sure why)

            g->message_from_stopped_L1_robot = 0;
            set_color(RGB(0, 1, 0));
            set_motors(kilo_straight_left, kilo_straight_right);
            delay(700);
            set_motors(0, 0);
            delay(500);
            set_motors(kilo_straight_left, 0);
            delay(300);
            set_motors(0, 0);
            delay(300);
        }

        if (g->message_from_stopped_L1_robot == 1 && g->my_stop_status == 0 &&
            g->ring_status_from_planet_robot == 1 && g->distance > 30) {  // ring 2 formation
            // OTTE: circle 1 is formed

            g->message_from_stopped_L1_robot = 0;

            if (g->distance_from_L1_robot >= 28) {
                // OTTE: too far (>28 away from circle 1) but we did hear from a circle 1 robot, so just sleep?

                // log_message("Distance is %d:",(int) g->distance_from_L1_robot);

                g->my_stop_status = 1;
                set_color(RGB(1, 0, 0));
                set_motors(0, 0);
                delay(10000);
                g->my_ring_number = 2;
                g->outgoing_message.data[1] = g->my_ring_number;  // indicating we the L2 robot
            }

            if (g->distance_from_L1_robot < 28) {
                // change this for "perfect-ness of the circle".
                // Larger the value, better the circle but trades off with time

                // OTTE: close (< 28 away from circle 1) but we did hear from a circle 1 robot

                if (g->distance_from_L1_robot > g->previous_dist_from_planet_robot) {  // going away
                    // OTTE: don't want to screw things up, so just sleep?

                    g->previous_dist_from_planet_robot = g->distance_from_L1_robot;
                    set_color(RGB(1, 0, 1));
                    set_motors(kilo_straight_left, kilo_straight_right);
                    delay(1000);
                    set_motors(0, 0);
                    // g->my_stop_status=1; //try removing this
                }

                if (g->distance_from_L1_robot < g->previous_dist_from_planet_robot) {
                    // Otte: turn left, then move straight. Not sure what this is about

                    g->my_stop_status = 1;
                    set_color(RGB(0, 1, 0));
                    set_motors(kilo_straight_left, 0);
                    delay(1500);
                    set_motors(0, 0);
                    delay(300);
                    set_motors(kilo_straight_left, kilo_straight_right);
                    delay(500);
                    g->previous_dist_from_planet_robot = g->distance_from_L1_robot;
                }
            }
        }

        // OTTE ---------- END different cases for circle 1 and 2 formation given a message from a robot on circle 1
    }

    if (g->new_message_from_L2_robot == 1) {
        // OTTE: got message from robot on circle 2


        g->new_message_from_L2_robot = 0;


        // OTTE ---------- START different cases given a message from a robot on circle 1

        if (g->my_stop_status == 1 && g->ring_status_from_planet_robot == 1 && g->distance > 32) {
            // OTTE already stopped, and the circle 1 is already formed, and we're greater than 32 from,
            // presumably, the robot on circle 2


            // log_message("COUNTING ");
            for (g->y = 0; g->y < 96; g->y++) {  // OR-ING
                g->my_array[g->y / 32] = g->my_array[g->y / 32] | g->rcvd_array[g->y / 32];
            }

            g->i1 = (g->mask1&g->my_array[0])>>24;
            g->i2 = (g->mask2&g->my_array[0])>>16;
            g->i3 = (g->mask3&g->my_array[0])>>8;
            g->i4 = (g->mask4&g->my_array[0]);

            g->count++;
            if (g->count == 4) {
            g->count = 0;
            }


            if (g->rcvd_count > g->my_count) {
                g->my_count = g->rcvd_count;
            }
            // log_message("Red N %d", g->my_count);
            g->outgoing_message.data[8] = g->my_count;
        }

        if (g->my_stop_status == 0 && (g->my_count >= 3 || g->rcvd_count >= 6)) {
            // OTTE: not stopped, and we've heard from 6 robots on the circle (or 3 this time)



            // log_message("FTS");
            if ((g->distance_from_L2_robot == 25 || g->distance_from_L2_robot == 26 ||
                g->distance_from_L2_robot == 27 || g->distance_from_L2_robot == 28 ||
                g->distance_from_L2_robot == 29 || g->distance_from_L2_robot == 30 ||
                g->distance_from_L2_robot == 31 || g->distance_from_L2_robot == 32) &&
                g->distance_from_L1_robot > 30 && g->distance > 30) {
               // Otte: if in appoperiate range of circle 2 and also greater than 30 from all
               // other types of robots in circles, then stop


                set_color(RGB(1, 1, 0));
                set_motors(0, 0);
               // log_message("Brrrrrr");
                g->my_stop_status = 1;
                delay(10000);
            }

            if (g->distance_from_L2_robot < 28 && g->distance > 30) {
                // change this for "perfect-ness of the circle".
                // Larger the value, better the circle but trades off with time

               // Otte: if too close in circle 2 and also greater than 30 from all other types of robots in circles

                if (g->distance_from_L2_robot > g->previous_dist_from_L2) {  // going away
                    // OTTE: keep moving away

                    g->previous_dist_from_L2 = g->distance_from_L2_robot;
                    set_color(RGB(1, 0, 1));
                    set_motors(kilo_straight_left, kilo_straight_right);
                    delay(1000);
                    set_motors(0, 0);
                    // g->my_stop_status=1; //try removing this
                }

                if (g->distance_from_L2_robot < g->previous_dist_from_L2) {
                    // Otte: turn left, then move straight. Not sure what this is about


                    g->my_stop_status = 1;
                    set_color(RGB(0, 1, 0));
                    set_motors(kilo_straight_left, 0);
                    delay(1500);
                    set_motors(0, 0);
                    delay(300);
                    set_motors(kilo_straight_left, kilo_straight_right);
                    delay(500);
                    g->previous_dist_from_L2 = g->distance_from_L2_robot;
                }
            }
        }
        // OTTE ---------- END different cases given a message from a robot on circle 1
    }

    if ((kilo_ticks - g->timer > 32 * 10 || kilo_ticks == 32 * 15) &&
        g->my_stop_status == 0) {  // if the robot hasnt recieved message in  while
        // OTTE: if not stopped and havn't recieved a message in a while then move forward a bit

        set_color(RGB(0, 0, 0));
        // g->steady_counter=0;
        // g->message.data[3]=0;
        set_motors(kilo_straight_left, kilo_straight_right);
        delay(500);
        set_motors(0, 0);
        delay(500);
        // g->my_stop_status=0;

        if (kilo_ticks > g->last_changed + 32 * 30) {
            // OTTE: if not stopped and havn't recieved a message in a very long while then stop for a while


            set_color(RGB(1, 1, 1));
            g->last_changed = kilo_ticks;
            set_motors(kilo_straight_left, 0);
            delay(3400);
            set_motors(0, 0);
            delay(300);
        }
    }
}

// recieve message callback function
void message_rx(message_t *m, distance_measurement_t *d) {
    if (m->data[0] == 0) {  // getting message from star robot
        g->timer = kilo_ticks;
        g->new_message = 1;
        g->distance = estimate_distance(d);

        if (m->data[1] >= 12) {
            // star robot saying that it has reached desired number of neighbors
            g->ring_status_from_star_robot = 1;  // ring has been formed
        } else if (m->data[1] < 12) {
            g->ring_status_from_star_robot = 0;
        }
    }

    if (m->data[0] == 1) {  // getting message from planet robot
        g->new_message_from_L1_robot = 1;

        if (m->data[1] == 1) {  // saying that we got a message from a L1 robot
            g->distance_from_L1_robot = estimate_distance(d);
            g->message_from_stopped_L1_robot = 1;
            g->timer = kilo_ticks;
        }

        if (m->data[1] == 2) {  // getting message from L2 robot (alrready stopped)
            // g->timer=kilo_ticks;
            g->heard_from_L2 = kilo_ticks;
            g->new_message_from_L2_robot = 1;
            g->distance_from_L2_robot = estimate_distance(d);

            //************
            // g->time_array[m->data[2]]=kilo_ticks;
            SetBit(g->my_array, m->data[2]);
            // setting m->data[1]th bit equal 1 , m->data[1] is the kilouid of the robot we recieve message from

            // printf("bit : %d", SetBit(g->my_array, m->data[2]));
            g->rcvd_array[g->count] = ((uint32_t)m->data[4] << 24) +
                                        ((uint32_t)m->data[5] << 16) +
                                        ((uint32_t)m->data[6] << 8) +
                                        ((uint32_t)m->data[7]);
            g->rcvd_count = m->data[8];
            //*************
        }

        if (m->data[3] == 1) {  // a planet sent us message that the ring has formed
            g->ring_status_from_planet_robot = 1;
        }
    }
}

// send message function
message_t *message_tx() {
    // log_message_from_sim("|| P  data[0]: %u", g->outgoing_message.data[0]);
    memcpy(&g->outgoing_message.data[4], &g->i1, sizeof(uint8_t));
    memcpy(&g->outgoing_message.data[5], &g->i2, sizeof(uint8_t));
    memcpy(&g->outgoing_message.data[6], &g->i3, sizeof(uint8_t));
    memcpy(&g->outgoing_message.data[7], &g->i4, sizeof(uint8_t));

    return &(g->outgoing_message);
}

// sucessfull send message callback
void message_tx_success() {
    // do nothing
}

int main() {
    struct GLOBALS* g_safe = (struct GLOBALS *)malloc(sizeof(struct GLOBALS));

#ifdef USING_SIMULATION
    // register the global variables (only necessary on simulation)
    // g is a pointer
    // so the address of g is a pointer to a pointer,the address of it is getting casted to a pointer to a void pointers

    kilo_globals = (void **)&g;  // NOLINT
#endif

    kilo_init();
    debug_init();
    // Register the message_rx callback function.
    kilo_message_rx = message_rx;

    // Register the message_tx callback function.
    kilo_message_tx = message_tx;

    // Register the message_tx_success callback function.
    kilo_message_tx_success = message_tx_success;

    g = g_safe;

#ifdef USING_SIMULATION
    kilo_start(setup, loop, message_rx, message_tx, message_tx_success);
#else
    kilo_start(setup, loop);
    // free user defined globals
    free(g_safe);
#endif

    return 0;
}
