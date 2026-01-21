/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define PRESCALER 64
#define CPU_F 16000000UL
#define MAX_COUNTER 65536UL

static volatile uint16_t ovf_count = 0, prev_ovf = 0;
static volatile uint16_t prev_capture = 0;
static volatile uint8_t new = 0;
static volatile double freq = 0;

// Overflow int
ISR(TIMER1_OVF_vect)
{
    ovf_count++;
}

ISR(TIMER1_CAPT_vect)
{
    uint16_t curr_capture = ICR1;
    uint32_t ticks;

    if (ovf_count == prev_ovf)
    {
        ticks = curr_capture - prev_capture;
    }
    else // Only for ovf_count > prev_ovf
    {
        if (curr_capture >= prev_capture)
        {
            ticks = (uint32_t)(ovf_count - prev_ovf) * MAX_COUNTER + (uint32_t)(curr_capture - prev_capture);
        }
        else
        {
            ticks = (uint32_t)(ovf_count - prev_ovf - 1) * MAX_COUNTER + (uint32_t)(MAX_COUNTER - prev_capture + curr_capture);
        }
    }

    prev_capture = curr_capture;
    prev_ovf = ovf_count;

    freq = (double)CPU_F / ((double)PRESCALER * ticks);
    new = 1;

    // Logika w ISR, a printf w pętli głównej?
    // printf("Frequency on ICP1: %.2lfHz\r\n", freq);
}

void timer_init()
{

    TCCR1A = 0;
    // Rising edge
    TCCR1B = _BV(ICES1);
    // Prescaler 128
    TCCR1B |= _BV(CS11) | _BV(CS10);

    // Input capture int
    TIMSK1 = _BV(ICIE1) | _BV(TOIE1);
}

FILE uart_file;

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
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

void uart_wait()
{
    while (!(UCSR0A & _BV(TXC0)))
        ;
}

int main()
{
    uart_init();
    timer_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    while (1)
    {
        if (new)
        {
            new = 0;
            printf("Frequency on ICP1: %.2lfHz\r\n", freq);
        }
        sleep_mode();
    }
}