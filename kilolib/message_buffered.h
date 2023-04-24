/**
 * @file message_buffered.h
 * @author Team Harvard
 *
 * @brief Provides functions for buffered message transmission and reception.
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __MESSAGE_BUFFERED_H__
#define __MESSAGE_BUFFERED_H__

#include "kilolib.h"
#include "ringbuffer.h"

#ifndef RXBUFFER_SIZE
#define RXBUFFER_SIZE 16
#endif
#ifndef TXBUFFER_SIZE
#define TXBUFFER_SIZE 4
#endif

/**
 * @brief Structure for storing received messages and distance measurements.
 * 
 */
typedef struct {
    message_t msg;  //  Received message
    distance_measurement_t dist;  //  Distance measurement.
} received_message_t;

RB_create(rxbuffer, received_message_t, RXBUFFER_SIZE);  //  Ring buffer for received messages and distance measurements
RB_create(txbuffer, message_t, TXBUFFER_SIZE);  //  Ring buffer for transmitted messages

/**
 * @brief Returns the current size of the receive buffer.
 * 
 * @return uint8_t (The size of the receive buffer.)
 */
uint8_t rxbuffer_size() {
    return RB_size(rxbuffer);
}

/**
 * @brief Adds a received message and distance measurement to the receive buffer
 * 
 * @param msg (Pointer to the received message)
 * @param dist (Pointer to the distance measurement)
 */
void rxbuffer_push(message_t *msg, distance_measurement_t *dist) {
    received_message_t *rmsg = &RB_back(rxbuffer);
    rmsg->msg = *msg;
    rmsg->dist = *dist;
    RB_pushback(rxbuffer);
}

/**
 * @brief Returns the next received message and distance measurement without removing them from the buffer
 * 
 * @param dist (Pointer to the distance measurement)
 * @return message_t* (Pointer to the next received message, or NULL if the buffer is empty)
 */
message_t *rxbuffer_peek(distance_measurement_t *dist) {
    if (RB_empty(rxbuffer))
        return '\0';
    else {
        received_message_t *rmsg = &RB_front(rxbuffer);
        *dist = rmsg->dist;
        return &rmsg->msg;
    }
}

/**
 * @brief Removes the oldest received message and distance measurement from the receive buffer
 * 
 */
void rxbuffer_pop() {
    if (!RB_empty(rxbuffer)) {
        RB_popfront(rxbuffer);
    }
}

/**
 * @brief Returns the current size of the transmit buffer
 * 
 * @return uint8_t (The size of the transmit buffer)
 */
uint8_t txbuffer_size() {
    return RB_size(txbuffer);
}

/**
 * @brief Adds a message to the transmit buffer.
 * 
 * @param msg (Pointer to the message to transmit)
 */
void txbuffer_push(message_t *msg) {
    RB_back(txbuffer) = *msg;
    RB_pushback(txbuffer);
}

/**
 * @brief Returns the next message to transmit without removing it from the buffer
 * 
 * @return message_t* (Pointer to the next message to transmit, or NULL if the buffer is empty)
 */
message_t *txbuffer_peek() {
    if (RB_empty(txbuffer))
        return '\0';
    else
        return &RB_front(txbuffer);
}

/**
 * @brief Removes the oldest transmitted message from the transmit buffer after a successful transmission
 * 
 */
void txbuffer_pop() {
    if (!RB_empty(txbuffer))
        RB_popfront(txbuffer);
}

/**
 * @brief Initializes the message handling functions for buffered transmission and reception
 * 
 */
inline void kilo_message_buffered() {
    RB_init(rxbuffer);
    RB_init(txbuffer);
    kilo_message_rx = rxbuffer_push;
    kilo_message_tx = txbuffer_peek;
    kilo_message_tx_success = txbuffer_pop;
}

#endif//__MESSAGE_BUFFERED_H__
