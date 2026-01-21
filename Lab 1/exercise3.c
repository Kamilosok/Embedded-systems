#include <avr/io.h>
#include <util/delay.h>
// Chyba brakuje jednego rezystora 220
#define LED_DDR DDRD
#define LED_PORT PORTD

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

    uint8_t num = 0;

    while (1)
    {
        LED_PORT = digits[num];
        _delay_ms(1000);

        if (num == 9)
            num = 0;
        else
            num += 1;
    }
}

// Po tym ze zdjęcia zmienia się na USB(X+1)