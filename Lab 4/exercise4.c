/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

#define OCR1A_VAL 210 // From fig 16-6 to get ~39.7Khz
#define PULSE_LEN 600 // in us
#define PULSE_COUNT 6
#define BREAK_LEN 100 // in ms

uint8_t gotIR = 0;

void toggle_OC1A(uint8_t connect)
{
    if (connect)
        TCCR1A |= _BV(COM1A0);
    else
        TCCR1A &= ~_BV(COM1A0);
}

void ctc_init()
{
    DDRB |= _BV(PB1);

    // WGM12 - CTC
    // CS1   = 001  -- prescaler 1
    TCCR1B = _BV(WGM12) | _BV(CS10);
    toggle_OC1A(1);
    OCR1A = OCR1A_VAL;
}

void ir_det_init()
{
    // Input
    DDRB &= ~_BV(PB0);
    PORTB |= _BV(PB0); // Pull-up
}

void send_pulse()
{
    for (uint8_t i = 0; i < PULSE_COUNT; ++i)
    {
        DDRB |= _BV(PB1);
        _delay_us(PULSE_LEN);

        if (!(PINB & (_BV(PB0))))
        {
            gotIR = 1;
        }

        DDRB &= ~_BV(PB1);
        _delay_us(PULSE_LEN);
    }
}

int main()
{
    ctc_init();
    ir_det_init();

    DDRB |= _BV(PB5);

    while (1)
    {
        send_pulse();

        // Check input (0 = detected)
        if (gotIR)
        {
            PORTB |= _BV(PB5);
        }
        else
        {
            PORTB &= ~_BV(PB5);
        }

        gotIR = 0;

        _delay_ms(BREAK_LEN);
    }
}