#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "joystick.h"
#include "keypad.h"
#include "difficulty.h"

#define CONFIG_PIN_CMD "config-pin p8.43 gpio"
#define EXPORT_GPIO "/sys/class/gpio/export"

#define GPIO_JSUP "26"
#define GPIO_JSDN "46"
#define GPIO_JSRT "47"
#define GPIO_JSLFT "65"

#define JSUP_INPUT "/sys/class/gpio/gpio26/direction"
#define JSDN_INPUT "/sys/class/gpio/gpio46/direction"
#define JSRT_INPUT "/sys/class/gpio/gpio47/direction"
#define JSLFT_INPUT "/sys/class/gpio/gpio65/direction"

#define JOYSTICK_UP "/sys/class/gpio/gpio26/value"
#define JOYSTICK_DN "/sys/class/gpio/gpio46/value"
#define JOYSTICK_RT "/sys/class/gpio/gpio47/value"
#define JOYSTICK_LFT "/sys/class/gpio/gpio65/value"

#define DIRECTION_IN "in"
#define NOT_PRESSED "1"
#define PRESSED "0"

static bool terminateProgram = false;
static bool inSetting = false;
static int debounce = 500;

static void printJoystickMainMenuAction(void);
static void enterSetting(void);
static enum EnumJoystickDirection getDirection(void);

// Initialize Joystick
void Joystick_init(void)
{
    // Config pin (tell BBG - using pin for GPIO)
    runCommand(CONFIG_PIN_CMD);

    // GPIO Export (Handle pin as GPIO)
    openFileToWrite(EXPORT_GPIO, GPIO_JSUP);
    openFileToWrite(EXPORT_GPIO, GPIO_JSDN);
    openFileToWrite(EXPORT_GPIO, GPIO_JSRT);
    openFileToWrite(EXPORT_GPIO, GPIO_JSLFT);
    sleepForMs(500);

    // Make GPIO pin an Input
    openFileToWrite(JSUP_INPUT, DIRECTION_IN);
    openFileToWrite(JSDN_INPUT, DIRECTION_IN);
    openFileToWrite(JSRT_INPUT, DIRECTION_IN);
    openFileToWrite(JSLFT_INPUT, DIRECTION_IN);

    // Init. GPIO pin value to NOT_PRESSED
    openFileToWrite(JOYSTICK_UP, NOT_PRESSED);
    openFileToWrite(JOYSTICK_DN, NOT_PRESSED);
    openFileToWrite(JOYSTICK_RT, NOT_PRESSED);
    openFileToWrite(JOYSTICK_LFT, NOT_PRESSED);

    // Start the Program
    printf("Welcome to Memorization Game!\n");
    mainMenu();
}

// Clean Up Joystick
void Joystick_cleanUp(void)
{
    printf("Clean Up!\n");
}

// Main menu of the application
void mainMenu(void)
{
    printJoystickMainMenuAction();
    while (!terminateProgram)
    {
        enum EnumJoystickDirection joyStickDirection = getDirection();
        if (joyStickDirection == LEFT)
        {
            // Enter to Start Game
            Keypad_init();
            // Change the text color back to 'black'
            printf("\033[0;30m");
            printf("\n");
            printJoystickMainMenuAction();
            sleepForMs(debounce);
        }
        else if (joyStickDirection == RIGHT)
        {
            // Enter Setting
            inSetting = true;
            sleepForMs(debounce);
            enterSetting();
            // Change the text color back to 'black'
            printf("\033[0;30m");
            printf("\n");
            sleepForMs(debounce);
            printJoystickMainMenuAction();
        }
        else if (joyStickDirection == UP)
        {
            // How to Play Guide
            printf("\033[0;31m"); // Change the text colors to 'red'
            printf("----- How To Play Guide -----\n");
            printf("Memorize the sequence of flashing lights on a keypad.\n"
                   "When the sequence ends, entire board will flash green.\n"
                   "Using your memory, repeat the sequence of flashing lights by pressing the keypad.\n"
                   "There are total of 3 levels of the sequence to win the game.\n"
                   "May change the difficulty of the sequence in Setting.\n"
                   "Train your brain by repeating the flashings. Have Fun!\n\n");
            // Change the text color back to 'black' and re-print the Menu
            printf("\033[0;30m");
            printJoystickMainMenuAction();
            sleepForMs(debounce);
        }
        else if (joyStickDirection == DOWN)
        {
            // Quit (Terminate a Program)
            printf("Exiting Memorization Game.\n");
            terminateProgram = true;
        }
    }

    // Clean Up process when terminating the application completely.
    Joystick_cleanUp();
    Keypad_cleanUp();
}

// Print the List of Joystick Action in Main Menu
static void printJoystickMainMenuAction(void)
{
    printf("\033[0m");
    printf("Control the Joystick to take following actions:\n");
    printf("  Direction 'LEFT'  --  Game Start!\n");
    printf("  Direction 'RIGHT' --  Setting\n");
    printf("  Direction 'UP'    --  How To Play\n");
    printf("  Direction 'DOWN'  --  Quit\n\n");
}

// Entered Setting to handle the difficulty of the game
static void enterSetting(void)
{
    printf("\033[0;31m"); // Change the text colors to 'red'
    printf("----- Setting -----\n");
    printf("\033[0;33m"); // Change the text colors to 'blue'
    printf("Direction 'LEFT'   --  Exit Setting\n"
           "Direction 'RIGHT'  --  Change Difficulty\n");

    while (inSetting)
    {
        enum EnumJoystickDirection joyStickDirection = getDirection();
        if (joyStickDirection == LEFT)
        {
            // Exit Setting
            printf("Exit Setting.\n");
            inSetting = false;
        }
        else if (joyStickDirection == RIGHT)
        {
            // Increase the difficulty by 1 by calling changeDifficulty() function.
            printf("\033[0;31m"); // Change the text colors to 'red'
            printf("CHANGING DIFFICULTY ENTERED\n");
            changeDifficulty();
            printf("\033[0;33m"); // Change the text colors to 'blue'
            printf("Direction 'LEFT'   --  Exit Setting\n"
                   "Direction 'RIGHT'  --  Change Difficulty\n");
            sleepForMs(debounce);
        }
    }
}

// After a user completes all levels of the difficulty,
//    asked to move the Joystick for following options
void Joystick_optionProceedNextDifficulty(void)
{
    // Change the text color back to 'black'
    printf("\033[0;30m");
    int currentDifficulty = getDifficulty();
    bool optionDifficulty = true;
    // If current difficulty is not the max difficulty, ask for Joystick input
    if (currentDifficulty < 4)
    {
        printf("Proceed to next difficulty? or Stay at current difficulty?\n");
        printf("Direction 'LEFT'   --  Play current difficulty again\n"
               "Direction 'RIGHT'  --  Proceed and Play Next Difficulty\n"
               "Direction 'DOWN'     --  Back to Main Menu\n");
        while (optionDifficulty)
        {
            enum EnumJoystickDirection joyStickDirection = getDirection();
            if (joyStickDirection == LEFT)
            {
                // No changes to the difficulty
                printf("No changes to the difficulty.\n");
                optionDifficulty = false;
            }
            else if (joyStickDirection == RIGHT)
            {
                // Proceed to next difficulty
                printf("\033[0;31m"); // Change the text colors to 'red'
                changeDifficulty();
                sleepForMs(debounce);
                optionDifficulty = false;
            }
            else if (joyStickDirection == DOWN)
            {
                // Exit to Main Menu
                printf("\033[0;31m"); // Change the text colors to 'red'
                printf("Exit to Main Menu.\n");
                Keypad_setGameOver();
                sleepForMs(debounce);
                optionDifficulty = false;
            }
        }
    }
    else
    {
        // If current difficulty is max difficulty, congrats and exit to the main menu.
        printf("Already at Max. Difficulty! Good Job Completing the Highest Difficulty!\n");
        Keypad_setGameOver();
    }
}

// Get the Direction of the Joystick being Pressed
static enum EnumJoystickDirection getDirection(void)
{
    if (openFileToRead(JOYSTICK_UP) == 0)
    {
        return UP;
    }
    else if (openFileToRead(JOYSTICK_DN) == 0)
    {
        return DOWN;
    }
    else if (openFileToRead(JOYSTICK_RT) == 0)
    {
        return RIGHT;
    }
    else if (openFileToRead(JOYSTICK_LFT) == 0)
    {
        return LEFT;
    }
    else
    {
        // If Joystick not UP, DOWN, LEFT, or RIGHT
        return NO_ACTION;
    }
}

// Globally used function to sleep the program for given Ms.
void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;

    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;

    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

void runCommand(char *command)
{
    // Execute command
    FILE *pipe = popen(command, "r");

    // Ignore output, consume it - no error when closing pipe
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe))
    {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
        {
            break;
        }
    }

    // Get exit code from pipe; non-zero is an error
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0)
    {
        perror("Unable to execute command:");
        printf("    command:    %s\n", command);
        printf("    exit code: %d\n", exitCode);
    }
}

void openFileToWrite(char *filename, char *writeStr)
{
    FILE *pFile = fopen(filename, "w");
    if (pFile == NULL)
    {
        printf("ERROR OPENING %s.", filename);
        exit(1);
    }

    int charWritten = fprintf(pFile, writeStr);
    if (charWritten <= 0)
    {
        printf("ERROR WRITING DATA TO %s.", filename);
        exit(1);
    }
    fclose(pFile);
}

int openFileToRead(char *filename)
{
    FILE *pFile = fopen(filename, "r");
    if (pFile == NULL)
    {
        printf("ERROR: Unable to open file (%s) for read.\n", filename);
        exit(-1);
    }

    // Read String (line)
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);

    // Close
    fclose(pFile);

    return atoi(buff);
}