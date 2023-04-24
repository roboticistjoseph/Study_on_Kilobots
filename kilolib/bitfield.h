/**
 * @file bitfield.h
 * @author Team Harvard
 *
 * @brief Header file defining the functionality of Bit Field
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __BITFIELD_H__
#define __BITFIELD_H__

/**
 * @brief Macro to create a bit field with the specified name and size.
 * 
 * @param NAME The name of the bit field.
 * @param SIZE The total number of bits in the bit field.
 */
#define BF_create(NAME, SIZE)\
    /* Define an array of unsigned chars to hold the bit field data. */ \
    unsigned char NAME##_data[(SIZE>>3)] = {0};\
    /* Define a constant to hold the size of the array in bytes (2^3 = 8 bits). */ \
    static const unsigned int NAME##_size = (SIZE>>3)

/**
 * @brief Initializes a bit field by setting all of its bits to 0.
 *
 * @param NAME The name of the bit field to initialize.
 */
#define BF_init(NAME) {\
    int i = 0;\
    for (i = 0; i < NAME##_size; i++) NAME##_data[i] = 0;\
}

/**
 * @brief Set the value of a bit in a bit field.
 *
 * This macro sets the value of a single bit in a bit field. The bit field is
 * identified by its NAME, which is the name of the bit field array. The index of
 * the bit to set within the bit field is specified by IDX. The macro uses
 * bitwise operations to set the bit at the specified index to 1.
 *
 * @param NAME The name of the bit field to set.
 * @param IDX The index of the bit to set within the bit field.
 *
 * @note This macro assumes that the bit field array has already been defined
 * using the BF_create macro.
 */
#define BF_set(NAME, IDX) NAME##_data[IDX >> 3] |= (1 << (IDX&7))

/**
 * @brief Clears the bit at the specified index in the bit field.
 *
 * This macro clears the bit at the specified index in the bit field
 * by performing a 'bitwise AND' operation between the byte that contains
 * the bit and the bitwise complement of a bit mask that has a 1 at the
 * bit position within the byte.
 *
 * @param NAME The name of the bit field.
 * @param IDX The index of the bit to clear.
 */
#define BF_clr(NAME, IDX) NAME##_data[IDX >> 3] &= ~(1 << (IDX&7))

/**
 * @brief Returns the value of a bit with the given index in a bit field.
 *
 * The `BF_get` macro extracts the value of the `IDX`th bit from the bit field `NAME`.
 * The value of the `IDX`th bit is determined by finding the byte that contains that bit,
 * and then masking and shifting the bits of that byte to extract the value of the bit.
 *
 * @param NAME The name of the bit field.
 * @param IDX The index of the bit to get.
 *
 * @return The value of the `IDX`th bit in the bit field `NAME`.
 */
#define BF_get(NAME, IDX) (NAME##_data[IDX >> 3] & (1 << (IDX&7)))

/**
 * @brief Returns the capacity (in bytes) of the bit field with the specified name.
 *
 * This macro returns the capacity (in bytes) of the bit field with the specified name.
 * The name of the bit field should be passed to the macro as a parameter, without the "_data" suffix.
 * 
 * @param NAME The name of the bit field (without the "_data" suffix).
 *
 * @return The capacity (in bytes) of the bit field.
 */
#define BF_capacity(NAME) NAME##_size

/**
 * @brief Clears all bits in the given bit field.
 *
 * This macro clears all bits in the given bit field by setting all bytes in the bit field array to 0.
 *
 * @param NAME The name of the bit field.
 */
#define BF_clear(NAME) {\
    int i = 0;\
    for (i = 0; i < NAME##_size; i++) NAME##_data[i] = 0;\
}

#endif//__BITFIELD_H__
