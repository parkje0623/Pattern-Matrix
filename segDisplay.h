// Handles 14 Segment Display to display the Timer for the game.
#ifndef SEG_DISPLAY_H
#define SEG_DISPLAY_H

#define I2C_RIGHT_DIGIT "/sys/class/gpio/gpio44/value"
#define I2C_LEFT_DIGIT "/sys/class/gpio/gpio61/value"

// Initialize Segment Display
// config the pins and turn on the GPIO
void Seg_init(void);

// Turn off the Display for both digits
void Seg_turnOffSegDisplay(void);
// Display the digits of the given side (left or right)
void Seg_displayDigits(int displayDigit, char *i2cDigitDirectionPath);

// Initialize/CleanUp Segment Display Thread for the timer
// Used to run the timer while having players to press the buttons for the game. 
void Seg_threadInit(void);
void Seg_threadCleanUp(void);

#endif
