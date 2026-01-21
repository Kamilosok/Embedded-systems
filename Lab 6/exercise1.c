/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

// Must be a power of 2
#define BUF_SIZE 64
static volatile uint8_t curr_out = 0;
static volatile uint8_t to_write = 0;
static volatile char out_buf[BUF_SIZE];

#define BUF_MASK(x) ((x) & (BUF_SIZE - 1))

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

    // włącz przerwania

    UCSR0B |= _BV(RXCIE0);
    // UCSR0B |= _BV(TXCIE0);
    UCSR0B |= _BV(UDRIE0);
}

ISR(USART_RX_vect)
{
    char c = UDR0;
    if (to_write < BUF_SIZE)
    {
        out_buf[BUF_MASK(curr_out + to_write)] = c;
        to_write += 1;

        // Enable sending
        UCSR0B |= _BV(UDRIE0);
    }

    // Else we can't store more data in the buffer, so ignore it instead of overwriting
}

ISR(USART_UDRE_vect)
{
    if (to_write > 0)
    {
        uint8_t crlf = 0;
        if (out_buf[curr_out] == '\r')
            crlf = 1;

        UDR0 = out_buf[curr_out];
        if (!crlf)
        {
            curr_out = BUF_MASK(curr_out + 1);
            to_write -= 1;
        }
        else
            out_buf[curr_out] = '\n';
    }
    else
    {
        UCSR0B &= ~_BV(UDRIE0);
    }
}

int main()
{
    // zainicjalizuj UART
    uart_init();

    set_sleep_mode(SLEEP_MODE_IDLE);
    sei();
    while (1)
    {
        sleep_mode();
    }
}
