/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                              // Baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // From datasheet

#define NUM_5V 1024UL
#define V_REF_INTERNAL 1100UL

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

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
    DIDR0 = _BV(ADC0D); // Disable digital input on ADC0
    // ADC clock frequency: 125 kHz (16 MHz / 128)
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // prescaler 28
    ADCSRA |= _BV(ADEN);                           // enable ADC
}

FILE uart_file;

int main()
{
    uart_init();

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    LED_DDR |= _BV(LED);
    adc_init();
    while (1)
    {

        // LED_PORT |= _BV(LED); // Possibly turn on LED before converting

        ADCSRA |= _BV(ADSC); //

        while (!(ADCSRA & _BV(ADIF)))
            ; // Poll

        // LED_PORT &= ~_BV(LED); // Possibly turn off LED after converting

        ADCSRA |= _BV(ADIF);                                // Erase ADIF (by writing 1!)
        uint32_t v = ADC;                                   // Get the value
        uint64_t voltage_vcc = V_REF_INTERNAL * NUM_5V / v; // In millivolts

        printf("Vcc: %.4f V (ADC=%lu)\r\n", (float)((double)voltage_vcc / 1000.0), v);
    }

    // More stable with LED?
}
