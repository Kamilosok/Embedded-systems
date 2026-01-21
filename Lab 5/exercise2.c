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
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
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

void adc_init()
{
    ADMUX = _BV(REFS0);                            // referencja AVcc, wejście ADC0
    DIDR0 = _BV(ADC0D);                            // wyłącz wejście cyfrowe na ADC0
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    ADCSRA |= _BV(ADEN) | _BV(ADIE);               // włącz ADC i przerwania
}

void int_init()
{
    // ustaw pull-up na PD2 (INT0)
    PORTD |= _BV(PORTD2);
    // Falling edge
    EICRA = _BV(ISC01);
    // odmaskuj przerwania dla INT0
    EIMSK |= _BV(INT0);
}

void timer_init()
{
    // ustaw tryb licznika
    // CTC
    TCCR1B = _BV(WGM12);
    // Prescaler 128
    TCCR1B |= (_BV(CS11)) | (_BV(CS10)) | (_BV(CS00));

    // Compare match A int
    TIMSK1 = (_BV(OCIE1A));

    OCR1A = 2499;
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
        // ADC samo się włącza przy wyjściu ze sleep???
        // sleep_mode();
    }
}