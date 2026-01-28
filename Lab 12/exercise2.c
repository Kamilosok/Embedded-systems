/*Kamil Zdancewicz 345320*/

/*
FIXME:
This doesn't work fully for the exercise, the pid_Reset_Integrator() should be called less often,
but I wasn't able to get it working better before the deadline
*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "pid.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static volatile uint16_t adc_speed = 0;
static volatile uint8_t adc_mode = 0; // 1 - speed, 2 - potentiometer
static volatile uint8_t adc_in_progress = 0;

static volatile uint8_t when_change = 0;

volatile uint16_t desired_val = 0;
uint16_t pid_want = 0;
volatile pidData_t pid;

static inline uint16_t adc_to_mv(uint16_t adc)
{
    // 5000 mV * adc / 1023
    return adc * 5000UL / 1023UL;
}

#define PWM_STEP 10
#define PWM_MIN 10

// When ADC finishes, save the value
ISR(ADC_vect)
{
    switch (adc_mode)
    {
    case 1:
        adc_speed = ADC;
        if (when_change == 1)
        {
            uint16_t speed = 5000 - adc_to_mv(adc_speed);
            int16_t pid_val = pid_Controller(desired_val, speed, &pid);

            // Maybe change this?
            if (pid_val < 0)
                pid_val = 0;

            if (pid_val >= ICR1)
                pid_Reset_Integrator(&pid);

            static int16_t pwm_out = 0;

            if (pid_val > pwm_out + PWM_STEP)
                pwm_out += PWM_STEP;
            else if (pid_val < pwm_out - PWM_STEP)
                pwm_out -= PWM_STEP;
            else
                pwm_out = pid_val;

            if (pwm_out > 0 && pwm_out < PWM_MIN)
                pwm_out = PWM_MIN;

            pid_want = pwm_out;

            OCR1A = MIN(ICR1, pwm_out);

            when_change = 0;
        }
        else
        {
            when_change += 1;
            pid_Reset_Integrator(&pid);
        }

        break;
    case 2:
        uint16_t temp = ADC;

        desired_val = temp;
        break;

    default:
        break;
    }

    adc_in_progress = 0;
}

// Middle of MOSFET OFF is when reaches top
ISR(TIMER1_CAPT_vect)
{
    if (!adc_in_progress)
    {
        adc_mode = 1;
        adc_in_progress = 1;
        ADCSRA |= _BV(ADSC);
    }
}

void adc_init()
{
    ADMUX = _BV(REFS0);                            // AVcc reference, ADC0
    DIDR0 = _BV(ADC0D) | _BV(ADC1D);               // Disable digital input on ADC0
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // prescaler 128
    ADCSRA |= _BV(ADEN) | _BV(ADIE);               // enable ADC and interrupt
}

#define MOTOR PB1
#define MOTOR_DDR DDRB

void timer_init()
{
    MOTOR_DDR |= _BV(MOTOR);

    TCCR1A |= _BV(COM1A1);

    /*Clear OC1A on Compare Match when up-counting,
    Set OC1A on Compare Match when down-counting.*/

    TCCR1B =
        _BV(WGM13) | // Phase and Frequency Correct
        _BV(CS11);   // Prescaler 8

    // Interrupts
    TIMSK1 |= _BV(ICIE1);

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
void uart_wait()
{
    while (!(UCSR0A & _BV(UDRE0)))
        ;
}

FILE uart_file;

static inline void change_adc_input(uint8_t input)
{
    if (input)
        input = 1;

    ADMUX = (ADMUX & 0xF0) | input;
}

// Adjusted with experiments
#define Kc 28

#define Pc 25

#define Kp (0.65f * Kc)

#define Ti (0.12f * Pc)

#define Td (0.5f * Pc)

int main()
{

    adc_init();
    timer_init();
    uart_init();

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    pid_Init(Kp, Ti, Td, &pid);

    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    while (1)
    {
        // Can't print from volatile
        uint16_t speed;
        uint16_t adc_pot;
        cli();
        speed = 5000 - adc_to_mv(adc_speed);
        adc_pot = pid_want;
        sei();
        // Disable debug information to get more accurate function
        printf("%u %u\r\n", adc_pot, speed);

        // Change to potentiometer (ADC0) and get the val
        adc_in_progress = 1;
        change_adc_input(0);
        adc_mode = 2;

        ADCSRA |= _BV(ADSC);
        sleep_cpu();
        change_adc_input(1); // Return to ADC1
    }
}