/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED PB2
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PA7
#define BTN_PIN PINA

ISR(TIM0_COMPA_vect)
{
    ;
}

// inicjalizacja SPI
void usi_spi_init()
{
    // In / Out, oznaczenia MOSI/MISO nie mają znaczenia, tylko DI/DO
    DDRA |= _BV(PA5) | _BV(PA4);
    DDRA &= ~_BV(PA6);

    // Pull-up na DI magicznie sprawia, że nie działa
    // PORTA |= _BV(PA6);

    // SCK i DO na stan niski
    PORTA &= ~_BV(PA4);
    PORTA &= ~_BV(PA5);

    // Three wire mode
    // 3-wire mode, clock source = Timer0 Compare Match
    USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USICLK);
}

void timer0_init()
{
    TCCR0A = _BV(WGM01); // CTC
    TCCR0B = _BV(CS00);  // preslaler 1
    TIMSK0 |= _BV(OCIE0A);

    OCR0A = 31; // 125 kHz
}

// Synchronicznie więc bez volatile na zewnątrz

uint8_t usi_spi_transfer(uint8_t data)
{
    USIDR = data;

    // Daje trochę więcej kontroli
    USISR = _BV(USIOIF);
    while (!(USISR & _BV(USIOIF)))
    {
        sleep_mode();
        USICR |= _BV(USITC);
    }

    return USIDR;
}

int main()
{
    LED_DDR |= _BV(LED);
    // SS
    LED_DDR |= _BV(PB3);

    LED_PORT |= _BV(PB3);

    PORTA |= _BV(PORTA7);

    usi_spi_init();
    timer0_init();
    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    while (1)
    {
        uint8_t out;
        if (BTN_PIN & _BV(BTN))
        {
            out = 0;
        }
        else
        {
            out = 0xFF;
        }

        // out = 0xFF;

        uint8_t data = usi_spi_transfer(out);

        if (data == 0xFF)
        {
            LED_PORT |= _BV(LED);
        }
        else
        {
            LED_PORT &= ~_BV(LED);
        }
    }
}
