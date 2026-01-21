#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB

#define BUF_SIZE 128
#define SAMPLE_LEN 10

uint8_t history[BUF_SIZE];

// The first 1s is blank
uint8_t read_id = 0;
uint8_t write_id = 100;

void next_cycle()
{
    // To not use modulo, we waste some memory (28 bytes) to return to the initial index of history faster
    read_id = (read_id + 1) & (BUF_SIZE - 1);
    write_id = (write_id + 1) & (BUF_SIZE - 1);
}

int main()
{
    // Port is input as default
    BTN_PORT |= _BV(BTN);

    LED_DDR |= _BV(LED);

    while (1)
    {
        // If button is not pressed at the start of the cycle
        if (BTN_PIN & _BV(BTN))
            history[write_id] = 0;
        else
            history[write_id] = 1;

        if (history[read_id])
            LED_PORT |= _BV(LED);
        else
            LED_PORT &= ~_BV(LED);

        next_cycle();
        _delay_ms(SAMPLE_LEN);
    }
}