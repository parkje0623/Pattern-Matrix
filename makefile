TARGET = pattern_matrix
SOURCES= main.c joystick.c keypad.c segDisplay.c seeSaw.c trellis.c utils.c difficulty.c

PUBDIR = $(HOME)/cmpt433/public/myApps
OUTDIR = $(PUBDIR)
CROSS_TOOL = arm-linux-gnueabihf-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -pthread
all:
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(OUTDIR)/$(TARGET)
clean:
	rm -f $(OUTDIR)/$(TARGET)