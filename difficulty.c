#include <stdio.h>
#include "difficulty.h"

// Set Initial difficulty as 1.
static int difficulty = 1;

// Increase difficulty by 1.
// If at max difficulty (4), change the difficulty back to 1.
void changeDifficulty(void)
{
    if (difficulty < 4)
    {
        difficulty++;
    }
    else
    {
        difficulty = 1;
    }
    printf("Difficulty Changed to %d.\n\n", difficulty);
}

// Retrieve current difficulty
int getDifficulty(void)
{
    return difficulty;
}

void setDifficulty(int diff) {
    difficulty = diff;
}
