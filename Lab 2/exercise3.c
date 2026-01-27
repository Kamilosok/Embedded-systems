/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

#define RESET_BTN PB0
#define PREV_BTN PB1
#define NEXT_BTN PB2
#define BTN_PIN PINB
#define BTN_PORT PORTB

#define LED_DDR DDRD
#define LED_PORT PORTD

uint8_t resetPressed = 0, prevPressed = 0, nextPressed = 0;

uint8_t numToGray(uint8_t n)
{
    return n ^ (n >> 1);
}

int main()
{
    // Port is input as default, debouncing here
    DDRB &= ~(_BV(RESET_BTN) | _BV(PREV_BTN) | _BV(NEXT_BTN));
    BTN_PORT |= _BV(RESET_BTN) | _BV(PREV_BTN) | _BV(NEXT_BTN);

    // So we can use TXD and RXD for our LED
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);

    // ALL output
    LED_DDR = 0b11111111;

    uint8_t currN = 0;

    while (1)
    {
        // Reset blocks inputs
        if (!resetPressed)
        {
            if (BTN_PIN & _BV(RESET_BTN))
            {
                ;
            }
            else
            {
                currN = 0;
                resetPressed = 1;

                LED_PORT = numToGray(currN);
            }

            if (!prevPressed)
            {
                if (BTN_PIN & _BV(PREV_BTN))
                {
                    ;
                }
                else
                {
                    currN -= 1;
                    prevPressed = 1;
                    LED_PORT = numToGray(currN);
                }
            }
            else if (BTN_PIN & _BV(PREV_BTN))
                prevPressed = 0;

            if (!nextPressed)
            {
                if (BTN_PIN & _BV(NEXT_BTN))
                {
                    ;
                }
                else
                {
                    currN += 1;
                    nextPressed = 1;
                    LED_PORT = numToGray(currN);
                }
            }
            else if (BTN_PIN & _BV(NEXT_BTN))
                nextPressed = 0;
        }
        else if (BTN_PIN & _BV(RESET_BTN))
        {
            resetPressed = 0;
        }

        // If we press quickly, then it shifts by one place, however during slow presses it can register multiple presses otherwise
        _delay_ms(10);
    }
}