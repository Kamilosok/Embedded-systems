/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

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

void ctc_init()
{
    // ustaw tryb licznika
    // WGM1  = 0000 -- normal
    // CS1   = 001  -- prescaler 1
    TCCR1B = _BV(CS10);
}
FILE uart_file;

// Powinien byc jeden cykl opóźnienia przy takim liczeniu czasu z licznika (z testów)
#define MEASURE_OPERATION(type, op, val1, val2) ({ \
    volatile type a = val1;                        \
    volatile type b = val2;                        \
    TCNT1 = 0;                                     \
    volatile type result = a op b;                 \
    uint16_t elapsed = TCNT1;                      \
    elapsed;                                       \
})

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj licznik
    ctc_init();

    // Sekcja int8_t
    uint16_t i8_add = MEASURE_OPERATION(int8_t, +, 10, 21);
    uint16_t i8_mul = MEASURE_OPERATION(int8_t, *, 2, 139);
    uint16_t i8_div = MEASURE_OPERATION(int8_t, /, 98, 7);

    // Sekcja int16_t
    uint16_t i16_add = MEASURE_OPERATION(int16_t, +, 1221, 230);
    uint16_t i16_mul = MEASURE_OPERATION(int16_t, *, 30, 191);
    uint16_t i16_div = MEASURE_OPERATION(int16_t, /, 980, 76);

    // Sekcja int32_t
    uint16_t i32_add = MEASURE_OPERATION(int32_t, +, 10013, 2304);
    uint16_t i32_mul = MEASURE_OPERATION(int32_t, *, 302, 1503);
    uint16_t i32_div = MEASURE_OPERATION(int32_t, /, 9350, 720);

    // Sekcja int32_t
    uint16_t i64_add = MEASURE_OPERATION(int64_t, +, 10013, 2304);
    uint16_t i64_mul = MEASURE_OPERATION(int64_t, *, 3044, 19034);
    uint16_t i64_div = MEASURE_OPERATION(int64_t, /, 98541, 7534);

    // Sekcja float
    uint16_t float_add = MEASURE_OPERATION(int64_t, +, 1012.13f, 20.4315f);
    uint16_t float_mul = MEASURE_OPERATION(int64_t, *, 334.24f, 192.0342f);
    uint16_t float_div = MEASURE_OPERATION(int64_t, /, 9804.3f, 7514.34444f);

    printf("Sekcja int8_t:\r\n");
    printf("Dodawanie: %" PRId16 ", Mnożenie:  %" PRId16 ", Dzielenie:  %" PRId16 "\r\n", i8_add - 1, i8_mul - 1, i8_div - 1);

    printf("\nSekcja int16_t:\r\n");
    printf("Dodawanie: %" PRId16 ", Mnożenie:  %" PRId16 ", Dzielenie:  %" PRId16 "\r\n", i16_add - 1, i16_mul - 1, i16_div - 1);

    printf("\nSekcja int32_t:\r\n");
    printf("Dodawanie: %" PRId16 ", Mnożenie:  %" PRId16 ", Dzielenie:  %" PRId16 "\r\n", i32_add - 1, i32_mul - 1, i32_div - 1);

    printf("\nSekcja int64_t:\r\n");
    printf("Dodawanie: %" PRId16 ", Mnożenie:  %" PRId16 ", Dzielenie:  %" PRId16 "\r\n", i64_add - 1, i64_mul - 1, i64_div - 1);

    printf("\nSekcja float:\r\n");
    printf("Dodawanie: %" PRId16 ", Mnożenie:  %" PRId16 ", Dzielenie:  %" PRId16 "\r\n", float_add - 1, float_mul - 1, float_div - 1);
}

/*
TEST 1

Sekcja int8_t:
Dodawanie: 19, Mnożenie:  22, Dzielenie:  242

Sekcja int16_t:
Dodawanie: 26, Mnożenie:  34, Dzielenie:  242

Sekcja int32_t:
Dodawanie: 40, Mnożenie:  109, Dzielenie:  637

Sekcja int64_t:
Dodawanie: 160, Mnożenie:  456, Dzielenie:  652

Sekcja float:
Dodawanie: 100, Mnożenie:  360, Dzielenie:  556


TEST 2

Sekcja int8_t:
Dodawanie: 19, Mnożenie:  22, Dzielenie:  242

Sekcja int16_t:
Dodawanie: 26, Mnożenie:  34, Dzielenie:  241

Sekcja int32_t:
Dodawanie: 40, Mnożenie:  109, Dzielenie:  634

Sekcja int64_t:
Dodawanie: 160, Mnożenie:  456, Dzielenie:  652

Sekcja float:
Dodawanie: 100, Mnożenie:  360, Dzielenie:  540
*/

/*
W praktyce:
kompilator używa Y jako wskaźnika do danych w RAM (np. volatile int8_t),
a te subi/sbci służą do przemieszczania wskaźnika między zmiennymi (a, b, c itp.).
*/