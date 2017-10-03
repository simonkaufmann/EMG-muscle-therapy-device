CC = gcc
LD = ld

LDFLAGS = `pkg-config --libs gtk+-2.0`
CFLAGS = `pkg-config --cflags gtk+-2.0`

SRC = window_main.c window_settings.c window_reference_signal.c window_signal.c window_difficulty.c window_calibration.c window_music.c window_shutdown.c
TARGET = main

all: main

%.o: %.c
	$(CC) $(CFLAGS) -c $<

main: $(SRC:%.c=%.o) main.c
	gcc $(CFLAGS) -o $(TARGET) $^ $(LDFLAGS)

cursor:
	DISPLAY=:0 /home/pi/blank_cursor

run: main cursor
	DISPLAY=:0 ./main

clean: 
	rm -f main $(SRC:%.c=%.o) 

