/**
 * @file grid_formation.c
 * @author Joseph Katakam (jkatak73@terpmail.umd.edu)
 *
 * @brief Code to make kilobots form a Grid shape formation of 2 x 3.
 * @version 0.1
 * @date 2023-03-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// include kilolib API
#include "../../../kilolib/kilolib.h"

// Define MOTION
#define FORWARD 0
#define LEFT 1
#define RIGHT 2
#define STOP 3

//---- PARAMETERS FOR SHAPE FORMATION/ COMMUNICATION -------
#define DESIRED_DISTANCE 65
#define EPSILON_MARGIN 5
#define MOTOR_ON_DURATION 500
#define NUMBER_COMMUNICATION 3

// // Constants for orbit control.
// #define TOO_CLOSE_DISTANCE 40
// #define DESIRED_DISTANCE 60

//---- DEFINE STATE -------
#define NEIGHBOURS_IN_RANGE 1
#define COMPARE_DESIRED_DISTANCE 2
#define ORBIT_AND_UPDATE_INDEX 3
#define FINISH 4
#define INFINITY 5

// Variable Declaration
int state = -1, last_state;  // State
int distance = 10000, last_distance = -1, min_distance;  // Distance Flag
int message_rx_status = 0, message_sent = -1;  // Rx Flag
int distance_holder = -1, last_kilobot_UID = -1;  // value holder
int index = 3, max_index = 2;
int check = 0, x = -1, y = -1;

// kilobot message structure
message_t message;

//---- ARRAYS FOR STORING NEIGHBOURS INFORMATION -----
int reception_id[3] = {0, 0, 0};
float reception_distance[3] = {0, 0, 0};

//---- SHAPE MATRIX ------
int neighbours[8][2] = {{0, 0},
                        {0, 0},
                        {0, 1},
                        {1, 2},
                        {2, 3},
                        {3, 4},
                        {4, 5},
                        {5, 6}};

float distance_multiplier[8][2] = {{0, 0},
                                    {0, 0},
                                    {0, 0},
                                    {1, 1},
                                    {1, 1.4},
                                    {1, 1},
                                    {1, 1.4},
                                    {1, 1}};

/**
 * @brief Function to handle motion.
 * 
 * @param new_motion 
 */
void move(int direction) {
    switch (direction) {
        case FORWARD:
            // STRAIGHT
            spinup_motors();
            set_motors(kilo_staight_left, kilo_straight_right);
            delay(MOTOR_ON_DURATION);
            set_motors(0, 0);
            break;
        case LEFT:
            // LEFT
            spinup_motors();
            set_motors(kilo_staight_left, 0);
            delay(MOTOR_ON_DURATION);
            set_motors(0, 0);
            break;
        case RIGHT:
            // RIGHT
            spinup_motors();
            set_motors(0, kilo_straight_right);
            delay(MOTOR_ON_DURATION);
            set_motors(0, 0);
            break;
        case STOP:
            // STOP
            set_motors(0, 0);
            break;
    }
}

/**
 * @brief transmitter callback function.
 * 
 * @return message_t* 
 */
message_t *message_tx() {
    return &message;
}

/**
 * @brief transmitter success callback function.
 * 
 */
void message_tx_success() {
    message_sent = 1;
    set_color(RGB(1, 0, 1));
    delay(100);
    set_color(RGB(0, 0, 0));
}

/**
 * @brief receiver callback function.
 * 
 * @param msg 
 * @param measured_distance 
 */
void messgae_rx(message_t *msg, distance_measurement_t *measured_distance) {
    // initialize distance to high value if Rx status is 0
    if (message_rx_status == 0) {
        distance = 1000;
    }

    // if status not reached it's limit
    if (message_rx_status != NUMBER_COMMUNICATION) {
        // calculate minimum distance
        distance_holder = estimate_distance(measured_distance);
        if (distance_holder < distance) {
            // update distance
            distance = distance_holder;
        }

        // Store reception ID
        reception_id[message_rx_status] = (*msg).data[0];

        // Maximum Index in Current Communication
        if (reception_id[messgae_rx_status] > max_index) {
            max_index = reception_id[message_rx_status];
        }

        // Store reception distance
        reception_distance[message_rx_status] = distance_holder;

        // store calculated distance
        last_distance = distance;

        if (last_kilobot_UID != (*msg).data[0]) {
            // update latest rx kilobot UID
            last_kilobot_UID = (*msg).data[0];

            // update Rx status
            message_rx_status++;
        }

        // // update Rx status
        // message_rx_status++;
    }
}

void measure_distance() {
    message_rx_status = 0;
}

/**
 * @brief Kilobot Setup
 * 
 */
void setup() {
    // Reset Finite State Machine (FSM)
    state = NEIGHBOURS_IN_RANGE;
    set_color(RGB(0, 0, 1));
}

/**
 * @brief Kilobot Loop
 * 
 */
void loop() {
    // FSM
    switch (state) {
        case NEIGHBOURS_IN_RANGE:
            // check if kilobot is in between 3 neighbours
            if (message_rx_status == NUMBER_COMMUNICATION) {
                // check successful, therefore change state localize
                state = COMPARE_DESIRED_DISTANCE;

                // initiate reception
                message_rx_status = 0;

                break;
            } else if (message_rx_status >= 1) {
                // if kilobot not among neighbours, keep orbiting
                state = ORBIT_AND_UPDATE_INDEX;
            } else {
                move(FORWARD);
            }

            // state = COMPARE_DESIRED_DISTANCE;
            // // initiate reception
            // message_rx_status = 0;
            // break;
        case ORBIT_AND_UPDATE_INDEX:
            // Algorithm for Orbiting Clockwise
            if (last_distance >= DESIRED_DISTANCE) {
                set_color(RGB(1, 0, 0));
                move(RIGHT);
                state = NEIGHBOURS_IN_RANGE;
            } else {
                set_color(RGB(0, 0, 1));
                move(LEFT);
                state = NEIGHBOURS_IN_RANGE;
            }
            // Update index if Required
            if (max_index+1 > index) {
                index = max_index + 1;
            }
            break;
        case COMPARE_DESIRED_DISTANCE:
            // localizing in accordance with shape formation matrix
            for (int i = 0; i < NUMBER_COMMUNICATION; i++) {
                for (int j = i+1; j < NUMBER_COMMUNICATION; j++) {
                    // Check if desired numbers in Range
                    if ( (reception_id[i] == neighbours[index][0]) && (reception_id[j] == neighbours[index][1]) ) {
                        x = i;
                        y = j;
                        check = 1;
                        break;
                    }
                }
                if (check == 1) {
                    break;
                }
            }

            // Check if desired neighbours at desired distance
            if ((check == 1) &&
                (reception_distance[x] > distance_multiplier[index][0] * DESIRED_DISTANCE - EPSILON_MARGIN &&
                    reception_distance[x] < distance_multiplier[index][0] * DESIRED_DISTANCE + EPSILON_MARGIN) &&
                (reception_distance[y] > distance_multiplier[index][1] * DESIRED_DISTANCE - EPSILON_MARGIN &&
                    reception_distance[y] < distance_multiplier[index][1] * DESIRED_DISTANCE + EPSILON_MARGIN) ) {
                        state = FINISH;
                    } else {
                        state = ORBIT_AND_UPDATE_INDEX;
                    }
            break;
        case FINISH:
            set_color(RGB(0, 1, 0));
            // Start Transmission after desired location is acheived
            message.type = NORMAL;
            messgae.data[0] = index;
            message.crc = message_crc(&message);
            kilo_message_tx = message_tx;
            kilo_message_success = message_tx_success;
            // state = INFINITY;
            break;
        // case INFINITY:
        //     break;
        default:
            break;
    }
}

int main() {
    kilo_init();

    // Initialize Reception
    kilo_message_rx = message_rx;

    kilo_start(setup, loop);

    return 0;
}
