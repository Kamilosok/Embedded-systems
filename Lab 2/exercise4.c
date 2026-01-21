#include <avr/io.h>
#include <util/delay.h>

// A signal on the D port corresponds to the same segment in both displays
#define LED_DDR DDRD
#define LED_PORT PORTD

#define TRANS_DDR DDRC
#define TRANS_PORT PORTC

#define DISPLAY_ONE PC0
#define DISPLAY_TWO PC1

#define SECOND_MS 1000
// Must be a divisor of SECOND_MS / 2
#define SHOW_TIME 5

uint8_t digits[10] = {
    0b11000000, // 0
    0b01111001, // 1
    0b10100100, // 2
    0b00110000, // 3
    0b10011001, // 4
    0b00010010, // 5
    0b10000010, // 6
    0b01111000, // 7
    0b10000000, // 8
    0b00010000  // 9
};

int main()
{
    // So we can use TXD and RXD for our LED
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);

    // ALL output
    LED_DDR = 0b11111111;

    TRANS_DDR |= _BV(DISPLAY_ONE) | _BV(DISPLAY_TWO);

    // Everything off
    TRANS_PORT |= _BV(DISPLAY_ONE);
    TRANS_PORT |= _BV(DISPLAY_TWO);

    uint8_t numDec = 0, numSing = 0;
    uint8_t showCycles = SECOND_MS / (2 * SHOW_TIME);

    while (1)
    {
        for (uint8_t i = 0; i < showCycles; i++)
        {
            // First we display the singular number, turn off FIRST the decimal, turn on singular
            TRANS_PORT |= _BV(DISPLAY_ONE);
            TRANS_PORT &= ~_BV(DISPLAY_TWO);

            LED_PORT = digits[numSing];
            // Show it for 5 ms
            _delay_ms(SHOW_TIME);

            // Then we display the decimal
            TRANS_PORT |= _BV(DISPLAY_TWO);
            TRANS_PORT &= ~_BV(DISPLAY_ONE);

            LED_PORT = digits[numDec];
            // Show it for 5 ms
            _delay_ms(SHOW_TIME);
        }

        // Everything off
        TRANS_PORT |= _BV(DISPLAY_ONE);
        TRANS_PORT |= _BV(DISPLAY_TWO);

        if (numSing == 9)
        {
            if (numDec == 9)
            {
                numDec = 0;
            }
            else
                numDec += 1;

            numSing = 0;
        }
        else
            numSing += 1;
    }
}