/*Kamil Zdancewicz 345320*/

#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

#define BAUD 9600                              // Baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // From datasheet

void uart_init()
{
    UBRR0 = UBRR_VALUE;
    UCSR0A = 0;
    // Enable receiver and transmitter
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    // 8n1 format
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// Transmit one character
int uart_transmit(char data, FILE *stream)
{
    // Wait until ready
    while (!(UCSR0A & _BV(UDRE0)))
        ;
    UDR0 = data;
    return 0;
}

// Receive one character
int uart_receive(FILE *stream)
{
    // Wait until ready
    while (!(UCSR0A & _BV(RXC0)))
        ;
    return UDR0;
}

FILE uart_file;

volatile uint8_t spi_in = 0;

// inicjalizacja SPI
void spi_init()
{
    // SS, MOSI, SCK as output
    DDRB &= ~(_BV(DDB2) | _BV(DDB3) | _BV(DDB5));

    // MISO output for future reference, even if we aren't using it now
    DDRB |= _BV(DDB4);

    // Pull-up
    PORTB |= _BV(PB2) | _BV(PB3) | _BV(PB5);

    DDRD |= _BV(PD4) | _BV(PD5) | _BV(PD6);
    // From the right -> SS, SCK, MOSI, MISO
    PORTD |= _BV(PD4) | _BV(PD5) | _BV(PD6) | _BV(PD7);

    // Enable SPI in slave mode, without clock, interrupt
    SPCR = _BV(SPE) | _BV(SPIE);
}

ISR(SPI_STC_vect)
{
    spi_in = SPDR;
}

ISR(TIMER1_COMPA_vect)
{
    ;
}

void timer_init()
{
    // CTC
    TCCR1B = _BV(WGM12);
    // Prescaler 8
    TCCR1B |= _BV(CS11);

    // Compare match A int
    TIMSK1 = (_BV(OCIE1A));

    // ~250Khz
    OCR1A = 31;
}

// Transfer one byte
void bitbang_send(uint8_t data)
{
    // SS
    PORTD &= ~(_BV(PD4));

    for (uint8_t i = 0; i < 8; i++)
    {
        // From the most significant bits
        if (data & 0x80)
        {
            PORTD |= _BV(PD6);
        }
        else
        {
            PORTD &= ~_BV(PD6);
        }

        // printf("MOSI %hu, SCK %hu, SS %hu\r\n", (PINB & _BV(PB3)) != 0, (PINB & _BV(PB5)) != 0, (PINB & _BV(PB2)) != 0);

        // SCK
        PORTD |= _BV(PD5);
        sleep_mode();

        // printf("MOSI %hu, SCK %hu, SS %hu\r\n\n", (PINB & _BV(PB3)) != 0, (PINB & _BV(PB5)) != 0, (PINB & _BV(PB2)) != 0);

        PORTD &= ~_BV(PD5);
        sleep_mode();

        data <<= 1;
    }

    PORTD |= (_BV(PD4));
}

int main()
{
    spi_init();
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    timer_init();

    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    for (uint8_t i = 0; i < 100; i++)
    {
        printf("Sending %u\r\n", i);
        bitbang_send(i);

        printf("Gotten %u\r\n", spi_in);

        _delay_ms(1000);
    }
}
