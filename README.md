# Simple Timer Embedded System

## Overview

This project involves designing and implementing a simple embedded timer system using the AVR-BLE microcontroller. The timer uses LEDs to display time in binary, buttons for user interaction, and a speaker for sound notifications. The timer supports the following features:

- **Digital Output and Control:** Eight LEDs represent numbers in binary from 0 to 255.
- **Digital Input:** Buttons to modify the timer’s duration and start/cancel the timer.
- **Interrupt Handling:** Button interactions are handled via interrupts.
- **Timer Interrupts:** Countdown operation uses periodic timer interrupts.
- **Sound Notification:** A speaker emits sound upon timer expiration.

---

## Hardware Setup

### Components
- **Microcontroller:** AVR-BLE
- **LEDs:** 8 LEDs with resistors to display the count in binary.
- **Buttons:** Three buttons for user input.
- **Speaker:** Emits sound when the timer expires.
- **Resistors:** Six 220 Ω and two 470 Ω resistors for LEDs.

### Circuit Connections
#### LEDs for Count Display
- Connect LEDs to the following pins:

| LED Index | Color  | Output Pin |
|-----------|--------|------------|
| 0         | Red    | D7         |
| 1         | Red    | D5         |
| 2         | Yellow | A7         |
| 3         | Yellow | A6         |
| 4         | Green  | A5         |
| 5         | Green  | A4         |
| 6         | Blue   | A3         |
| 7         | Blue   | A2         |

- Use 220 Ω resistors for most LEDs and 470 Ω resistors for blue LEDs.
- Arrange LEDs with high-index on the left and low-index on the right.
- LED states reflect the binary representation of the count value.

#### Buttons for Count Modification
- **Increment Button:** Connected to pin C0.
- **Up5 Button:** Connected to pin C1.
- **Start/Cancel Button:** Connected to pin D6.
- Configure interrupts to detect button release events.

#### Speaker for Notification
- Connect the speaker input to pin D1.
- The speaker emits sound by toggling voltage between high and low.

---

## Software Behavior

### Modes of Operation
1. **Setup Mode:**
   - Adjust the timer’s duration using the buttons:
     - Increment Button: Increase count by 1 (max 255).
     - Up5 Button: Increase count by 5 (max 255).
   - Display the count value using LEDs.
   - Press Start/Cancel button to begin countdown.

2. **Countdown Mode:**
   - Decrease the count value by 1 per second using timer interrupts.
   - Ignore Increment and Up5 button inputs during countdown.
   - Display the current count value using LEDs.
   - Press Start/Cancel button to cancel countdown and reset count to 0.

3. **Expiration Mode:**
   - Play a sound for 1 second by toggling the speaker output at 250 Hz.
   - Reset system to setup mode after the sound completes.

---

Created as part of an embedded systems course project to demonstrate fundamental concepts in microcontroller-based design.
