/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED PB2
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PA7
#define BTN_PIN PINA

#define BUF_SIZE 128
#define SAMPLE_LEN 10

static volatile uint8_t history[BUF_SIZE];

// The first 1s is blank
static volatile uint8_t read_id = 0;
static volatile uint8_t write_id = 100;
int a = 0;

void next_cycle()
{
    // To not use modulo, we waste some memory (28 bytes) to return to the initial index of history faster
    read_id = (read_id + 1) & (BUF_SIZE - 1);
    write_id = (write_id + 1) & (BUF_SIZE - 1);
}

ISR(TIM0_COMPA_vect)
{
    if (BTN_PIN & _BV(BTN))
        history[write_id] = 0;
    else
        history[write_id] = 1;

    if (history[read_id])
        LED_PORT |= _BV(LED);
    else
        LED_PORT &= ~_BV(LED);

    next_cycle();
}

void timer_init()
{
    // CTC
    TCCR0A = _BV(WGM01);
    TCCR0B = 0;

    // Preskaler 64
    TCCR0B |= _BV(CS01) | _BV(CS00);

    // Interrupt
    TIMSK0 |= _BV(OCIE0A);

    // ~100 Hz
    OCR0A = 155;
}

int main()
{

    LED_DDR |= _BV(LED);

    // Pull-up chyba nie istnieje
    PORTA |= _BV(PORTA7);

    timer_init();
    set_sleep_mode(SLEEP_MODE_IDLE);
    //   odmaskuj przerwania
    sei();

    while (1)
    {
        sleep_mode();
    }
}