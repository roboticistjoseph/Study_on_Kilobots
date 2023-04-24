/**
 * @file bootldr.h
 * @author Team Harvard
 *
 * @brief Header file to structure the boot message
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/** @internal */

/**
 * @brief Structure to represent a boot message
 * 
 */
typedef struct {
    uint8_t page_address;  // The page address of the boot message
    uint8_t page_offset;  // The page offset of the boot message
    uint16_t word1;  // The first word of the boot message
    uint16_t word2;  // The second word of the boot message
    uint16_t word3;  // The third word of the boot message
    uint8_t unused;
} bootmsg_t;
