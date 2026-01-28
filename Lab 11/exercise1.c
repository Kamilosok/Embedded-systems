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

ISR(TIMER1_COMPA_vect)
{
    ;
}

#define MOTOR PB1
#define MOTOR_DDR DDRB

void timer_init()
{
    MOTOR_DDR |= _BV(MOTOR);

    TCCR1A = _BV(WGM10) | _BV(WGM11); // Phase correct PWM

    TCCR1A |= _BV(COM1A1);
    /*Clear OC1A on Compare Match when up-counting,
    Set OC1A on Compare Match when down-counting.*/

    TCCR1B =
        _BV(WGM12) | // Phase correct PWM
        _BV(CS11);   // Prescaler 8

    OCR1A = 0;
}

int main()
{

    adc_init();
    timer_init();

    sei();
    ADCSRA |= _BV(ADSC);
    _delay_ms(1);

    while (1)
    {
        // Since both are in the range 0, 1023, we can set it directly
        OCR1A = adc_last;
    }
}