// Handles Keypad (Adafruit NeoTrellis RGB 4x4 Matrix Keypad)
// Keypad to generate random sequence & for user to press the keypad to play the game
// Handles Game Over, level complete, or game complete
#ifndef KEYPAD_H
#define KEYPAD_H

// Initialize/Clean Up the Keypad before its usage.
void Keypad_init(void);
void Keypad_cleanUp(void);

// Start the game:
//   Generate Random Sequence
//   Use may start entering the sequence after random sequence has been generated
//   Handles Game Over, level complete, or game complete depends on the user input
void userPlayGame(void);

// Set the game status to Game Over (isGameOver = false)
void Keypad_setGameOver(void);
// Get the current game status 
bool Keypad_getGameOver(void);

#endif
