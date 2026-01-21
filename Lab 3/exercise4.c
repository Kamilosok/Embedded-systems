#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <util/delay.h>

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

#define R_REF 4400
#define NUM_5V 1024
#define B 4000
#define ZERO_C 273.15f

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

// inicjalizacja ADC
void adc_init()
{
    ADMUX = _BV(REFS0); // referencja AVcc, wejście ADC0
    DIDR0 = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    ADCSRA |= _BV(ADEN);                           // włącz ADC
}

uint32_t adc_measure()
{
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
    while (!(ADCSRA & _BV(ADIF)))
        ;
    ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
    uint32_t v = ADC;    // weź zmierzoną wartość (0..1023)

    return v;
}

FILE uart_file;

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj ADC
    adc_init();
    // mierz napięcie

    // Z prawa dzielnika napięcia R_ntc = R_ref * V_adc / (V_cc - V_adc)
    while (1)
    {
        uint32_t v = adc_measure();
        uint32_t r_ntc = (R_REF * v) / (NUM_5V - v);
        printf("Resistance: %lu\r\n", r_ntc);
        // Nie mam termometra w domu, więc uznam B = 4000K, przekształcenie wzoru z listy
        // W 25C, R0 4700 B 4000
        float temp = 1.0f / ((1.0f / 298.15f) + (1.0f / B) * log(r_ntc / 4700.0f));
        printf("Temp in C: %f\r\n", temp - ZERO_C);
        _delay_ms(1000);
    }
}
