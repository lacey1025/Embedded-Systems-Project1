/*
 * File:   newavr-main.c
 * Author: laceysemansky
 *
 * Created on September 28, 2024, 1:53 PM
 */


#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#define F_CPU 3333333

volatile uint8_t count = 0; // Time count
volatile uint8_t start_button_pressed = 0; // Start/stop flag
volatile uint8_t is_running = 0; // Status of timer - either running or in setting mode
volatile uint8_t count_changed = 0; // Flag for when the count changes value
volatile uint8_t incr_1_pressed = 0; // Flag for increment button
volatile uint8_t incr_5_pressed = 0; // Flag for up5 button


// checks to ensure button release is settled
uint8_t button_debounce(uint8_t port, uint8_t pin_bm) {
    if (port & pin_bm) { // check if button is not pressed
        _delay_ms(20); // wait and check again
        if (port & pin_bm) {
            return 1;
        }
    }
    return 0;
}

// Handle TCA overflow interrupt - aka TCA reaches one second
ISR(TCA0_OVF_vect) {
    count_changed = 1;
    TCA0.SINGLE.INTFLAGS &= TCA_SINGLE_OVF_bm;
}

//interrupt handler for PORTC - increment and Up5 buttons
ISR(PORTC_PORT_vect) {
    // if interrupt came from increment button
    if (PORTC.INTFLAGS & PIN0_bm) {
        if (button_debounce(PORTC.IN, PIN0_bm)) {
            incr_1_pressed = 1;
        }
        PORTC.INTFLAGS &= PIN0_bm;
    }
    // if interrupt came from Up5 button
    else if (PORTC.INTFLAGS & PIN1_bm) {
        if (button_debounce(PORTC.IN, PIN1_bm)) {
            incr_5_pressed = 1;
        }
        PORTC.INTFLAGS &= PIN1_bm;
    }
}

//interrupt handler for PORTD - start/cancel button
ISR(PORTD_PORT_vect) {
    // if interrupt came from start/cancel button
    if (PORTD.INTFLAGS & PIN6_bm) {
        if (button_debounce(PORTC.IN, PIN0_bm)) {
            start_button_pressed = 1;
        }
        PORTD.INTFLAGS &= PIN6_bm;
    }
}


// This function is used to set up some of the registers necessary for the project's function
void init_io() {
    // LEDs and speaker set to output
    PORTA.DIR |= PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
    PORTD.DIR |= PIN5_bm | PIN7_bm | PIN1_bm;
    // Buttons set to input
    PORTC.DIR &= ~(PIN0_bm | PIN1_bm);
    PORTD.DIR &= ~PIN6_bm;
    // enable pullup resistor and set interrupt for rising edge (button released)
    PORTC.PIN0CTRL |= PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
    PORTC.PIN1CTRL |= PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
    PORTD.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
}

// Set up the TCA
void init_tca() {
    // enable overflow interrupt
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    // set normal waveform mode
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    // disable event counting
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
    // set the period
    TCA0.SINGLE.PER = 52082;
    // set the clock source
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;
}

// This function is used to perform all the work after the flags were set by the ISRs
void handle_button_presses() {
    // Start/stop button pressed. Starts the timer if its not currently running. Otherwise resets back to setting mode
    if (start_button_pressed) {
        if (!is_running) {
            start_timer();
        }
        else {
            reset();
        }
        start_button_pressed = 0;
        is_running = !is_running;
    }
    // Increment 1 button pressed. Checks for overflow, increases count, and enables/disables flags
    if (incr_1_pressed) {
        if (count < 255) {
            count++;
            count_changed = 1;
        }
        incr_1_pressed = 0;
    }
    // Increment 5 button pressed.
    if (incr_5_pressed) {
        if (count <= 250) {
            count += 5;
            count_changed = 1;
        }
        incr_5_pressed = 0;
    }
}

// Runs only when the count has changed. It either decrements the count variable or deals with the timer running out
void handle_count_change() {
    if (is_running && count > 0) {
        count -= 1;
    }
    else if (is_running && count <= 0) { // Timer ran out
        is_running = 0;
        play_sound();
        reset();
    }
    display_count_value();
    count_changed = 0;
}

// This function turns on the LEDs that correspond to the count value
void display_count_value() {
    // Clear all pins
    PORTA.OUTCLR = PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
    PORTD.OUTCLR = PIN5_bm | PIN7_bm;

    if (count & 1) {
        PORTD.OUTSET = PIN7_bm;
    }
    if (count & 2) {
        PORTD.OUTSET = PIN5_bm;
    }
    if (count & 4) {
        PORTA.OUTSET = PIN7_bm;
    }
    if (count & 8) {
        PORTA.OUTSET = PIN6_bm;
    }
    if (count & 16) {
        PORTA.OUTSET = PIN5_bm;
    }
    if (count & 32) {
        PORTA.OUTSET = PIN4_bm;
    }
    if (count & 64) {
        PORTA.OUTSET = PIN3_bm;
    }
    if (count & 128) {
        PORTA.OUTSET = PIN2_bm;
    }
}

// Executed when the start button is pressed after setting the desired countdown value
void start_timer() {
    // Disable interrupts from increment buttons
    PORTC.PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    PORTC.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    // Start timer
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

// Generate square waves using _delay_ms macro
void play_sound() {
    uint16_t spkrCount = 0;
    
    for (uint8_t i=0; i<3; i++) {
        while (spkrCount < 250) {
            PORTD.OUT |= PIN1_bm;
            _delay_ms(1.276);
            PORTD.OUT &= ~PIN1_bm;
            _delay_ms(1.276);
            spkrCount++;
        }
        _delay_ms(30);
        spkrCount = 0;
    }
    while (spkrCount < 1590) {
        PORTD.OUT |= PIN1_bm;
        _delay_ms(1.607);
        PORTD.OUT &= ~PIN1_bm;
        _delay_ms(1.607);
        spkrCount++;
    }
    spkrCount = 0;
}

// Executed when the start button is pressed while the timer is running or when the timer reaches zero
void reset() {
    // Disable and reset timer
    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
    TCA0.SINGLE.CTRLESET |= TCA_SINGLE_CMD_RESTART_gc;
    // Enable interrupts from increment buttons
    PORTC.PIN0CTRL = (PORTC.PIN0CTRL & ~PORT_ISC_gm) | PORT_ISC_RISING_gc;
    PORTC.PIN1CTRL = (PORTC.PIN1CTRL & ~PORT_ISC_gm) | PORT_ISC_RISING_gc;
    count = 0;
    count_changed = 1;
}


int main(void) {
    init_io(); // Enable some of the bits necessary for input and output
    init_tca(); // Set up the TCA
    
    sei(); // Enable global interrupts
    
    while (1) {
        // Do work for button ISRs
        handle_button_presses();

        // Do work for timer ISR
        if (count_changed) {
            handle_count_change();
        }
    }
}



/* Thank you! That was a really fun one! */
