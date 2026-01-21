/*Kamil Zdancewicz 345320*/

#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
    // wyczyść rejestr UCSR0A
    UCSR0A = 0;
    // włącz odbiornik i nadajnik
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    // ustaw format 8n1
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
    // czekaj aż transmiter gotowy
    while (!(UCSR0A & _BV(UDRE0)))
        ;
    UDR0 = data;
    return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
    // czekaj aż znak dostępny
    while (!(UCSR0A & _BV(RXC0)))
        ;

    return UDR0;
}

FILE uart_file;

volatile uint8_t spi_in = 0;

// inicjalizacja SPI
void spi_init()
{
    // SS, MOSI, SCK jako wejścia
    DDRB &= ~(_BV(DDB2) | _BV(DDB3) | _BV(DDB5));

    // MISO na wyjście, ale i tak nie używamy
    DDRB |= _BV(DDB4);

    // Pull-up
    PORTB |= _BV(PB2) | _BV(PB3) | _BV(PB5);

    DDRD |= _BV(PD4) | _BV(PD5) | _BV(PD6);
    // Od prawej -> SS, SCK, MOSI, MISO
    PORTD |= _BV(PD4) | _BV(PD5) | _BV(PD6) | _BV(PD7);

    // włącz SPI w trybie slave, bez zegara, interrupt
    SPCR = _BV(SPE) | _BV(SPIE);
}

ISR(SPI_STC_vect)
{
    spi_in = SPDR;

    // SPDR = spi_in + 1;
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

    // ~250Khz
    OCR1A = 31;
}

// transfer jednego bajtu
void bitbang_send(uint8_t data)
{
    // SS
    PORTD &= ~(_BV(PD4));

    for (uint8_t i = 0; i < 8; i++)
    {
        // Od najważniejszych bitów
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
