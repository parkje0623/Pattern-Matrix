// Manages actions depends on the Joystick Directions
// Actions include starting a game, entering/exitting settings.
// Additional actions to change the difficulty after completing the level.
#ifndef JOYSTICK_H
#define JOYSTICK_H

// Directions of the Joystick
enum EnumJoystickDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NO_ACTION
};

// init() must be called before any other functions.
// Export GPIO & initialize each direction to be NOT_PRESSED state
// cleanUp() must be called when stopping playback.
void Joystick_init(void);
void Joystick_cleanUp(void);

// Game's Main Menu
// A player may access actions such as Start Game, Setting, How To Play Guide, Quit
//     through pressing the Joystick in a certain direction.
void mainMenu(void);

// Called to sleep (delay) a process for given Ms
void sleepForMs(long long delayInMs);
// Run given commands on a command prompt
void runCommand(char* command);

void openFileToWrite(char* filename, char* writeStr);
int openFileToRead(char* filename);

// Get difficulty of the current game setting
int getDifficulty(void);

// When the user finishes all levels of the current difficulty
// User is given options to either play at current difficulty again
//  Or proceed to next difficulty and play (if at max difficulty, go to main menu)
//  Or exit to main menu
void Joystick_optionProceedNextDifficulty(void);

#endif