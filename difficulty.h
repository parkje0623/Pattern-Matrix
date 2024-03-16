// Handles Difficulty changes, fetching difficulty
#ifndef DIFFICULTY_H
#define DIFFICULTY_H

// Assign Enum value for the Number of Sequence for each Difficulty of the game
enum DIFFICULTY
{
    ROOKIE = 3,
    INTERMEDIATE = 5,
    ADVANCED = 7,
    MASTER = 9
};

// change difficulty - difficulty is increased by 1, if at max. difficulty, back to the lowest difficulty.
void changeDifficulty(void);

// get current difficulty
int getDifficulty(void);

// set difficulty
void setDifficulty(int difficulty);

#endif 