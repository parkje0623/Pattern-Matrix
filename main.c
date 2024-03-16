#include <stdio.h>
#include "joystick.h"
#include "trellis.h"

int main()
{
    NeoTrellis_LEDs_Init();
    Joystick_init();
    return 0;
}
