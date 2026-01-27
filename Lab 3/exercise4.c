/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <util/delay.h>

#define R_REF 4400
#define NUM_5V 1024
#define B 4000
#define ZERO_C 273.15f

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

// ADC initialization
void adc_init()
{
    ADMUX = _BV(REFS0); // AVcc reference, ADC0 input
    DIDR0 = _BV(ADC0D); // Disable analog input on ADC0
    // ADC clock frequency: 125 kHz (16 MHz / 128)
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // prescaler 128
    ADCSRA |= _BV(ADEN);                           // enable ADC
}

uint32_t adc_measure()
{
    ADCSRA |= _BV(ADSC); // Convert and poll
    while (!(ADCSRA & _BV(ADIF)))
        ;
    ADCSRA |= _BV(ADIF); // Erase ADIF (by writing 1!)
    uint32_t v = ADC;    // Get value [0,1023]

    return v;
}

FILE uart_file;

int main()
{
    uart_init();

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    adc_init();

    // From law of voltage division R_ntc = R_ref * V_adc / (V_cc - V_adc)
    while (1)
    {
        uint32_t v = adc_measure();
        uint32_t r_ntc = (R_REF * v) / (NUM_5V - v);
        printf("Resistance: %lu\r\n", r_ntc);
        // I assume B = 4000K, converting from the equation in the exercise list
        // At 25C, R0 4700 B 4000
        float temp = 1.0f / ((1.0f / 298.15f) + (1.0f / B) * log(r_ntc / 4700.0f));
        printf("Temp in C: %f\r\n", temp - ZERO_C);
        _delay_ms(1000);
    }
}
