#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

#define NUM_5V 1024UL
#define V_REF_INTERNAL 1100UL

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

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

    LED_DDR |= _BV(LED);
    // zainicjalizuj ADC
    adc_init();
    while (1)
    {

        // LED_PORT |= _BV(LED); // Turn on LED before converting

        ADCSRA |= _BV(ADSC); // wykonaj konwersję

        while (!(ADCSRA & _BV(ADIF)))
            ; // czekaj na wynik

        // LED_PORT &= ~_BV(LED); // Turn off LED after converting

        ADCSRA |= _BV(ADIF);                                // wyczyść bit ADIF (pisząc 1!)
        uint32_t v = ADC;                                   // weź zmierzoną wartość (0..1023) ADLAR 0 bo nieustawiony nigdzie
        uint64_t voltage_vcc = V_REF_INTERNAL * NUM_5V / v; // in millivolts

        printf("Vcc: %.4f V (ADC=%lu)\r\n", (float)((double)voltage_vcc / 1000.0), v);
    }

    // Z led bardziej stabilny?
}
