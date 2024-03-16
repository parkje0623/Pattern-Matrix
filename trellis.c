#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include "trellis.h"
#include "seeSaw.h"
#include "utils.h"
#include "keypad.h"

#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#define PIXELS_NUM 16
#define KEYS_NUM 16
#define TOTAL_EVENT_NUMBERS 16

/*
 *Registers for LEDS
 */

#define TRELLIS_MODULE_BASE 0x0E /*module base register*/
#define TRELLIS_FUNC_STATUS 0x00
#define TRELLIS_FUNC_PIN 0x01
#define TRELLIS_FUNC_BUFLEN 0x03
#define TRELLIS_FUNC_BUF 0x04
#define TRELLIS_FUNC_SHOW 0x05
#define TRELLIS_EVENT_RISING_EDGE 0x11
#define TRELLIS_EVENT_FALLING_EDGE 0x09
#define TRELLIS_BUFFER_SIZE_MSB_8 0x00
#define TRELLIS_BUFFER_SIZE_LSB_8 0x30
#define TRELLIS_OUTPUT_PIN 0x3
#define MAX_ROUNDS 3
#define MAX_NEIGHBOURS 9
/*
 * Registers for pushed buttons
 */
#define KEYPAD_MODULE_BASE 0x10
#define KEYPAD_FIFO 0x10
#define KEYPAD_EVENT 0x01
#define KEYPAD_EVENT_COUNT 0x04

/*
 *Function prototypes
 */
static inline int getEventKey(unsigned int val);
static inline void setEventForButton(unsigned int index, unsigned char edge, bool enable);
static void setEventsForAllButtons(unsigned char edge, bool enable);
static inline void enableEventsForAllButtons();
static inline void disableEventsForAllButtons();
static int readAllEvents_from_buttons(unsigned char *buff, unsigned int buff_size);
static int readEventsNum(void);
static void clearUpAllPreviousEvents(void);
static color nocolor;

static inline int getEventKey(unsigned int val)
{
    return val / 4 * 8 + val % 4;
}

static inline int getSeeSawKey(unsigned int val)
{
    return val / 8 * 4 + val % 8;
}

static void setEventForButton(unsigned int index, unsigned char edge, bool enable)
{

    SeeSaw_Write(4, KEYPAD_MODULE_BASE, KEYPAD_EVENT, getEventKey(index), edge | enable);
}

static void setEventsForAllButtons(unsigned char edge, bool enable)
{
    for (int i = 0; i < KEYS_NUM; i++)
    {
        setEventForButton(i, edge, enable);
    }
}

static void clearUpAllPreviousEvents()
{
    unsigned char eventsBuff[TOTAL_EVENT_NUMBERS] = {0x00};
    int eventsCounter = readEventsNum();
    readAllEvents_from_buttons(eventsBuff, min(eventsCounter + 2, TOTAL_EVENT_NUMBERS));

    while (eventsCounter > 0)
    {
        readAllEvents_from_buttons(eventsBuff, min(eventsCounter + 2, TOTAL_EVENT_NUMBERS));
        eventsCounter = readEventsNum();
    }
}

static int readEventsNum()
{
    int eventsNum = SeeSaw_byteRead(KEYPAD_MODULE_BASE, KEYPAD_EVENT_COUNT);
    return eventsNum;
}

static inline void enableEventsForAllButtons()
{
    setEventsForAllButtons(TRELLIS_EVENT_RISING_EDGE, true);
}

static inline void disableEventsForAllButtons()
{
    setEventsForAllButtons(TRELLIS_EVENT_RISING_EDGE, false);
}

static int readAllEvents_from_buttons(unsigned char *buff, unsigned int buff_size)
{
    int bytesRead = SeeSaw_Read(KEYPAD_MODULE_BASE,
                                KEYPAD_FIFO,
                                buff,
                                buff_size);
    return bytesRead;
}

void NeoTrellis_LEDs_Init(void)
{
    SeeSaw_Init();
    nocolor.blue = 0x00;
    nocolor.red = 0x00;
    nocolor.green = 0x00;
    /*
     *set the buffer size for LEDs buffer, its 48 bytes
     *3 bytes for each of the 16 pixels
     */
    SeeSaw_Write(4,
                 TRELLIS_MODULE_BASE,
                 TRELLIS_FUNC_BUFLEN,
                 TRELLIS_BUFFER_SIZE_MSB_8,
                 TRELLIS_BUFFER_SIZE_LSB_8);

    /*
     *Set the output pin
     */
    SeeSaw_Write(3,
                 TRELLIS_MODULE_BASE,
                 TRELLIS_FUNC_PIN,
                 TRELLIS_OUTPUT_PIN);
    /*
     *Enable push events for all indices in the matrix
     */
    enableEventsForAllButtons();
    // turns on all leds by writing to the show register
    color white;
    white.blue = 255;
    white.red = 255;
    white.green = 255;
    for (int i = 0; i < PIXELS_NUM; i++)
    {
        NeoTrellis_LEDs_SetPixel_to_Color(i, white);
        NeoTrellis_LEDs_UpdateTrellisBuff();
        sleepMs(90);
        NeoTrellis_LEDs_SetAllLEDs_to(nocolor);
    }
    NeoTrellis_LEDs_SetAllLEDs_to(nocolor);
    NeoTrellis_LEDs_UpdateTrellisBuff();
}

int NeoTrellis_Keys_getPushedButtonIndex(void)
{
    unsigned char eventsBuff[TOTAL_EVENT_NUMBERS] = {0x00};
    clearUpAllPreviousEvents();

    bool isGameOver = Keypad_getGameOver();
    while (!isGameOver)
    {
        int count = readEventsNum();

        if (count != 0)
        {

            int bytesRead = readAllEvents_from_buttons(eventsBuff, min(count + 2, TOTAL_EVENT_NUMBERS));

            for (int i = 0; i < bytesRead; i++)
            {
                // Check trellis file in blinka library for reference
                unsigned int eventNumber = getSeeSawKey((eventsBuff[i] >> 2) & 0x3F);

                if (eventNumber < TOTAL_EVENT_NUMBERS)
                {
                    return eventNumber;
                }
            }
        }
        sleepMs(5);
        isGameOver = Keypad_getGameOver();
    }
    return -1;
}

void NeoTrellis_LEDs_SetPixel_to_Color(unsigned int index, color color)
{
    SeeSaw_Write(7,
                 TRELLIS_MODULE_BASE,
                 TRELLIS_FUNC_BUF,
                 0x00,
                 index * 3,
                 color.green,
                 color.red,
                 color.blue);
}

void NeoTrellis_LEDs_SetAllLEDs_to(color color)
{
    for (int i = 0; i < PIXELS_NUM; i++)
    {
        NeoTrellis_LEDs_SetPixel_to_Color(i, color);
    }
}

void NeoTrellis_LEDs_TurnAllLEDs_off(void)
{
    for (int i = 0; i < PIXELS_NUM; i++)
    {
        NeoTrellis_LEDs_TurnLED_off(i);
    }
}

void NeoTrellis_LEDs_TurnLED_off(unsigned int index)
{
    NeoTrellis_LEDs_SetPixel_to_Color(index, nocolor);
    NeoTrellis_LEDs_UpdateTrellisBuff();
}

void NeoTrellis_LEDs_UpdateTrellisBuff(void)
{
    SeeSaw_Write(2, TRELLIS_MODULE_BASE, TRELLIS_FUNC_SHOW);
}

void NeoTrellis_LEDs_Destroy(void)
{
    SeeSaw_Destroy();
}
