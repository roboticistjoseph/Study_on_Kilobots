/**
 * @file macros.h
 * @author Team Harvard
 *
 * @brief Header file containing various macros for setup and control of a microcontroller system.
 * These include the ADC, analog comparator, charging status, motor control, and timers for transmitting and receiving data.
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#define adc_on()            ADCSRA |= (1 << ADEN)  //  Enable the ADC.
#define adc_off()           ADCSRA &= ~(1 << ADEN)  //  Disable the ADC.

#define acomp_on()          ACSR |= (1 << ACIE)|(1 << ACI)  //  Enable the analog comparator.
#define acomp_off()         ACSR &= ~(1 << ACIE)  //  Disable the analog comparator.

/**
 * @brief brief Check if the system is currently charging.
 *
 * @return True if the system is charging, false otherwise.
 */
#define is_charging()       ((PIND&(1<<0)) == 0)

// Configure the analog comparator for use with a specific microcontroller.
#define acomp_setup() {\
    ACSR = (1 << ACIE) | (1 << ACIS1) | (1 << ACIS0); /* trigger interrupt on rising output edge */\
    DIDR1 = 3; \
}

/**
 * @brief Configure the ADC for conversion on a specific channel.
 *
 * @param CHANNEL The channel to use for the conversion.
 */
#define adc_setup_conversion(CHANNEL) {\
    ADMUX = CHANNEL;\
    ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0);\
}

#define adc_start_conversion() ADCSRA |= (1 << ADSC)  //  Start an ADC conversion.

#define adc_finish_conversion() while ((ADCSRA&(1 << ADSC)))  //  Wait for an ADC conversion to complete.

// Configure the ADC to trigger on a low gain.
#define adc_trigger_low_gain() {\
    ADMUX = 1;\
    ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADPS1) | (1 << ADPS0);\
    ADCSRB = (1 << ADTS0);\
}

// Configure the ADC to trigger on a high gain.
#define adc_trigger_high_gain() {\
    ADMUX = 0;\
    ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADPS1) | (1 << ADPS0);\
    ADCSRB = (1 << ADTS0);\
}

// Set up the ADC for a specific microcontroller.
#define adc_setup() {\
    adc_setup_conversion(6);\
    adc_start_conversion();\
    adc_finish_conversion();\
}

#define adc_trigger_stop() ADCSRA &= ~(1 << ADATE)  //  Stop ADC triggering.

// Turn off all ports.
#define ports_off() {\
    DDRB = 0;\
    DDRC = 0;\
    DDRD = 0;\
    PORTB = 0;\
    PORTC = 0;\
    PORTD = 0;\
}

// Turn on the necessary ports.
#define ports_on() {\
    DDRD |= (1<<2);\
    PORTD |= (1<<2);\
}

// Set up motor control for a specific microcontroller.
#define motors_setup() {\
    DDRD &= ~(1<<3);\
    DDRB &= ~(1<<3);\
    TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM20);\
    TCCR2B = (1 << CS01);\
    OCR2B = 0;\
    OCR2A = 0;\
}

// Turn off both motors
#define motors_off() {\
    DDRD &= ~(1<<3);\
    DDRB &= ~(1<<3);\
    OCR2B = 0;\
    OCR2A = 0;\
}

// Turn on both motors
#define motors_on() {\
    DDRD |= (1<<3);\
    DDRB |= (1<<3);\
    OCR2B = 0;\
    OCR2A = 0;\
}

// Setup the transmission timer
#define tx_timer_setup() {\
    TCCR0A = 0;\
    TCCR0B = (1 << CS02) | (1 << CS00);   /* Set prescalar multiplier. */\
    OCR0A = 0xFF;                   /* Set compare register to 255. */\
    TIMSK0 = (1 << OCIE0A);           /* Enable timer1 interrupt. */\
}

// Turn on the transmission timer
#define tx_timer_on() {\
    TCNT0 = 0;             /* reset count. */\
    TIMSK0 |= (1 << OCIE0A); /* Enable timer1 interrupt. */\
}

// Turn off the transmission timer
#define tx_timer_off() {\
    TIMSK0 &= ~(1 << OCIE0A); /* Disable timer1 interrupt. */\
    TCNT0 = 0;              /* reset count. */\
}

// Setup the reception timer
#define rx_timer_setup() {\
    TCCR1A = 0;\
    TCCR1B = 0;             /* Set prescalar to 0 (disabled). */\
    OCR1A = rx_msgcycles;   /* Set compare register to rx_msgcycles. */\
    TIMSK1 = (1 << OCIE1A);   /* Interrupt enable on match output compare register A */\
}

// Turn on the reception timer
#define rx_timer_on() {\
    TCNT1 = 0;              /* reset count */ \
    TCCR1B = 1;             /* set prescalar to 1 (enabled). */\
}

// Turn off the reception timer
#define rx_timer_off() {\
    TCCR1B = 0;             /* set prescalar to 0 (disabled). */\
    TCNT1 = 0;              /* reset count */\
}
