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
    ADMUX = _BV(REFS0);                            // referencja 5V, ADC0
    DIDR0 = _BV(ADC0D);                            // wyłącz wejście cyfrowe na ADC0
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    ADCSRA |= _BV(ADEN) | _BV(ADIE);               // włącz ADC i przerwania
}

#define MOTOR PB1
#define MOTOR_DDR DDRB

#define CTRL_DDR DDRB
#define CTRL_PORT PORTB
#define LEFT PB3  // A0
#define RIGHT PB4 // A1

void timer_init()
{
    MOTOR_DDR |= _BV(MOTOR);

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

int main()
{

    adc_init();
    timer_init();

    CTRL_DDR |= _BV(LEFT) | _BV(RIGHT);

    sei();
    ADCSRA |= _BV(ADSC);
    _delay_ms(1);

    while (1)
    {
        uint32_t adc, pwm;
        cli();
        adc = adc_last;
        sei();

        if (adc < 512)
        {
            // Rotate left
            CTRL_PORT &= ~_BV(RIGHT);
            CTRL_PORT |= _BV(LEFT);

            pwm = (512 - adc) * 2;
        }
        else
        {
            // Rotate right
            CTRL_PORT &= ~_BV(LEFT);
            CTRL_PORT |= _BV(RIGHT);
            pwm = (adc - 512) * 2;
        }

        OCR1A = pwm; // EN
    }
}