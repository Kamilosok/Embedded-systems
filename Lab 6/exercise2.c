/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <stdio.h>
FILE uart_file;

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem
// Must be a power of 2
#define BUF_SIZE 64

static volatile uint8_t curr_out = 0;
static volatile uint8_t to_write = 0;
static volatile char out_buf[BUF_SIZE];

static volatile uint8_t curr_in = 0;
static volatile uint8_t to_receive = 0;
static volatile uint8_t new_received = 0;
static volatile char in_buf[BUF_SIZE];

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

ISR(USART_UDRE_vect)
{
    if (to_write > 0)
    {
        UDR0 = out_buf[curr_out];
        curr_out = BUF_MASK(curr_out + 1);
        to_write -= 1;
    }
    else
    {
        UCSR0B &= ~_BV(UDRIE0);
    }
}

// transmisja jednego znaku
int uart_transmit(char data)
{
    // Active wait
    while (to_write > BUF_SIZE)
    {
        ;
    }

    cli();

    out_buf[BUF_MASK(curr_out + to_write)] = data;
    to_write += 1;

    // Enable sending
    UCSR0B |= _BV(UDRIE0);

    sei();

    return 0;
}

ISR(USART_RX_vect)
{
    if (to_receive > 0 && new_received < BUF_SIZE)
    {
        in_buf[BUF_MASK(curr_in + new_received)] = UDR0;
        to_receive -= 1;
        new_received += 1;
    }
    else
    {
        // To ignore
        volatile char dummy = UDR0;
    }
}

// odczyt jednego znaku
int uart_receive(char *data)
{
    cli();
    to_receive += 1;
    sei();
    // Active wait
    while (new_received == 0)
    {
        ;
    }

    cli();

    *data = in_buf[BUF_MASK(curr_in)];
    curr_in = BUF_MASK(curr_in + 1);
    new_received -= 1;

    sei();

    return 0;
}

int main()
{
    // zainicjalizuj UART
    uart_init();
    // fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    // stdin = stdout = stderr = &uart_file;

    set_sleep_mode(SLEEP_MODE_IDLE);
    sei();

    uart_transmit('W');
    uart_transmit('e');
    uart_transmit('l');
    uart_transmit('c');
    uart_transmit('o');
    uart_transmit('m');
    uart_transmit('e');
    uart_transmit(':');
    uart_transmit(' ');

    while (1)
    {
        char x = 'a';
        uart_receive(&x);

        uart_transmit(x);
    }
}
