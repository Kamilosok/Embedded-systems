/*Kamil Zdancewicz 345320*/

#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "sound.c"

extern const uint8_t dzwiek_raw[];    // deklaracja tablicy
extern const uint16_t dzwiek_raw_len;

// inicjalizacja SPI
void spi_init()
{
    // włącz odbiornik i nadajnik
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    // ustaw piny MOSI, SCK i ~SS jako wyjścia
    DDRB |= _BV(DDB3) | _BV(DDB5) | _BV(DDB2);
    PORTB |= _BV(PB2);

    // włącz SPI w trybie master z zegarem 250 kHz
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1);

    SPSR &= ~_BV(SPI2X);
}

// transfer jednego bajtu
void spi_transfer(uint8_t data)
{
    // rozpocznij transmisję
    SPDR = data;
    // czekaj na ukończenie transmisji
    while (!(SPSR & _BV(SPIF)))
        ;
    // zwróć otrzymane dane
    uint8_t dummy = SPDR;
}

ISR(TIMER1_COMPA_vect)
{
    ;
}

void timer_init()
{
    // ustaw tryb licznika
    // CTC
    TCCR1B = _BV(WGM12);
    // Prescaler 8
    TCCR1B |= _BV(CS11);

    // Compare match A int
    TIMSK1 = (_BV(OCIE1A));

    // 100 us
    OCR1A = 198;
}

void dac_send(uint8_t data)
{
    uint8_t upper = (data >> 4);

    uint8_t msb = 0b00110000 | upper;
    uint8_t lsb = data << 4;

    PORTB &= ~_BV(PB2);
    spi_transfer(msb);
    spi_transfer(lsb);
    PORTB |= _BV(PB2);
}

int main()
{
    spi_init();
    timer_init();

    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    for (uint32_t i = 0; i < dzwiek_raw_len; ++i)
    {
        uint8_t data = pgm_read_byte(&dzwiek_raw[i]);
        dac_send(data);
        sleep_mode();
    }
}
