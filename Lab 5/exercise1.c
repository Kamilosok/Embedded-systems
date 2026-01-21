/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PD2
#define BTN_PIN PIND

#define BUF_SIZE 128
#define SAMPLE_LEN 10

static volatile uint8_t history[BUF_SIZE];

// The first 1s is blank
static volatile uint8_t read_id = 0;
static volatile uint8_t write_id = 100;
/*
Działało gorzej ze sprawdzaniem naciśnięcia w ten sposób
static volatile uint8_t curr_state = 0;
static volatile uint8_t pressed_this_cycle = 0;

ISR(INT0_vect)
{
    if (!pressed_this_cycle)
    {
        pressed_this_cycle = 1;
        if (BTN_PIN & _BV(BTN))
        {
            curr_state = 0;
        }
        else
        {
            curr_state = 1;
        }
    }
}
*/

void next_cycle()
{
    // To not use modulo, we waste some memory (28 bytes) to return to the initial index of history faster
    // cli();
    read_id = (read_id + 1) & (BUF_SIZE - 1);
    write_id = (write_id + 1) & (BUF_SIZE - 1);
    // sei();
}

ISR(TIMER1_COMPA_vect)
{
    if (BTN_PIN & _BV(BTN))
        history[write_id] = 0;
    else
        history[write_id] = 1;

    if (history[read_id])
        LED_PORT |= _BV(LED);
    else
        LED_PORT &= ~_BV(LED);

    // pressed_this_cycle = 0;
    next_cycle();
}
/*
void int_init()
{
    // ustaw pull-up na PD2 (INT0)
    PORTD |= _BV(PORTD2);
    // Any logical change
    EICRA |= _BV(ISC00);
    // odmaskuj przerwania dla INT0
    EIMSK |= _BV(INT0);
}
*/

void timer_init()
{
    // ustaw tryb licznika
    // CTC
    TCCR1B = _BV(WGM12);
    // Prescaler 64
    TCCR1B |= (_BV(CS11)) | (_BV(CS10));

    // Compare match A int
    TIMSK1 = (_BV(OCIE1A));

    // Z wzoru datasheetowego
    OCR1A = 2499;
}

int main()
{

    LED_DDR |= _BV(LED);
    // ustaw pull-up na PD2 (INT0)
    PORTD |= _BV(PORTD2);
    // int_init();

    timer_init();
    set_sleep_mode(SLEEP_MODE_IDLE);
    //  odmaskuj przerwania
    sei();

    while (1)
    {
        sleep_mode();
    }
}