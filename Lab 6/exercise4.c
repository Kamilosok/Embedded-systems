/*Kamil Zdancewicz 345320*/

#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

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

// inicjalizacja SPI
void spi_init()
{
    // Set MOSI, SCK, ~SS and PB1 as output
    DDRB |= _BV(DDB3) | _BV(DDB5) | _BV(DDB2) | _BV(DDB1);

    // LEDS ON
    PORTB &= ~_BV(PB2);

    PORTB &= ~_BV(PB1);

    // Enable SPI in master mode with 250 kHz clock
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1);

    SPSR &= ~_BV(SPI2X);
}

// Transfer one byte
void spi_transfer(uint8_t data)
{
    // Start the transmission
    SPDR = data;
    // Active wait till the end of transmission
    while (!(SPSR & _BV(SPIF)))
        ;
    // Return received data
    uint8_t dummy = SPDR;
}

ISR(TIMER1_COMPA_vect)
{
    ;
}

void timer_init()
{
    // CTC
    TCCR1B = _BV(WGM12);
    // Prescaler 128
    TCCR1B |= _BV(CS12) | _BV(CS10);

    // Compare match A int
    TIMSK1 = (_BV(OCIE1A));

    // 1s
    OCR1A = 15624;
}

void led_send(uint8_t number)
{
    uint8_t data = digits[number];

    spi_transfer(data);
    PORTB |= _BV(PB1);
    PORTB &= ~_BV(PB1);
}

int main()
{
    spi_init();
    timer_init();

    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    uint8_t number = 0;

    while (1)
    {
        led_send(number);
        number = (number + 1) % 10;
        sleep_mode();
    }
}
