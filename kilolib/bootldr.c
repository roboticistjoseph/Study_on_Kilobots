/**
 * @file bootldr.c
 * @author Team Harvard
 *
 * @brief Main bootloader program for Kilobot firmware update.
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/**
 * @brief List of kilolib files in the current directory
 * 
 */
#include "kilolib.h"
#include "bitfield.h"
#include "bootldr.h"
#include "message_send.h"
#include <avr/interrupt.h>  // for cli/sei
#include <avr/io.h>         // for port and register definitions
#include <avr/boot.h>       // to write boot pages
#include <util/delay.h>     // for delay_ms

/**
 * @brief List of global variables
 * 
 */
uint8_t  page_total;            // The total number of pages in the program.
uint8_t  page_count;            // The number of pages currently loaded into flash memory.
uint8_t  page_address;          // The address of the page currently being loaded.
uint16_t page_byte_count;       // The number of bytes loaded into the current page buffer.
uint16_t page_buffer[SPM_PAGESIZE/2+2];  // A buffer used to store pages loaded into memory.
bootmsg_t *bootmsg;             // Pointer to the message received by the bootloader.
BF_create(page_table, 224);     // Create a bit field to track which pages have been loaded.

/**
 * @brief Jump to the beginning of the program.
 */
void goto_program() {
    MCUCR = (1 << IVCE);
    MCUCR = 0;
    asm volatile ("jmp 0x0000");
}

/**
 * @brief Process incoming messages.
 * 
 * This function is called whenever a message is received by the bootloader. If the message
 * is a BOOTPGM_PAGE message, the message data is loaded into the page buffer. If the buffer
 * is full, the page is written to flash memory. If the message is a BOOTPGM_SIZE message,
 * the total number of pages in the program is updated. If the message is a BOOT message,
 * the bootloader jumps to the bootloader section of memory to allow a new firmware to be
 * flashed onto the Kilobot.
 *
 * @param msg A pointer to the message received.
 * @param dist A pointer to the distance measurement associated with the message.
 */
void message_rx(message_t *msg, distance_measurement_t *dist) {
    if (msg->type == BOOTPGM_PAGE) {
        bootmsg = (bootmsg_t*)msg->data;
        if (page_address != bootmsg->page_address) {
            page_address = bootmsg->page_address;
            page_byte_count = 0;
        }
        page_buffer[bootmsg->page_offset] = bootmsg->word1;
        page_buffer[bootmsg->page_offset+1] = bootmsg->word2;
        page_buffer[bootmsg->page_offset+2] = bootmsg->word3;
        page_byte_count += 6;
        if (page_byte_count >= SPM_PAGESIZE && !BF_get(page_table, page_address)) {
            /**
             * Write program page to flash.
             *
             * Taken from http://www.nongnu.org/avr-libc/user-manual/group__avr__boot.html
             */

            eeprom_busy_wait();

            boot_page_erase(page_address*SPM_PAGESIZE);
            boot_spm_busy_wait();

            int i, j;
            for (i = 0, j = 0; i < SPM_PAGESIZE; i += 2, j++)
                boot_page_fill(page_address*SPM_PAGESIZE+i, page_buffer[j]);

            boot_page_write(page_address*SPM_PAGESIZE);
            boot_spm_busy_wait();

            boot_rww_enable();

            set_color(RGB(0, 3, 0));
            BF_set(page_table, page_address);
            page_count++;
            if (page_count == page_total)
                goto_program();
        }
        else
            set_color(RGB(0, 0, 1));
    } else if (msg->type == BOOTPGM_SIZE) {
        page_total = msg->data[0];
        if (page_count == page_total)
            goto_program();
    } else if (msg->type == BOOT) {
            asm volatile ("jmp 0x7000");
    } else {
        if (page_count == 0)
            goto_program();
    }
}

int main() {
    // Disable interrupts
    cli();

    // Move interrupt vectors to bootloader interrupts
    MCUCR = (1 << IVCE);
    MCUCR = (1 << IVSEL);

    // Initialize bitfield and other variables
    BF_init(page_table);
    page_total = 220;
    page_count = 0;
    page_address = 0;
    page_byte_count = 0;

    // Enable interrupts
    sei();

    // Set the message receive callback function
    kilo_message_rx = message_rx;

    // Initialize the kilobot hardware
    kilo_init();

    // Flash blue LED to indicate successful initialization
    while (1) {
        set_color(RGB(0, 0, 3));
        _delay_ms(5);
        set_color(RGB(0, 0, 0));
        _delay_ms(1000);
    }

    // Return success
    return 0;
}
