/**
 * @file message_crc.c
 * @author Team Harvard
 *
 * @brief Implements functions for working with message_t data type
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "message.h"
#include <util/crc16.h>  // for optimized crc routines

/**
 * @brief Calculates the CRC-16 value for the given message 
 * 
 * @param msg (Pointer to the message_t structure to calculate CRC)
 * @return uint16_t (The CRC-16 value for the message)
 */
uint16_t message_crc(const message_t *msg) {
    uint8_t i;
    const uint8_t *rawmsg = (const uint8_t*)msg;
    uint16_t crc = 0xFFFF;
    for (i = 0; i < sizeof(message_t)-sizeof(msg->crc); i++)
        crc = _crc_ccitt_update(crc, rawmsg[i]);
    return crc;
}
