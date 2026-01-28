/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "pid.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ADC_SAMPLES 32

static volatile uint8_t adc_counter = 0;
static volatile uint32_t adc_sum = 0;
static float target_temp = 0.0f;

// When ADC finishes, save the value
ISR(ADC_vect)
{
    adc_sum += ADC;
    adc_counter += 1;

    if (adc_counter < ADC_SAMPLES)
    {
        ADCSRA |= _BV(ADSC);
    }
    else
    {
        adc_counter = 0;
    }
}

void adc_init()
{
    ADMUX = _BV(REFS1) | _BV(REFS0); // 1.1 V reference, ADC0
    DIDR0 = _BV(ADC0D);              // Disable digital input on ADC0
    // ADC clock frequency: 125 kHz (16 MHz / 128)
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // prescaler 128
    ADCSRA |= _BV(ADEN) | _BV(ADIE);               // enable ADC and interrupt
}

void timer_init()
{
    TCCR1A |= _BV(COM1A1);

    /*Clear OC1A on Compare Match when up-counting,
    Set OC1A on Compare Match when down-counting.*/

    TCCR1B =
        _BV(WGM13) | // Phase and Frequency Correct
        _BV(CS11);   // Prescaler 8

    // For 1kHz
    ICR1 = 1000;
    OCR1A = 0;
}
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

char getchar_nonblock(void)
{
    if (UCSR0A & _BV(RXC0))
        return UDR0;
    else
        return -1;
}

// Vout = Tc·Ta+V0c
// V0c = 500mV , Tc = 10mV/C
// T = (Vout - 500mV)/(10mV)

float adc_to_temperature(uint16_t adc)
{
    // adc * 1.1mV * 1000 / 1024
    float voltage_mv = (float)adc * (1100.0f / 1024.0f);

    return (voltage_mv - 500.0f) / 10.0f;
}

uint16_t temperature_to_adc(float temperature_c)
{
    float voltage_mv = temperature_c * 10.0f + 500.0f;

    return (uint16_t)(voltage_mv * (1024.0f / 1100.0f));
}

#define LED PB1
#define LED_DDR DDRB

FILE uart_file;

// Adjusted with datasheet values
#define Kc 5000

// Albo 300
#define Pc 350

#define Kp (0.65f * Kc)

#define Ti (0.5f * Pc)

#define Td (0.12f * Pc)

int main()
{
    LED_DDR |= _BV(LED);
    adc_init();
    uart_init();
    timer_init();

    uint16_t desired_val;
    uint8_t achieved = 0;
    pidData_t pid;

    pid_Init(Kp, Ti, Td, &pid);

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    set_sleep_mode(SLEEP_MODE_IDLE);

    uint32_t count = 0;

    printf("Input target temperature: ");
    scanf("%f", &target_temp);

    desired_val = temperature_to_adc(target_temp);

    sei();

    while (1)
    {
        ADCSRA |= _BV(ADSC);
        while (adc_counter != 0)
        {
            sleep_mode();
        }

        uint32_t adc_val = adc_sum / ADC_SAMPLES;

        if (adc_val > desired_val && !achieved)
        {
            achieved = 1;
            pid_Reset_Integrator(&pid);
        }

        adc_sum = 0;
        // Disable debug information to get more accurate function
        printf("%lu  Temp: %lu\t Target: %u \t OCR1A: %u\r\n", count, adc_val, desired_val, OCR1A);

        char c = getchar_nonblock();

        if (c != -1)
        {
            OCR1A = 0;
            achieved = 0;
            pid_Reset_Integrator(&pid);
            printf("Input target temperature: ");
            scanf("%f", &target_temp);
            desired_val = temperature_to_adc(target_temp);
        }

        count += 1;

        int16_t pid_val = pid_Controller(desired_val, adc_val, &pid);
        if (pid_val < 0)
            pid_val = 0;

        OCR1A = MIN(ICR1, pid_val);

        _delay_ms(50);
    }
}