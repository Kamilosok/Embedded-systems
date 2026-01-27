/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

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

// Set timer mode
void ctc_init()
{
    // WGM1 0 - normal
    // CS10 1 - prescaler 1
    TCCR1B = _BV(CS10);
}
FILE uart_file;

// Should be only one cycle of delay with calculating like this (from tests)
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
    uart_init();

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    ctc_init();

    // int8_t section
    uint16_t i8_add = MEASURE_OPERATION(int8_t, +, 10, 21);
    uint16_t i8_mul = MEASURE_OPERATION(int8_t, *, 2, 139);
    uint16_t i8_div = MEASURE_OPERATION(int8_t, /, 98, 7);

    // int16_t section
    uint16_t i16_add = MEASURE_OPERATION(int16_t, +, 1221, 230);
    uint16_t i16_mul = MEASURE_OPERATION(int16_t, *, 30, 191);
    uint16_t i16_div = MEASURE_OPERATION(int16_t, /, 980, 76);

    // int32_t section
    uint16_t i32_add = MEASURE_OPERATION(int32_t, +, 10013, 2304);
    uint16_t i32_mul = MEASURE_OPERATION(int32_t, *, 302, 1503);
    uint16_t i32_div = MEASURE_OPERATION(int32_t, /, 9350, 720);

    // int32_t section
    uint16_t i64_add = MEASURE_OPERATION(int64_t, +, 10013, 2304);
    uint16_t i64_mul = MEASURE_OPERATION(int64_t, *, 3044, 19034);
    uint16_t i64_div = MEASURE_OPERATION(int64_t, /, 98541, 7534);

    // float section
    uint16_t float_add = MEASURE_OPERATION(int64_t, +, 1012.13f, 20.4315f);
    uint16_t float_mul = MEASURE_OPERATION(int64_t, *, 334.24f, 192.0342f);
    uint16_t float_div = MEASURE_OPERATION(int64_t, /, 9804.3f, 7514.34444f);

    printf("int8_t section:\r\n");
    printf("Addition: %" PRId16 ", Multiplication:  %" PRId16 ", Division:  %" PRId16 "\r\n",
           i8_add - 1, i8_mul - 1, i8_div - 1);

    printf("\nint16_t section:\r\n");
    printf("Addition: %" PRId16 ", Multiplication:  %" PRId16 ", Division:  %" PRId16 "\r\n",
           i16_add - 1, i16_mul - 1, i16_div - 1);

    printf("\nint32_t section:\r\n");
    printf("Addition: %" PRId16 ", Multiplication:  %" PRId16 ", Division:  %" PRId16 "\r\n",
           i32_add - 1, i32_mul - 1, i32_div - 1);

    printf("\nint64_t section:\r\n");
    printf("Addition: %" PRId16 ", Multiplication:  %" PRId16 ", Division:  %" PRId16 "\r\n",
           i64_add - 1, i64_mul - 1, i64_div - 1);

    printf("\nfloat section:\r\n");
    printf("Addition: %" PRId16 ", Multiplication:  %" PRId16 ", Division:  %" PRId16 "\r\n",
           float_add - 1, float_mul - 1, float_div - 1);
}

/*
TEST 1

int8_t section:
Addition: 19, Multiplication: 22, Division: 242

int16_t section:
Addition: 26, Multiplication: 34, Division: 242

int32_t section:
Addition: 40, Multiplication: 109, Division: 637

int64_t section:
Addition: 160, Multiplication: 456, Division: 652

float section:
Addition: 100, Multiplication: 360, Division: 556

TEST 2

int8_t section:
Addition: 19, Multiplication: 22, Division: 242

int16_t section:
Addition: 26, Multiplication: 34, Division: 241

int32_t section:
Addition: 40, Multiplication: 109, Division: 634

int64_t section:
Addition: 160, Multiplication: 456, Division: 652

float section:
Addition: 100, Multiplication: 360, Division: 540


In practice:
the compiler uses Y as a pointer to data in RAM (e.g. volatile int8_t),
and the subi/sbci instructions are used to move the pointer
between variables (a, b, c, etc.).
*/