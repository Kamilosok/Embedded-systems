/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

static volatile uint32_t adc_last = 0;

// When ADC finishes, save the value
ISR(ADC_vect)
{
    adc_last = ADC;
    ADCSRA |= _BV(ADSC);
}

void adc_init()
{
    ADMUX = _BV(REFS0); // AVcc reference, ADC0 input
    DIDR0 = _BV(ADC0D); // Disable digital input on ADC0
    // ADC clock frequency: 125 kHz (16 MHz / 128)
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // prescaler 128
    ADCSRA |= _BV(ADEN) | _BV(ADIE);               // enable ADC and interrupt
}

#define MOTOR PB1
#define MOTOR_DDR DDRB

#define SERVO_LEFT 575
#define SERVO_RIGHT 2725

void timer_init()
{
    MOTOR_DDR |= _BV(MOTOR);

    TCCR1A |= _BV(COM1A1);

    /*Clear OC1A on Compare Match when up-counting,
    Set OC1A on Compare Match when down-counting.*/

    TCCR1B =
        _BV(WGM13) | // Phase and Frequency Correct
        _BV(CS11);   // Prescaler 8

    // For 50Hz
    ICR1 = 20000;
    OCR1A = (SERVO_LEFT + SERVO_RIGHT) / 2;
}

uint32_t adc_to_servo(uint32_t adc)
{
    // What part of the difference can we move
    return SERVO_LEFT + (adc * (SERVO_RIGHT - SERVO_LEFT)) / 1023;
}

int main()
{

    adc_init();
    timer_init();

    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();
    ADCSRA |= _BV(ADSC);
    _delay_ms(1);

    while (1)
    {
        uint32_t adc;
        cli();
        adc = adc_last;
        sei();

        OCR1A = adc_to_servo(adc);
    }
}