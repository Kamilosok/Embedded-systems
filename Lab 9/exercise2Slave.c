/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED PB1
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PD2
#define BTN_PIN PIND

volatile uint8_t out_data = 0;
volatile uint8_t in_data = 0;

void spi_slave_init()
{
    // MISO as output, the rest as input
    DDRB |= _BV(PB4);                          // MISO
    DDRB &= ~(_BV(PB3) | _BV(PB5) | _BV(PB2)); // MOSI, SCK, SS

    // SPI enable + interrupt, slave
    SPCR = _BV(SPE) | _BV(SPIE);

    SPDR = out_data;
}

void spi_slave_poll()
{
    if (SPSR & _BV(SPIF))
    {
        in_data = SPDR;
        SPDR = out_data;
    }
}

int main()
{
    // LED output
    LED_DDR |= _BV(LED);
    LED_PORT &= ~_BV(LED);

    // Button pull-up input
    DDRD &= ~_BV(BTN);
    PORTD |= _BV(BTN);

    spi_slave_init();

    while (1)
    {
        spi_slave_poll();

        if (PIND & _BV(BTN))
            out_data = 0;
        else
            out_data = 0xFF;

        if (in_data == 0xFF)
            LED_PORT |= _BV(LED);
        else
            LED_PORT &= ~_BV(LED);
    }
}