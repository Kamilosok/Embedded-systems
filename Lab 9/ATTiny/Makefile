TARGET     = exercise1
MCU        = attiny84
F_CPU      = 1000000
PORT       = /dev/ttyUSB0
BAUD       = 19200
OPTS       = -g -std=c99 -Wall -Os
LIBS       = 

# Configuration for ATMega328P as programmer
ISP_TARGET = atmega328p
ISP_PROG   = arduino
ISP_BAUD   = 115200

SRCS       = $(wildcard *.c)
OBJS       = $(SRCS:.c=.o)
EXTRA_OBJS = plik.o

CC         = avr-gcc
OBJCOPY    = avr-objcopy
AVRDUDE    = avrdude
SCREEN     = screen

# Compile flags
override CFLAGS = -DF_CPU=$(F_CPU) -mmcu=$(MCU) $(OPTS) -I.
# Link flags
override LDFLAGS = -Wl,-Map,$(TARGET).map 

# Tasks
all: $(TARGET).hex

# Compile and link
$(TARGET).elf: $(OBJS) $(EXTRA_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Install (ATtiny84)
install: $(TARGET).hex
	$(AVRDUDE) -p $(MCU) -c stk500v1 -P $(PORT) -b $(BAUD) -v -U flash:w:$(TARGET).hex

# Install the programmer on ATMega328P
arduinoisp:
	$(AVRDUDE) -p $(ISP_TARGET) -c $(ISP_PROG) -P $(PORT) -b $(ISP_BAUD) -v -U flash:w:arduinoisp.hex

# For use with UART
screen:
	$(SCREEN) $(PORT) $(BAUD)

# Clean
clean:
	rm -f *.o $(TARGET).elf $(TARGET).hex $(TARGET).map