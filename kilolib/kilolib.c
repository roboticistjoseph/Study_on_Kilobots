/**
 * @file kilolib.c
 * @author Team Harvard
 *
 * @brief Kilolib library main source file
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <avr/io.h>         // io port and addresses
#include <avr/wdt.h>        // watch dog timer
#include <avr/interrupt.h>  // interrupt handling
#include <avr/eeprom.h>     // read eeprom values
#include <avr/sleep.h>      // enter powersaving sleep mode
#include <util/delay.h>     // delay macros
#include <stdlib.h>         // for rand()

#include "kilolib.h"
#include "message_send.h"
#include "macros.h"
#include "ohc.h"

#define EEPROM_OSCCAL         (uint8_t*)0x01
#define EEPROM_TXMASK         (uint8_t*)0x90
#define EEPROM_IRLOW          (uint8_t*)0x20
#define EEPROM_IRHIGH         (uint8_t*)0x50
#define EEPROM_UID            (uint8_t*)0xB0
#define EEPROM_LEFT_ROTATE    (uint8_t*)0x05
#define EEPROM_RIGHT_ROTATE   (uint8_t*)0x09
#define EEPROM_LEFT_STRAIGHT  (uint8_t*)0x0C
#define EEPROM_RIGHT_STRAIGHT (uint8_t*)0x14
#define TX_MASK_MAX   ((1<<0)|(1<<1)|(1<<2)|(1<<6)|(1<<7))
#define TX_MASK_MIN   ((1<<0))

/* Number of clock cycles per bit. */
#define rx_bitcycles 269
/* Number of clock cycles for an entire message. */
#define rx_msgcycles (11*rx_bitcycles)

/**
 * @brief Function pointer type for function that does not return and never exits (no return value and attribute noreturn).
 */
typedef void (*AddressPointer_t)(void) __attribute__ ((noreturn));

/** Dummy functions to avoid null-pointer exceptions when accessing message reception and transmission functions. */
void message_rx_dummy(message_t *m, distance_measurement_t *d) { }
message_t *message_tx_dummy() { return NULL; }
void message_tx_success_dummy() {}

/** Function pointers to message reception and transmission functions. */
message_rx_t kilo_message_rx = message_rx_dummy;
message_tx_t kilo_message_tx = message_tx_dummy;
message_tx_success_t kilo_message_tx_success = message_tx_success_dummy;

message_t rx_msg;                  // message being received
distance_measurement_t rx_dist;    // signal strength of message being received
static uint8_t *rawmsg = (uint8_t*)&rx_msg;
volatile uint8_t rx_busy;          // flag that signals if message is being received
uint8_t rx_leadingbit;             // flag that signals start bit
uint8_t rx_leadingbyte;            // flag that signals start byte
uint8_t rx_byteindex;              // index to the current byte being decoded
uint8_t rx_bytevalue;              // value of the current byte being decoded
volatile uint8_t tx_mask;
volatile uint16_t kilo_tx_period;

#ifndef BOOTLOADER
uint16_t tx_clock;                 // number of timer cycles we have waited
uint16_t tx_increment;             // number of timer cycles until next interrupt
volatile uint32_t kilo_ticks;      // internal clock (updated in tx ISR)
uint16_t kilo_uid;                 // unique identifier (stored in EEPROM)
uint8_t kilo_turn_left;
uint8_t kilo_turn_right;
uint8_t kilo_straight_left;
uint8_t kilo_straight_right;
uint16_t kilo_irhigh[14];
uint16_t kilo_irlow[14];
#endif

/**
 * @brief Enumeration of the possible states of the kilobot.
 * 
 */
static volatile enum {
    SLEEPING,
    IDLE,
    BATTERY,
    SETUP,
    RUNNING,
    CHARGING,
    MOVING
} kilo_state;

/**
 * @brief Initializes kilobot at startup
 *
 * This function initializes the kilobot at startup. It disables interrupts, sets up various 
 * components such as ports, timers, motors, analog-to-digital converter (ADC), auto-comparator (ACOMP), 
 * and read EEPROM values for oscillator calibration, transmission mask, unique ID (UID), and infrared (IR) values. 
 * It also sets up variables related to transmission and reception of IR messages, such as the 
 * transmission mask, the transmission clock, kilobot ticks, and the kilobot's state.
 * It sets the UID by reading the values from EEPROM and merges the two bytes to create a 16-bit UID.
 * Finally, it enables interrupts and exits the function.
 *
 * @return void
 */
void kilo_init() {
    cli();  // disable interrupts

    ports_off();  // turn off all I/O ports
    ports_on();  // turn on all I/O ports

    tx_timer_setup();  // setup transmission timer
    rx_timer_setup();  // setup reception timer

    motors_setup();  // setup motors
    acomp_setup();  // setup auto-comparator (ACOMP)

    adc_setup();  // setup analog-to-digital converter (ADC)
    adc_trigger_high_gain();  // trigger high gain for ADC

    uint8_t osccal = eeprom_read_byte(EEPROM_OSCCAL);  // read oscillator calibration value from EEPROM

    // if oscillator calibration value is not 0xFF, then set the oscillator calibration register to the read value
    if (osccal != 0xFF)
        OSCCAL = osccal;

    rx_busy = 0;  // set reception flag to 0
    rx_leadingbit = 1;  // set reception leading bit to 1
    rx_leadingbyte = 1;  // set reception leading byte to 1
    rx_byteindex = 0;  // set reception byte index to 0
    rx_bytevalue = 0;  // set reception byte value to 0

#ifndef BOOTLOADER
    tx_mask = eeprom_read_byte(EEPROM_TXMASK);  // read transmission mask from EEPROM

    // if transmission mask is outside of maximum value, then set it to the minimum value
    if (tx_mask & ~TX_MASK_MAX)
        tx_mask = TX_MASK_MIN;

    tx_clock = 0;  // set transmission clock to 0
    tx_increment = 255;  // set transmission increment to 255
    kilo_ticks = 0;  // set kilobot ticks to 0
    kilo_state = IDLE;  // set kilobot state to IDLE
    kilo_tx_period = 3906;  // set kilobot transmission period to 3906

    // read UID values from EEPROM and merge the two bytes to create a 16-bit UID
    kilo_uid = eeprom_read_byte(EEPROM_UID) | eeprom_read_byte(EEPROM_UID+1) << 8;

    kilo_turn_left = eeprom_read_byte(EEPROM_LEFT_ROTATE);  // read left rotation value from EEPROM
    kilo_turn_right = eeprom_read_byte(EEPROM_RIGHT_ROTATE);  // read right rotation value from EEPROM
    kilo_straight_left = eeprom_read_byte(EEPROM_LEFT_STRAIGHT);  // read left straight value from EEPROM
    kilo_straight_right = eeprom_read_byte(EEPROM_RIGHT_STRAIGHT);  // read right straight value from EEPROM

    uint8_t i;
    for (i = 0; i < 14; i++) {
        kilo_irlow[i]=(eeprom_read_byte(EEPROM_IRLOW + i*2) <<8) | eeprom_read_byte(EEPROM_IRLOW + i*2+1);
        kilo_irhigh[i]=(eeprom_read_byte(EEPROM_IRHIGH + i*2) <<8) | eeprom_read_byte(EEPROM_IRHIGH + i*2+1);
    }
#endif
    sei();
}

#ifndef BOOTLOADER
/**
 * @brief Initializes the watchdog timer
 *
 * This function is used to ensure that the watchdog timer is inactive after system reset.
 *
 */
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

/**
 * @brief Disables the watchdog timer
 * 
 */
void wdt_init(void) {
    MCUSR = 0;
    wdt_disable();
}

/**
 * @brief Watchdog timer interrupt.
 *
 * This function is used as an interrupt handler for the watchdog timer, and is used to
 * wake up from low power sleep mode.
 * 
 */
ISR(WDT_vect) {
    wdt_disable();
}

/**
 * @brief Enumeration of possible robot motion states.
 *
 * This enumeration lists the possible robot motion states.
 * 
 */
enum {
    MOVE_STOP,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_STRAIGHT
};

/**
 * @brief Previous and current robot motion states.
 *
 * These variables are used to store the previous and current robot motion states.
 * 
 */
static volatile uint8_t prev_motion = MOVE_STOP, cur_motion = MOVE_STOP;

/**
 * @brief The main function for the kilobot, responsible for setting up the kilobot,
 * entering different states (e.g. sleeping, idle, charging, moving), and calling 
 * the user-defined setup and loop functions.
 * 
 * @param setup (A pointer to the user-defined setup function)
 * @param loop (A pointer to the user-defined loop function)
 *
 * @return void
 */
void kilo_start(void (*setup)(void), void (*loop)(void)) {
    int16_t voltage;
    uint8_t has_setup = 0;
    while (1) {
        switch (kilo_state) {
            case SLEEPING:
                cli();
                acomp_off();
                adc_off();
                ports_off();
                wdt_enable(WDTO_8S);
                WDTCSR |= (1 << WDIE);
                set_sleep_mode(SLEEP_MODE_PWR_DOWN);
                cli();
                sleep_enable();
                sei();
                sleep_cpu();
                sleep_disable();
                sei();
                rx_busy = 0;
                ports_on();
                adc_on();
                _delay_us(300);
                acomp_on();

                set_color(RGB(3, 3, 3));
                _delay_ms(10);
                if (rx_busy) {
                    set_color(RGB(3, 0, 0));
                    _delay_ms(100);
                }
                set_color(RGB(0, 0, 0));
                break;
            case IDLE:
                set_color(RGB(0, 3, 0));
                _delay_ms(1);
                set_color(RGB(0, 0, 0));
                _delay_ms(200);
                break;
            case BATTERY:
                voltage = get_voltage();
                if (voltage > 682)
                    set_color(RGB(0, 3, 0));
                else if (voltage > 648)
                    set_color(RGB(0, 0, 3));
                else if (voltage > 614)
                    set_color(RGB(3, 3, 0));
                else
                    set_color(RGB(3, 0, 0));
                break;
            case CHARGING:
                if (is_charging()) {
                    set_color(RGB(1, 0, 0));
                    _delay_ms(1);
                    set_color(RGB(0, 0, 0));
                    _delay_ms(200);
                } else
                    set_color(RGB(0, 0, 0));
                break;
            case SETUP:
                if (!has_setup) {
                    setup();
                    has_setup = 1;
                }
                kilo_state = RUNNING;
            case RUNNING:
                loop();
                break;
            case MOVING:
                if (cur_motion == MOVE_STOP) {
                    set_motors(0, 0);
                    prev_motion = MOVE_STOP;
                } else {
                    if (cur_motion != prev_motion) {
                        prev_motion = cur_motion;
                        if (cur_motion == MOVE_LEFT) {
                            set_motors(0xFF, 0);
                            _delay_ms(15);
                            set_motors(kilo_turn_left, 0);
                        } else if (cur_motion == MOVE_RIGHT) {
                            set_motors(0, 0xFF);
                            _delay_ms(15);
                            set_motors(0, kilo_turn_right);
                        } else {
                            set_motors(0, 0xFF);
                            set_motors(0xFF, 0xFF);
                            _delay_ms(15);
                            set_motors(kilo_straight_left, kilo_straight_right);
                        }
                    }
                }
                break;
        }
    }
}

/**
 * @brief Processes a received message, executing the appropriate action depending on the message type
 * 
 * @return void
 */
static inline void process_message() {
    AddressPointer_t reset = (AddressPointer_t)0x0000, bootload = (AddressPointer_t)0x7000;
    calibmsg_t *calibmsg = (calibmsg_t*)&rx_msg.data;
    if (rx_msg.type < BOOT) {
        kilo_message_rx(&rx_msg, &rx_dist);
        return;
    }
    if (rx_msg.type != READUID && rx_msg.type != RUN && rx_msg.type != CALIB)
        motors_off();
    switch (rx_msg.type) {
        case BOOT:
            tx_timer_off();
            bootload();
            break;
        case RESET:
            reset();
            break;
        case SLEEP:
            kilo_state = SLEEPING;
            break;
        case WAKEUP:
            kilo_state = IDLE;
            break;
        case CHARGE:
            kilo_state = CHARGING;
            break;
        case VOLTAGE:
            kilo_state = BATTERY;
            break;
        case RUN:
            if (kilo_state != SETUP && kilo_state != RUNNING) {
                motors_on();
                kilo_state = SETUP;
            }
            break;
        case CALIB:
            switch (calibmsg->mode) {
                case CALIB_SAVE:
                    if (kilo_state == MOVING) {
                        eeprom_write_byte(EEPROM_UID, kilo_uid&0xFF);
                        eeprom_write_byte(EEPROM_UID+1, (kilo_uid>>8)&0xFF);
                        eeprom_write_byte(EEPROM_LEFT_ROTATE, kilo_turn_left);
                        eeprom_write_byte(EEPROM_RIGHT_ROTATE, kilo_turn_right);
                        eeprom_write_byte(EEPROM_LEFT_STRAIGHT, kilo_straight_left);
                        eeprom_write_byte(EEPROM_RIGHT_STRAIGHT, kilo_straight_right);
                        motors_off();
                        kilo_state = IDLE;
                    }
                    break;
                case CALIB_UID:
                    kilo_uid = calibmsg->uid;
                    cur_motion = MOVE_STOP;
                    break;
                case CALIB_TURN_LEFT:
                    if (cur_motion != MOVE_LEFT || kilo_turn_left != calibmsg->turn_left) {
                        prev_motion = MOVE_STOP;
                        cur_motion = MOVE_LEFT;
                        kilo_turn_left = calibmsg->turn_left;
                    }
                    break;
                case CALIB_TURN_RIGHT:
                    if (cur_motion != MOVE_RIGHT || kilo_turn_right != calibmsg->turn_right) {
                        prev_motion = MOVE_STOP;
                        cur_motion = MOVE_RIGHT;
                        kilo_turn_right = calibmsg->turn_right;
                    }
                    break;
                case CALIB_STRAIGHT:
                    if (cur_motion != MOVE_STRAIGHT || kilo_straight_right != calibmsg->straight_right || kilo_straight_left != calibmsg->straight_left) {
                        prev_motion = MOVE_STOP;
                        cur_motion = MOVE_STRAIGHT;
                        kilo_straight_left = calibmsg->straight_left;
                        kilo_straight_right = calibmsg->straight_right;
                    }
                    break;
            }
            if (calibmsg->mode != CALIB_SAVE && kilo_state != MOVING) {
                motors_on();
                kilo_state = MOVING;
            }
            break;
        case READUID:
            if (kilo_state != MOVING) {
                motors_on();
                set_color(RGB(0, 0, 0));
                prev_motion = cur_motion = MOVE_STOP;
                kilo_state = MOVING;
            }

            if (kilo_uid&(1 << rx_msg.data[0]))
                cur_motion = MOVE_LEFT;
            else
                cur_motion = MOVE_STOP;
            break;
        default:
            break;
    }
}

/**
 * @brief Delays for the specified number of milliseconds
 * 
 * @param ms (Number of milliseconds to delay)
 */
void delay(uint16_t ms) {
    while (ms > 0) {
        _delay_ms(1);
        ms--;
    }
}

/**
 * @brief Sets the motors to the specified duty cycle
 * 
 * @param ccw (Duty cycle for the counterclockwise motor)
 * @param cw (Duty cycle for the clockwise motor)
 */
void set_motors(uint8_t ccw, uint8_t cw) {
    OCR2A = ccw;
    OCR2B = cw;
}

/**
 * @brief Spins up the motors by setting both to 100% duty cycle for 15 ms
 * 
 */
void spinup_motors() {
    set_motors(255, 255);
    delay(15);
}

/**
 * @brief Gets the ambient light level by reading from the ADC
 * 
 * @return int16_t Ambient light level as measured by the ADC, or -1 if the receiver is currently busy 
 */
int16_t get_ambientlight() {
    int16_t light = -1;
    if (!rx_busy) {
        cli();
        adc_setup_conversion(7);
        adc_start_conversion();
        adc_finish_conversion();
        light = ADCW;  // store AD result
        adc_trigger_high_gain();  // set AD to measure high gain (for distance sensing)
        sei();  // reenable interrupts
    }
    return light;
}

/**
 * @brief Gets the temperature by reading from the ADC
 * 
 * @return int16_t Temperature as measured by the ADC, or -1 if the receiver is currently busy
 */
int16_t get_temperature() {
    int16_t temp = -1;
    if (!rx_busy) {
        cli();
        ADMUX = (1<<3)|(1<<6)|(1<<7);
        ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
        adc_start_conversion();
        adc_finish_conversion();
        temp = ADCW;  // store AD result
        adc_trigger_high_gain();  // set AD to measure high gain (for distance sensing)
        sei();  // reenable interrupts
    }
    return temp;
}

/**
 * @brief Generates a random number using hardware-based randomization
 *
 * @details Generates an 8-bit random number using hardware-based randomization.
 * 
 * @return uint8_t (The generated random number)
 */
uint8_t rand_hard() {
    uint8_t num = 0;
    uint8_t a, b, i, tries;
    for (i = 0; i < 8; i++) {
        tries = 0;
        do {
            cli();  // disable interrupts

            adc_setup_conversion(6);
            adc_start_conversion();
            adc_finish_conversion();

            a = ADCW&0x1;  // read the least significant bit of the AD conversion result
            adc_start_conversion();
            adc_finish_conversion();

            b = ADCW&0x1;  // read another LSB from AD conversion
            adc_trigger_high_gain();  // set AD to measure high gain (for distance sensing)
            sei();  // reenable interrupts
            tries++;
        } while (a == b && tries < 128);  // repeat if the two LSBs are the same and less than 128 tries
        if (a)
            num |= (1 << i);
    }
    return num;
}

/**
 * @brief Generates a random number using software-based randomization
 *
 * @details Generates an 8-bit random number using software-based randomization.
 *
 * @return The generated random number
 */
static uint8_t seed = 0xaa, accumulator = 0;

uint8_t rand_soft() {
    seed ^= seed << 3;
    seed ^= seed >> 5;
    seed ^= accumulator++>>2;
    return seed;
}

/**
 * @brief Seeds the software-based random number generator.
 *
 * @details Sets the seed for the software-based random number generator.
 * 
 * @param s (The seed value)
 */
void rand_seed(uint8_t s) {
    seed = s;
}

/**
 * @brief Reads the battery voltage
 *
 * @details Reads the battery voltage using an analog-to-digital (AD) converter.
 * 
 * @return int16_t (The battery voltage in millivolts, or -1 if the RX module is busy)
 */
int16_t get_voltage() {
    int16_t voltage = -1;
    if (!rx_busy) {
        cli();  // disable interrupts

        adc_setup_conversion(6);
        adc_start_conversion();
        adc_finish_conversion();

        voltage = ADCW;  // store AD result
        // adc_trigger_high_gain();  // set AD to measure high gain (for distance sensing)
        sei();  // reenable interrupts
    }
    return voltage;
}

/**
 * @brief Estimate distance between two robots using IR distance measurements
 * 
 * This function estimates the distance between two robots using IR distance measurements.
 * It takes a pointer to a distance_measurement_t structure as input, which contains
 * the high and low gain IR distance measurements. It calculates the distance based on
 * the calibration values stored in the kilo_irlow and kilo_irhigh arrays.
 *
 * @param dist (Pointer to the distance_measurement_t structure containing IR distance measurements)
 * @return uint8_t (The estimated distance between two robots in millimeters)
 */
uint8_t estimate_distance(const distance_measurement_t *dist) {
    uint8_t i;
    uint8_t index_high = 13;
    uint8_t index_low = 255;
    uint8_t dist_high = 255;
    uint8_t dist_low = 255;

    if (dist->high_gain < 900) {
        if (dist->high_gain > kilo_irhigh[0]) {
            dist_high = 0;
        } else {
            for (i = 1; i < 14; i++) {
                if (dist->high_gain > kilo_irhigh[i]) {
                    index_high = i;
                    break;
                }
            }

            double slope = (kilo_irhigh[index_high]-kilo_irhigh[index_high-1])/0.5;
            double b = (double)kilo_irhigh[index_high] - (double)slope*((double)index_high*(double)0.5 + (double)0.0);
            b = (((((double)dist->high_gain-(double)b)*(double)10)));
            b = ((int)((int)b/(int)slope));
            dist_high = b;
        }
    }

    if (dist->high_gain > 700) {
        if (dist->low_gain > kilo_irlow[0]) {
            dist_low = 0;
        } else {
            for (i = 1; i < 14; i++) {
                if (dist->low_gain > kilo_irlow[i]) {
                    index_low = i;
                    break;
                }
            }

            if (index_low == 255) {
                dist_low = 90;
            } else {
                double slope = (kilo_irlow[index_low]-kilo_irlow[index_low-1])/0.5;
                double b = (double)kilo_irlow[index_low] - (double)slope*((double)index_low*(double)0.5 + (double)0.0);
                b = (((((double)dist->low_gain-(double)b)*(double)10)));
                b = ((int)((int)b/(int)slope));
                dist_low = b;
            }
        }
    }

    if (dist_low != 255) {
        if (dist_high != 255) {
            return 33 + ((double)dist_high*(900.0-dist->high_gain)+(double)dist_low*(dist->high_gain-700.0))/200.0;
        } else {
            return 33 + dist_low;
        }
    } else {
        return 33 + dist_high;
    }
}

/**
 * Timer0 interrupt.
 * Used to send messages every kilo_tx_period ticks.
 */

/**
  * @brief brief Interrupt service routine for Timer0 compare match A.
  * 
  * Used to send messages every kilo_tx_period ticks
  *
  * @return void
  *
  * @note This function is only compiled and used if it is not in bootloader mode.
  */
ISR(TIMER0_COMPA_vect) {
    tx_clock += tx_increment;
    tx_increment = 0xFF;
    OCR0A = tx_increment;
    kilo_ticks++;

    if (!rx_busy && tx_clock > kilo_tx_period && kilo_state == RUNNING) {
        message_t *msg = kilo_message_tx();
        if (msg) {
            if (message_send(msg)) {
                kilo_message_tx_success();
                tx_clock = 0;
            } else {
                tx_increment = rand()&0xFF;
                OCR0A = tx_increment;
            }
        }
    }
}

#else  //  BOOTLOADER

/**
 * @brief Process received message
 *
 * This function is only compiled and used if in bootloader mode.
 *
 * @return void
 */
static inline void process_message() {
    kilo_message_rx(&rx_msg, &rx_dist);
}

/**
 * @brief Empty interrupt service routine for Timer0 compare match A
 *
 * This function is only compiled and used if in bootloader mode
 *
 * @return void
 */
EMPTY_INTERRUPT(TIMER0_COMPA_vect)

#endif

/**
 * @brief Set LED color.
 *
 * The RGB value is bit-packed in a uint8_t variable. The 6 least significant bits are used to control the 6 LEDs:
 * Bit 0 controls the red LED.
 * Bit 1 controls the green LED.
 * Bit 2 controls the blue LED.
 * Bit 3 controls the yellow LED.
 * Bit 4 controls the magenta LED.
 * Bit 5 controls the cyan LED.
 * If a bit is set to 1, the corresponding LED is turned on; if it is set to 0, the LED is turned off.
 *
 * @param rgb The packed RGB value.
 * 
 * @return void 
 */
void set_color(uint8_t rgb) {
    if (rgb&(1<<0))
        DDRD |= (1<<5);
    else
        DDRD &= ~(1<<5);

    if (rgb&(1<<1))
        DDRD |= (1<<4);
    else
        DDRD &= ~(1<<4);

    if (rgb&(1<<2))
        DDRC |= (1<<3);
    else
        DDRC &= ~(1<<3);

    if (rgb&(1<<3))
        DDRC |= (1<<2);
    else
        DDRC &= ~(1<<2);

    if (rgb&(1<<4))
        DDRC |= (1<<5);
    else
        DDRC &= ~(1<<5);

    if (rgb&(1<<5))
        DDRC |= (1<<4);
    else
        DDRC &= ~(1<<4);
}

/**
 * @brief Timer1 interrupt function which is triggered if stop bit is not received.
 *
 * @details This function is responsible for the Timer1 interrupt. It turns off the
 * receive timer and sets the leading bit to 1. It also sets the receive busy flag 
 * to 0 and triggers the high gain ADC conversion.
 *
 * @return void
 */
ISR(TIMER1_COMPA_vect) {
    rx_timer_off();
    rx_leadingbit = 1;
    rx_leadingbyte = 1;
    rx_busy = 0;
    adc_trigger_high_gain();
}

/**
 * @brief  Analog comparator trigger interrupt function.
 *
 * @details This function is responsible for the Analog comparator trigger interrupt
 * which is triggered for incoming IR pulses (i.e. individual bits).
 * 
 * @return void
 */
ISR(ANALOG_COMP_vect) {
    uint16_t timer = TCNT1;

    rx_busy = 1;
    /* adc_trigger_stop(); */

    if (rx_leadingbit) {  //  Start bit received.
        rx_timer_on();
        rx_bytevalue = 0;
        rx_leadingbit = 0;
        if (rx_leadingbyte) {
            adc_finish_conversion();
            rx_dist.high_gain = ADCW;
            adc_trigger_low_gain();
        }
    } else {
        // Stray bit received
        if (timer <= rx_bitcycles/2 || timer >= rx_bitcycles*9+rx_bitcycles/2) {
            rx_timer_off();
            rx_leadingbit = 1;
            rx_leadingbyte = 1;
            rx_busy = 0;
            adc_trigger_high_gain();
        } else {
            // NOTE: The following code avoids a division which takes
            // too many clock cycles and throws off the interrupt.
            const uint16_t M = ((1L<<16)+rx_bitcycles-1)/rx_bitcycles;
            uint8_t bitindex = ((uint32_t)(timer-rx_bitcycles/2)*M)>>16;
            if (bitindex <= 7) {  //  Data bit received.
                rx_bytevalue |= (1 << bitindex);
            } else {  //  Stop bit received.
                rx_leadingbit = 1;
                if (rx_leadingbyte) {
                    adc_finish_conversion();
                    rx_dist.low_gain = ADCW;
                    adc_trigger_high_gain();
                    if (rx_bytevalue != 0) {  //  Collision detected.
                        rx_timer_off();
                        rx_leadingbyte = 1;
                        rx_busy = 0;
                    } else {  //  Leading byte received.
                        rx_leadingbyte = 0;
                        rx_byteindex = 0;
                    }
                } else {
                    rawmsg[rx_byteindex] = rx_bytevalue;
                    rx_byteindex++;
                    if (rx_byteindex == sizeof(message_t)) {
                        rx_timer_off();
                        rx_leadingbyte = 1;
                        rx_busy = 0;

                        if (rx_msg.crc == message_crc(&rx_msg))
                            process_message();
                    }
                }
            }
        }
    }
}
