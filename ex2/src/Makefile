CC=avr32-gcc
CFLAGS=-O2 -g -std=gnu99 -pedantic -Wall -Wextra -Winline -Werror -I../include 
LDFLAGS=-g -Wall -L../lib -lavr32b-interrupts -lm

BASH=/bin/bash

AVR32PROGRAM=avr32program

TARGET=ex2.elf
MELODIES_RENDERED=melodies__rendered.c

OBJS=main.o switches.o utils.o leds.o synthesizer.o \
	 melodies.o dac.o controller.o



.PHONY: all
all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)


melodies.o: melodies.c $(MELODIES_RENDERED)
	$(CC) $(CFLAGS) $< -c -o $@ 


$(MELODIES_RENDERED): ../melodies/*.py ../melodies/*.bash
	cd ../melodies/ && $(BASH) ./render_all.bash > ../src/$(MELODIES_RENDERED)


.PHONY: upload
upload: $(TARGET)
	$(AVR32PROGRAM) halt
	sleep 1
	$(AVR32PROGRAM) program -e -f0,8Mb $<


.PHONY: lint
lint:
	cppcheck -v --enable=all --std=c99 .


.PHONY: clean
clean:
	find . | egrep "\.(~|o|s|elf)" | xargs rm -f
	rm -f tags $(TARGET) $(MELODIES_RENDERED) 
