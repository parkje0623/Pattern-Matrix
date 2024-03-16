#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include "segDisplay.h"
#include "joystick.h"
#include "keypad.h"

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

#define I2C_DEVICE_ADDRESS 0x20
// For Zen Cape Green
#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15
// For Zen Cape Red
#define RED_REG_DIRA 0x02
#define RED_REG_DIRB 0x03
#define RED_REG_OUTA 0x00
#define RED_REG_OUTB 0x01

#define I2C_RIGHT_DIGIT "/sys/class/gpio/gpio44/value"
#define I2C_LEFT_DIGIT "/sys/class/gpio/gpio61/value"

#define CONFIG_P9_18 "config-pin P9_18 i2c"
#define CONFIG_P9_17 "config-pin P9_17 i2c"
#define SET_DIRECTION_GPIO_61 "echo out > /sys/class/gpio/gpio61/direction"
#define SET_DIRECTION_GPIO_44 "echo out > /sys/class/gpio/gpio44/direction"
#define TURN_ON_GPIO_61 "echo 1 > /sys/class/gpio/gpio61/value"
#define TURN_ON_GPIO_44 "echo 1 > /sys/class/gpio/gpio44/value"

#define SECONDS 0
#define NANOSECONDS 5000000
static struct timespec reqDelay = {
    SECONDS,
    NANOSECONDS
};

static pthread_t segDisplay_thread;
static bool timeOver;
// Green Zen Cape
static int digitReg[10][2] = {{0xA1, 0x86}, {0x80, 0x12}, {0x31, 0x0F}, {0xB0,0x0E}, {0x90,0x8A}, {0xB0,0x8C}, {0xB1,0x8C}, {0x04,0x14}, {0xB1,0x8E}, {0xB0,0x8E}};
// Red Zen Cape
// static int digitReg[10][2] = {{0xD0, 0xA1}, {0xC0, 0x00}, {0x98, 0x83}, {0xD8, 0x03}, {0xC8, 0x22}, {0x58, 0x63}, {0x58, 0xA3}, {0xC0, 0x21}, {0xD8, 0xA3}, {0xD8, 0x63}};

static int initI2cBus(char *bus, int address);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
static void *displayTimer(void *arg);


// Initialize the Segment Setting (Config I2C and Export GPIO)
void Seg_init(void)
{
    char *configArray[] = {CONFIG_P9_18, CONFIG_P9_17,
                           SET_DIRECTION_GPIO_61, SET_DIRECTION_GPIO_44,
                           TURN_ON_GPIO_61, TURN_ON_GPIO_44};

    for (int i=0; i < sizeof(configArray) / sizeof(configArray[0]); i++)
    {
        runCommand(configArray[i]);
    }
    return;
}

// Initialize the Thread for the Timer
void Seg_threadInit(void) 
{
    timeOver = false;
    pthread_create(&segDisplay_thread, NULL, displayTimer, NULL);
}

// Cleanup the Thread for the Timer and set the timeOver to true
void Seg_threadCleanUp(void)
{
    timeOver = true;
    Seg_turnOffSegDisplay();
    pthread_join(segDisplay_thread, NULL);
}

// Turn off the Segement Display
void Seg_turnOffSegDisplay(void)
{
    openFileToWrite(I2C_RIGHT_DIGIT, "0");
    openFileToWrite(I2C_LEFT_DIGIT, "0");
}

// Function to display the timer
static void *displayTimer(void *arg)
{
    // Time Limit for each level of the game (60s)
    time_t start_time = time(NULL);
    time_t current_time;
    int elapsed_time = 15;
    while (!timeOver)
    {
        current_time = time(NULL);
        elapsed_time = (int)(current_time - start_time);
        if (elapsed_time % 1 == 0)
        {
            elapsed_time = 15 - elapsed_time;
            // Change Seg Display Timer
            int timerOnes = elapsed_time % 10;
            int timerTens = (elapsed_time / 10) % 10;
            // Turn off both digits
            Seg_turnOffSegDisplay();
            // Drive Display Pattern & Turn on Left Digit
            Seg_displayDigits(timerTens, I2C_LEFT_DIGIT);
            // Turn off both digits
            Seg_turnOffSegDisplay();
            // Drive Display Pattern & Turn on Right Digit
            Seg_displayDigits(timerOnes, I2C_RIGHT_DIGIT);
        }

        // If the Timer hits 0, set time over
        // Clean up thread and game over for keypad
        if (elapsed_time <= 0) 
        {
            Keypad_setGameOver();
            Seg_threadCleanUp();
            timeOver = true;
        }
    }
    return NULL;
}

// Turon on and Display the Number of Dips on the 14 segment
void Seg_displayDigits(int displayDigit, char *i2cDigitDirectionPath)
{
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

    // Green Zen Cape
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);
    writeI2cReg(i2cFileDesc, REG_OUTA, digitReg[displayDigit][0]);
    writeI2cReg(i2cFileDesc, REG_OUTB, digitReg[displayDigit][1]);

    // Red Zen Cape
    // writeI2cReg(i2cFileDesc, RED_REG_DIRA, 0x00);
    // writeI2cReg(i2cFileDesc, RED_REG_DIRB, 0x00);
    // writeI2cReg(i2cFileDesc, RED_REG_OUTA, digitReg[displayDigit][0]);
    // writeI2cReg(i2cFileDesc, RED_REG_OUTB, digitReg[displayDigit][1]);

    openFileToWrite(i2cDigitDirectionPath, "1");
    nanosleep(&reqDelay, (struct timespec *)NULL);
    close(i2cFileDesc);
    return;
}

// Initialize I2C Bus
static int initI2cBus(char *bus, int address)
{
    int i2cFileDesc = open(bus, O_RDWR);
    if (i2cFileDesc < 0)
    {
        printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
        perror("Error is:");
        exit(-1);
    }

    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0)
    {
        perror("Unable to set I2C device to slave address.");
        exit(-1);
    }
    return i2cFileDesc;
}

// Write I2C Register
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2)
    {
        perror("Unable to write i2c register");
        exit(-1);
    }
}
