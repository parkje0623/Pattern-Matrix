#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include "keypad.h"
#include "joystick.h"
#include "segDisplay.h"
#include "trellis.h"
#include "difficulty.h"

static int sequenceAmount;
static int currentLevel;
static bool isGameOver = false;
static int *sequence;
static color BLUE, RED, GREEN, YELLOW;
static int flashingSpeed;

static void generateRandomSequence(void);
static void getSequenceAmount(void);
static void lightOneKey(int key);
static void gameOver(void);
static void setColor(void);
static void flashColorOnBoard(color color, int timer);

// Initialize Keypad when starting
void Keypad_init(void)
{
    setColor();
    userPlayGame();
}

// clean up when ending game
void Keypad_cleanUp(void)
{
    free(sequence);
    NeoTrellis_LEDs_Destroy();
}

// Set the colors needed
static void setColor(void)
{
    BLUE.red = 0;
    BLUE.green = 0;
    BLUE.blue = 255;

    RED.red = 255;
    RED.green = 0;
    RED.blue = 0;

    GREEN.red = 0;
    GREEN.green = 255;
    GREEN.blue = 0;

    YELLOW.red = 255;
    YELLOW.green = 255;
    YELLOW.blue = 0;
}

// Get the number of sequence to be generated depends on the current difficulty from the setting
static void getSequenceAmount(void)
{
    int currDifficulty = getDifficulty();
    if (currDifficulty == 1)
    {
        sequenceAmount = ROOKIE;
    }
    else if (currDifficulty == 2)
    {
        sequenceAmount = INTERMEDIATE;
    }
    else if (currDifficulty == 3)
    {
        sequenceAmount = ADVANCED;
    }
    else
    {
        sequenceAmount = MASTER;
    }
    printf("Difficulty: %d, # of Sequence: %d\n", currDifficulty, sequenceAmount);
}

// Generate Random Sequence and flash each value generated
static void generateRandomSequence(void)
{
    // Light Up Entire Keypad before displaying the random sequence
    //     to notify the user that the sequence will begin
    printf("\033[0;30m");
    printf("Light Up Entire Keypad for 1 second before showing the random sequence.\n");
    flashColorOnBoard(YELLOW, 500);
    sleepForMs(1000);

    // Randomly pick and light up each random value for sequenceAmount times.
    sequence = malloc(sizeof(int) * 16);
    getSequenceAmount();
    srand(time(NULL)); // seed random number generator
    for (int i = 0; i < sequenceAmount; i++)
    {
        // Pick a random index between 0 and 15
        // Get the keypad value of randomly generated index
        // Store the values for right/wrong checking when user presses the keypad
        int index = rand() % 16;
        sequence[i] = index;

        // Light up a key of the randomly generated index every 1 second
        lightOneKey(index);
        sleepForMs(1000);
    }
    // Flash Entire board before the user can start pressing the buttons
    flashColorOnBoard(YELLOW, 500);
}

// Light One Key (given key to flash one of the key of the keypad)
static void lightOneKey(int key)
{
    // printf("Random Value: %u\n", key);
    NeoTrellis_LEDs_SetPixel_to_Color(key, BLUE);
    NeoTrellis_LEDs_UpdateTrellisBuff();
    // this determines how long the light will last
    sleepForMs(flashingSpeed);
    NeoTrellis_LEDs_TurnLED_off(key);
}

// Start Game
void userPlayGame(void)
{
    Seg_init();
    isGameOver = false;
    currentLevel = 1;
    flashingSpeed = 200;
    while (!isGameOver)
    {
        // Generate random sequence
        generateRandomSequence();

        // Start Segment Display Thread for 60 second timer
        Seg_threadInit();
        int count = 0;
        while (1)
        {
            // Accepts and receive the key pressed by the user & change the LED color to BLUE
            int pressed = NeoTrellis_Keys_getPushedButtonIndex();
            NeoTrellis_LEDs_SetPixel_to_Color(pressed, BLUE);
            NeoTrellis_LEDs_UpdateTrellisBuff();
            sleepForMs(100);
            NeoTrellis_LEDs_TurnAllLEDs_off();

            if (pressed != -1)
            {
                // printf("pressed: %d, sequence[%d]: %d\n", pressed, count, sequence[count]);

                // If wrong key pressed, flash the board with RED LED, then return to the main menu
                if (pressed != sequence[count])
                {
                    flashColorOnBoard(RED, 500);
                    printf("\033[0;31m"); // Change the text colors to 'red'
                    printf("Incorrect Keypad Pressed! Try Again!\n");
                    Seg_threadCleanUp();
                    isGameOver = true;
                    break;
                }
                else
                {
                    // If correct key pressed, increment count
                    count++;
                    // if all button pressed correctly, flash the entire board with GREEN LED
                    if (count == sequenceAmount)
                    {
                        flashColorOnBoard(GREEN, 500);
                        printf("\033[92m"); // Change the text colors to 'green'
                        if (currentLevel < 3)
                        {
                            // If current level is not the max level, increase the level
                            //    LED Flashing speed reduced by 50
                            printf("Level %d Completed!\n", currentLevel);
                            flashingSpeed -= 70;
                            currentLevel++;
                            sleepForMs(1000);
                        }
                        else
                        {
                            // If current level is the max level, congrats the user and provide options to
                            //   Play at current level again, proceed to next level or exit to main menu using the Joystick
                            printf("Congratulations, max level %d completed!!!\n", currentLevel);
                            Seg_threadCleanUp();
                            Joystick_optionProceedNextDifficulty();
                            // printf("\033[0;30m");
                            sleepForMs(1000);
                            // Reset the level and flashing speed to initial value
                            currentLevel = 1;
                            flashingSpeed = 200;
                            break;
                            // isGameOver = true;
                        }
                        Seg_threadCleanUp();
                        // printf("\033[0;30m");
                        break;
                    }
                }
            }
            else
            {
                // If button have not been pressed until the time is over
                // Flash the entire board with RED LED, and exit to the main menu.
                flashColorOnBoard(RED, 500);
                printf("\033[0;31m"); // Change the text colors to 'red'
                printf("Time Over! Try Again!\n");
                break;
            }
        }
    }

    // GAME OVER
    gameOver();
}

// Set the game status to Game Over
void Keypad_setGameOver(void)
{
    isGameOver = true;
}

// Get the current game status
bool Keypad_getGameOver(void)
{
    return isGameOver;
}

// Flash the entire board with the passed LED color for the length of the passed time.
static void flashColorOnBoard(color color, int timer)
{
    NeoTrellis_LEDs_SetAllLEDs_to(color);
    NeoTrellis_LEDs_UpdateTrellisBuff();
    sleepForMs(timer);
    NeoTrellis_LEDs_TurnAllLEDs_off();
}

// When game is over, turn off the timer
static void gameOver(void)
{
    Seg_turnOffSegDisplay();
}
