/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <util/delay.h>

static volatile uint32_t last_adc = 0;
static volatile uint32_t last_res = 0;

#define R_CONST 2200UL
#define NUM_5V 1024UL

uint32_t calc_resistance(uint32_t adc_val)
{
    return (R_CONST * adc_val) / (NUM_5V - adc_val);
}

ISR(INT0_vect)
{
    ADCSRA |= _BV(ADSC); // Convert
}

ISR(TIMER1_COMPA_vect)
{
    ;
}

// When ADC finishes, save the value
ISR(ADC_vect)
{
    last_adc = ADC;
    last_res = calc_resistance(last_adc);
}

// ADC initialization
void adc_init()
{
    ADMUX = _BV(REFS0); // AVcc reference, ADC0 input
    DIDR0 = _BV(ADC0D); // Disable analog input on ADC0
    // ADC clock frequency: 125 kHz (16 MHz / 128)
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // prescaler 128
    ADCSRA |= _BV(ADEN) | _BV(ADIE);               // enable ADC and Interrupt
}

void int_init()
{
    // Pull-up
    PORTD |= _BV(PORTD2);
    // Falling edge
    EICRA = _BV(ISC01);
    // Interrupt on INT0
    EIMSK |= _BV(INT0);
}

void timer_init()
{
    // CTC
    TCCR1B = _BV(WGM12);
    // Prescaler 128
    TCCR1B |= (_BV(CS11)) | (_BV(CS10)) | (_BV(CS00));

    // Compare match A int
    TIMSK1 = (_BV(OCIE1A));

    OCR1A = 2499;
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

int main()
{

    adc_init();
    uart_init();
    timer_init();
    int_init();

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    while (1)
    {
        printf("RESITANCE: %lu\r\n", last_res);
        uart_wait(); // poczekaj na UART
        _delay_ms(100);
        // Apparently ADC tunrs itself on when waking up from sleep
        // sleep_mode();
    }
}