/**
 * @file message_send.h
 * @author Team Harvard
 *
 * @brief Functions for sending messages.
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __MESSAGE_SEND_H__
#define __MESSAGE_SEND_H__

#include "message.h"

/**
 * @brief A mask to indicate whether the transmit buffer is full.
 * 
 *
 * This variable is used to indicate whether the transmit buffer is full.
 * It is declared as a volatile uint8_t, which means that it may change at any time and should be accessed carefully.
 *
 */
extern volatile uint8_t tx_mask;

/**
 * @brief Sends a message.
 * 
 * This function sends a message using the message_t structure provided as an argument.
 * It returns a uint8_t value indicating whether the send was successful or not.
 *
 * @param message (A pointer to a message_t structure containing the message to send)
 *
 * @return uint8_t (value indicating whether the send was successful (1) or not (0))
 */
uint8_t message_send(const message_t *);

#endif//__MESSAGE_SEND_H__
