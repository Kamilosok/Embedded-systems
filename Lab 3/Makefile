TARGET     = exercise1
MCU        = atmega328p
F_CPU      = 16000000
PORT       = /dev/ttyUSB0
BAUD       = 115200
OPTS	   = -O3# or -Os
LIBS       = 
DEFS       = 

CC         = avr-gcc
OBJCOPY    = avr-objcopy
AVRDUDE    = avrdude
SCREEN     = screen

# Compile flags
override CFLAGS = -g -std=c99 -DF_CPU=$(F_CPU) -Wall $(OPTS) -mmcu=$(MCU)
# Link flags, uncomment the second line if you wish UART to handle floats
override LDFLAGS = -Wl,-Map,$(TARGET).map 
#LDFLAGS += -Wl,-u,vfprintf -Wl,-u,vfscanf -lprintf_flt -lscanf_flt -lm

# Tasks
all: $(TARGET).hex

# Compile and link
$(TARGET).elf: $(TARGET).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Install
install: $(TARGET).hex
	$(AVRDUDE) -p $(MCU) -c arduino -P $(PORT) -b $(BAUD) -v -U flash:w:$(TARGET).hex

# For use with UART
screen:
	$(SCREEN) $(PORT)

# Clean
clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).map