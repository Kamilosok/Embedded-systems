/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define NUM_EVENTS 200

// 0 - polling, 1 - Noise Reduction
uint8_t mode = 0;
uint16_t event_num = 0;
static uint16_t val_pol[NUM_EVENTS];
static volatile uint16_t val_red[NUM_EVENTS];

// When ADC finishes
ISR(ADC_vect)
{
    if (mode == 0)
        ;
    else // mode == 1
    {
        val_red[event_num] = ADC;
    }
}

void adc_init()
{
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); // AVcc reference, 1.1V input
    DIDR0 = 0xFF;                                           // Disable analog input on EVERYTHING

    // ADC clock frequency: 125 kHz (16 MHz / 128)
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // prescaler 128
    ADCSRA |= _BV(ADEN) | _BV(ADIE);               // enable ADC and Interrupt
}

FILE uart_file;

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

void uart_wait()
{
    while (!(UCSR0A & _BV(TXC0)))
        ;
}

double avg()
{
    double sum = 0;
    for (uint16_t i = 0; i < NUM_EVENTS; ++i)
    {
        sum += (double)(mode == 0 ? val_pol[i] : val_red[i]);
    }

    sum /= NUM_EVENTS;

    return sum;
}

double var()
{
    double avg_val = avg();
    double sum = 0;
    for (uint16_t i = 0; i < NUM_EVENTS; ++i)
    {
        double val = (double)(mode == 0 ? val_pol[i] : val_red[i]) - avg_val;
        val *= val;
        sum += val;
    }

    sum /= NUM_EVENTS - 1;

    return sum;
}

int main()
{
    adc_init();
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    // Equivalent to setting SMCR
    set_sleep_mode(SLEEP_MODE_ADC);

    sei();
    // Adding delays in different places changes the dynamic
    while (1)
    {
        UCSR0B &= ~(_BV(RXEN0) | _BV(TXEN0)); // disable UART
        mode = 0;
        cli();
        for (uint16_t i = 0; i < NUM_EVENTS; ++i)
        {
            ADCSRA |= _BV(ADSC);
            while (!(ADCSRA & _BV(ADIF)))
                ; // Active wait

            _delay_ms(10);
            val_pol[i] = ADC;
            ADCSRA |= _BV(ADIF);
        }

        double var_pol = var();
        _delay_ms(100);

        mode = 1;
        sei();
        for (event_num = 0; event_num < NUM_EVENTS; ++event_num)
        {
            sleep_enable();
            sleep_cpu();
            sleep_disable();
            //_delay_ms(10);
        }

        double var_red = var();

        UCSR0B |= _BV(RXEN0) | _BV(TXEN0); // enable UART

        // Overall, there is no significant improvement, but this may be a problem with the board/cable/laptop,
        // Another program on this board that worked on another laptop+cable (with visible improvement) indicates that this mode is worse
        printf("Var while polling: %lf\r\nVar while reducing noise:%lf\r\n\n", var_pol, var_red);

        _delay_ms(1000);
    }
}