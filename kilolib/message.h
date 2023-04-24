/**
 * @file message.h
 * @author Team Harvard
 *
 * @brief This file contains the declaration of the message_type_t enum and its associated message types.
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <stdint.h>

/**
 * @brief The different types of messages that can be sent.
 * 
 */
typedef enum {
    NORMAL = 0,     //  A normal message
    GPS,            //  A message containing GPS data
    BOOT = 0x80,    //  A message indicating the device is booting up
    BOOTPGM_PAGE,   //  A message containing the current boot program page
    BOOTPGM_SIZE,   //  A message containing the size of the boot program
    RESET,          //  A message indicating that the device has been reset
    SLEEP,          //  A message indicating that the device is going to sleep
    WAKEUP,         //  A message indicating that the device is waking up
    CHARGE,         //  A message indicating that the device is charging
    VOLTAGE,        //  A message containing the current battery voltage
    RUN,            //  A message indicating that the device is running
    READUID,        //  A message to read the unique device ID
    CALIB,          //  A message to calibrate the device
} message_type_t;


/**
 * @brief Message structure.
 *
 * A message structure is 12 bytes in length and is composed of three
 * parts: the payload (9 bytes), the message type (1 byte), and a CRC (2
 * bytes).
 *
 * @note When preparing a message for transmission, at a minimum you
 * must specify the type (use a value between 0 and 127 for user
 * messages) and the CRC (use the message_crc() function for this
 * purpose).
 *
 * @see message_crc, kilo_message_rx, kilo_message_tx,
 * kilo_message_tx_success
 */
typedef struct {
    uint8_t data[9];  //  message payload.
    uint8_t type;     //  message type.
    uint16_t crc;     //  message crc.
} message_t;

#endif//__MESSAGES_H__
