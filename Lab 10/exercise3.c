/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

static volatile uint32_t last_adc = 0;
static volatile float target_temp = 0.0f;

// Vout = Tc·Ta+V0c
// V0c = 500mV , Tc = 10mV/C
// T = (Vout - 500mV)/(10mV)

// When ADC finishes, save the value
ISR(ADC_vect)
{
    last_adc = ADC;
}

void adc_init()
{
    ADMUX = _BV(REFS1) | _BV(REFS0);               // referencja 1.1 V, ADC0
    DIDR0 = _BV(ADC0D);                            // wyłącz wejście cyfrowe na ADC0
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    ADCSRA |= _BV(ADEN) | _BV(ADIE);               // włącz ADC i przerwania
}

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

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

void uart_wait()
{
    while (!(UCSR0A & _BV(UDRE0)))
        ;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
    // czekaj aż znak dostępny
    while (!(UCSR0A & _BV(RXC0)))
        ;
    return UDR0;
}

float adc_to_temperature_c(uint16_t adc)
{
    // adc * 1.1mV * 1000 / 1024
    float voltage_mv = (float)adc * 1100.0f / 1024.0f;

    // Z wzoru na górze
    return (voltage_mv - 500.0f) / 10.0f;
}

#define Th (1.0f)

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

FILE uart_file;

int main()
{
    LED_DDR |= _BV(LED);
    adc_init();
    uart_init();

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    set_sleep_mode(SLEEP_MODE_IDLE);
    uint8_t achieved = 0;

    printf("Input target temperature: ");
    scanf("%f", &target_temp);

    sei();
    ADCSRA |= _BV(ADSC);
    _delay_ms(1);

    while (1)
    {
        float temp = adc_to_temperature_c(last_adc);
        printf("Temp: %.2f C\t Target: %.2f C\r\n", temp, target_temp - (achieved ? Th : 0));

        if (temp > target_temp && !achieved)
        {
            achieved = 1;
            // Turn off the heat
            LED_PORT &= ~_BV(LED);
        }

        if (achieved && temp < target_temp - Th)
        {
            achieved = 0;
            printf("Input new target temperature: ");
            scanf("%f", &target_temp);
        }

        if (!achieved)
        {
            // Turn on the heat
            LED_PORT |= _BV(LED);
        }

        uart_wait(); // poczekaj na UART
        ADCSRA |= _BV(ADSC);
        _delay_ms(500);
    }
}