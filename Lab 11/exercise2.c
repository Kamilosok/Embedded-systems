/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

static volatile uint32_t adc_last = 0;
static volatile uint32_t adc_speed = 0;
static volatile uint8_t adc_mode = 0; // 0 - torque, 1 - speed

// When ADC finishes, save the value
ISR(ADC_vect)
{
    if (adc_mode)
        adc_speed = ADC;
    else
        adc_last = ADC;
}

// Middle of MOSFET ON is when downcounts to 0
ISR(TIMER1_OVF_vect)
{
    adc_mode = 0;
    ADCSRA |= _BV(ADSC);
}

// Middle of MOSFET OFF is when reaches top
ISR(TIMER1_CAPT_vect)
{
    adc_mode = 1;
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
    TIMSK1 |= _BV(TOIE1);
    TIMSK1 |= _BV(ICIE1);

    // For 1kHz
    ICR1 = 1000;
    OCR1A = 500;
}

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

// inicjalizacja UART
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
    while (!(UCSR0A & _BV(UDRE0)))
        ;
}

FILE uart_file;

static inline uint16_t adc_to_mv(uint32_t adc)
{
    // 5000 mV * adc / 1023
    return adc * 5000UL / 1023UL;
}

int main()
{

    adc_init();
    timer_init();
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    while (1)
    {
        // Can't print from volatile
        uint32_t speed;
        uint32_t torque;

        cli();
        speed = 5000 - adc_to_mv(adc_speed);
        torque = adc_to_mv(adc_last);
        sei();

        printf("Speed=%lu Torque=%lu\r\n", speed, torque);

        sleep_cpu();
    }
}