#include <avr/io.h>
#include <util/delay.h>
// DLACZEGO LED MA BOK ŚCIĘTY Z KATODY A DRABINKA LED Z ANODY
#define LEDK_DDR DDRD
#define LEDK_PORT PORTD

int main()
{
    // So we can use TXD and RXD for our LED
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);

    // ALL output
    LEDK_DDR = 0b11111111;

    // 0 - right, 1 - left
    uint8_t direction = 0;

    // Lighting only one LED at a time, states go from 0 to 0b10000000
    uint8_t state = 1, prevState = 1;

    while (1)
    {
        if (direction == 0)
        {
            state = state << 1;

            if (state == 0b10000000)
                direction = 1;
        }
        else
        {
            state = state >> 1;

            if (state == 0)
            {
                direction = 0;
                state = 1 << 1;
            }
        }

        LEDK_PORT |= state;
        _delay_ms(50);
        LEDK_PORT &= ~prevState;
        prevState = state;
        _delay_ms(50);
    }

    return 0;
}
