/**
 * @file battery_percentage.c
 * @author Joseph Katakam
 *
 * @brief This code indicates the battery percentage using different kilobot LED colors.
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// kilolib library
#include "../../kilolib/kilolib.h"

/**
 * @brief Voltage levels in a kilobot.
          1. The minimum voltage is around 2.2V, below which the Kilobot's behavior may become unstable,
          and it may not operate as expected.
          2. The maximum voltage is around 3.6V, beyond which the Kilobot's components may get damaged,
          and it may not function properly.
 * 
 */

// Define constants for the battery voltage thresholds
#define VOLTAGE_MIN 2200  // mV
#define VOLTAGE_LOW 2800  // mV
#define VOLTAGE_MED 3200  // mV
#define VOLTAGE_HIGH 3600  // mV

// Define LED color constants
#define LED_OFF          RGB(0, 0, 0)
#define LED_WHITE        RGB(1, 1, 1)
#define LED_GREEN        RGB(0, 2, 0)
#define LED_YELLOW       RGB(2, 2, 0)
#define LED_RED          RGB(2, 0, 0)

// Function to set the LED color based on the current battery voltage
void set_led_color() {
    uint16_t voltage = get_voltage();

    if (voltage < VOLTAGE_MIN) {
        // Battery voltage is too low - turn off the LED
        set_color(LED_OFF);
    } else if (voltage < VOLTAGE_LOW) {
        // Battery voltage is low - set LED to red
        set_color(LED_RED);
    } else if (voltage < VOLTAGE_MED) {
        // Battery voltage is medium - set LED to yellow
        set_color(LED_YELLOW);
    } else if (voltage < VOLTAGE_HIGH) {
        // Battery voltage is high - set LED to green
        set_color(LED_GREEN);
    } else {
        // Battery voltage is too high - set LED to blue
        set_color(LED_WHITE);
    }
}

/**
 * @brief Kilobot Setup
 * 
 */
void setup() {
    // Initialize the Kilobot
    kilo_init();

    // Set the LED color based on the current battery voltage
    set_led_color();
}

/**
 * @brief Kilobot Loop
 * 
 */
void loop() {
    // Continuously update the LED color based on the current battery voltage
    set_led_color();
    delay(1000);  // Wait for 1 second before updating again
}

int main() {
    // Start the Kilobot program
    kilo_start(setup, loop);

    // Return 0 (never reached)
    return 0;
}
