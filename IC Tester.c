/*
 * ATmega32 IC Tester Program
 * This code is designed for the ATmega32 microcontroller using the internal 1 MHz clock.
 * It tests the functionality of ICs. Currently supports SN74S138N, SN74260, and SN74S133. 
 * The program uses buttons to trigger the tests and displays the result on a 16x2 LCD.
 * 
 * The program includes three IC testing functions, each tailored to test the specific IC
 * by applying input combinations to the control pins and checking the output.
 * Test cases for other ICs can be added with the necessary modifications to the input/output logic and port configurations to test additional ICs.
 * Developed for educational purposes and integration with external hardware.
 */

#include <mega32.h>
#include <delay.h>
#include <alcd.h>

// Function to initialize ports for SN74S138N IC
void setup_ports_sn74s138n() {
    // Configure PORTC: I1, I2, I3, E1, E2, E3 as outputs; GND as output
    DDRC = 0xBF; // All as output except PORTC.6 (Q7)
    // Configure PORTA as input for Q0-Q6 except PORTA.0 as output
    DDRA = 0x01; // All pins of PORTA are inputs except PORTA.0 is output (VCC)

    // Set default values for enable inputs
    PORTC.3 = 0; // Enable line E1 = LOW
    PORTC.4 = 0; // Enable line E2 = LOW
    PORTC.5 = 1; // Enable line E3 = HIGH
    PORTC.7 = 0; // Ground connection
    PORTA.0 = 1; // VCC connection
}

// Function to test SN74S138N IC
char test_sn74s138n() {
    char match = 1; // Assume match is found initially
    unsigned char input;
    unsigned char expected_output;
    unsigned char actual_output;

    // Test all input combinations (0-7) for SN74S138N
    for (input = 0; input <= 7; input++) {
        setup_ports_sn74s138n();
        PORTC.0 = input & 1;
        PORTC.1 = (input >> 1) & 1;
        PORTC.2 = (input >> 2) & 1;

        delay_ms(20); // Delay to stabilize the IC

        // Calculate expected output based on input
        expected_output = ~(1 << (7 - input)); 
        expected_output = ((expected_output & 0x01) << 7) |
                          ((expected_output & 0x02) << 5) |
                          ((expected_output & 0x04) << 3) |
                          ((expected_output & 0x08) << 1) |
                          ((expected_output & 0x10) >> 1) |
                          ((expected_output & 0x20) >> 3) |
                          ((expected_output & 0x40) >> 5) |
                          ((expected_output & 0x80) >> 7);

        // Read the actual output from the IC
        actual_output = ((PINC >> 6) & 1) << 7 | (PINA & 0xFE) >> 1;

        // Check if the actual output matches the expected output
        if (actual_output != expected_output) {
            match = 0; // Mismatch found
            break;
        }
    }

    return match; // Return whether the IC matches expected behavior
}

// Function to initialize ports for SN74260 IC
void setup_ports_sn74260() {
    // Configure the direction of pins for inputs and outputs
    DDRC = 0x4F;  // Set the pins of PORTC
    DDRA = 0x7F;  // Set the pins of PORTA

    // Set VCC and GND connections
    PORTA.0 = 1;  // VCC should be HIGH
    PORTC.6  = 0;  // GND should be LOW
}

// Function to test SN74260 IC
char test_sn74260() {
    char error_detected = 0; // 0 = no error, 1 = error detected
    unsigned char input;

    // Test all input combinations (0-31) for SN74260
    for (input = 0; input < 32; input++) {
        setup_ports_sn74260();
        PORTC.0 = input & 1;
        PORTC.1 = (input >> 1) & 1;
        PORTC.2 = (input >> 2) & 1;
        PORTA.2 = (input >> 3) & 1;
        PORTA.1 = (input >> 4) & 1;

        PORTC.3 = input & 1;
        PORTA.6 = (input >> 1) & 1;
        PORTA.5 = (input >> 2) & 1;
        PORTA.4 = (input >> 3) & 1;
        PORTA.3 = (input >> 4) & 1;

        delay_ms(20); // Delay to stabilize the IC

        // Check for errors in the outputs
        if (input == 0) {
            if (PINC.4 != 1 || PINC.5 != 1) {
                error_detected = 1; // Error detected
                break;
            }
        } else {
            if (PINC.4 != 0 || PINC.5 != 0) {
                error_detected = 1; // Error detected
                break;
            }
        }
    }

    return !error_detected; // Return true if no errors were detected
}

// Function to initialize ports for SN74S133 IC
void setup_ports_sn74s133() {
    DDRC = 0xFF;  // Configure PORTC as output
    DDRA = 0x7F;  // Configure PA0–PA6 as output (PA7 remains input for IC outputs)
    PORTA = 0x01; // Set PA.0 (VCC) HIGH
    PORTC = 0x00; // Initialize PORTC outputs to LOW
    lcd_clear();  // Clear the LCD display
    delay_ms(20); // Stabilization delay
}

// Function to test SN74S133 IC
char test_sn74s133() {
    char match = 1; // Assume match is found initially
    unsigned char input;

    setup_ports_sn74s133(); // Initialize ports for SN74S133

    // Check if PA7 (IC output) is HIGH, indicating correct IC connection
    if ((PINA & 0x80) == 0) {
        match = 0; // No match found
    }

    // Test input combinations (0-12) for SN74S133
    for (input = 0; input < 13; input++) {
        if (!match) break;

        if (input < 7) {
            PORTC = (1 << input) | 0x80;
            PORTA = 0x01;
        } else {
            PORTC = 0x80;
            PORTA = (1 << (input - 7)) | 0x01;
        }

        delay_ms(20); // Delay to stabilize the IC

        // Check for expected output behavior
        if ((PINA & 0x80) == 0) {
            match = 0; // No match found
            break;
        }
    }

    // Reset the IC and check the final state
    PORTC = 0x7F;
    PORTA = 0x7F | 0x01;
    delay_ms(20);

    if (PINA & 0x80) {
        match = 0; // No match found
    }

    return match; // Return whether the IC matches expected behavior
}

// Main function
void main() {
    // Initialize LCD and Timer for display
    TCCR0 = (0 << WGM00) | (0 << COM01) | (0 << COM00) | (0 << WGM01) | (0 << CS02) | (0 << CS01) | (0 << CS00);
    TCNT0 = 0x00;
    OCR0 = 0x00;
    lcd_init(16); // Initialize 16x2 LCD display
    DDRD.3 = 0;   // Configure PIND.3 as input (Button)
    PORTD.3 = 1;  // Enable internal pull-up resistor

    while (1) {
        if (PIND.3 == 0) {  // Button pressed (logic LOW)
            delay_ms(50);    // Debounce delay
            if (PIND.3 == 0) {  // Confirm button is still pressed
                // Run the IC tests
                if (test_sn74s138n()) {
                    lcd_clear();
                    lcd_puts("SN74S138N"); // Display IC name on LCD
                } else if (test_sn74260()) {
                    lcd_clear();
                    lcd_puts("SN74LS260"); // Display IC name on LCD
                } else if (test_sn74s133()) {
                    lcd_clear();
                    lcd_puts("SN74S133"); // Display IC name on LCD
                } else {
                    lcd_clear();
                    lcd_puts("NO MATCH"); // Display error message
                }

                // Wait for the button to be released before continuing
                while (PIND.3 == 0) {
                    delay_ms(10); // Small delay to reduce CPU usage
                }
            }
        }

        delay_ms(10); // Small delay to avoid high CPU usage in the main loop
    }
}
