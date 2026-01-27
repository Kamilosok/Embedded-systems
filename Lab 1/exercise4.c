/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
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

FILE uart_file;

union calcNum
{
    int8_t d8;
    int16_t d16;
    int32_t d32;
    int32_t d64;
    float f;
};

int main()
{
    uart_init();
    // Printf and scanf may be expensive, but it's much easier in a simple program
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    while (1)
    {
        printf("Choose type: 1) int8 t, 2) int16 t, 3) int32 t, 4) int64 t, 5) float\r\n");
        int8_t numType;
        scanf("%" SCNu8, &numType);
        printf("%" PRId8 "\r\nInput the two numers\r\n", numType);

        union calcNum num1,
            num2;

        switch (numType)
        {
        case 1:
            scanf("%" SCNd8, &num1.d8);
            scanf("%" SCNd8, &num2.d8);

            printf("%" PRId8 " %" PRId8 "\r\n +: %" PRId8 "\r\n*: %" PRId8 "\r\n/: %" PRId8 "\r\n",
                   num1.d8, num2.d8, num1.d8 + num2.d8, num1.d8 * num2.d8, num1.d8 / num2.d8);
            break;

        case 2:
            scanf("%" SCNd16, &num1.d16);
            scanf("%" SCNd16, &num2.d16);

            printf("%" PRId16 " %" PRId16 "\r\n+: %" PRId16 "\r\n*: %" PRId16 "\r\n/: %" PRId16 "\r\n",
                   num1.d16, num2.d16, num1.d16 + num2.d16, num1.d16 * num2.d16, num1.d16 / num2.d16);
            break;

        case 3:
            scanf("%" SCNd32, &num1.d32);
            scanf("%" SCNd32, &num2.d32);

            printf("%" PRId32 " %" PRId32 "\r\n+: %" PRId32 "\r\n*: %" PRId32 "\r\n/: %" PRId32 "\r\n",
                   num1.d32, num2.d32, num1.d32 + num2.d32, num1.d32 * num2.d32, num1.d32 / num2.d32);
            break;

        case 4:
            // `32-bit input/output can be used for these data types` I simply selected the 32-bit field instead of ignoring the warnings, unless it was about scanf %ld
            scanf("%" SCNd32, &num1.d64);
            scanf("%" SCNd32, &num2.d64);

            printf("%" PRId32 " %" PRId32 "\r\n+: %" PRId32 "\r\n*: %" PRId32 "\r\n/: %" PRId32 "\r\n",
                   num1.d64, num2.d64, num1.d64 + num2.d64, num1.d64 * num2.d64, num1.d64 / num2.d64);
            break;

        case 5:
            scanf("%f", &num1.f);
            scanf("%f", &num2.f);

            printf("%f %f\r\n+: %f\r\n*: %f\r\n/: %f\r\n",
                   num1.f, num2.f, num1.f + num2.f, num1.f * num2.f, num1.f / num2.f);
            break;

        default:
            printf("Improper number type!\r\n");
            break;
        }
    }
}

/*
8-bit operations are performed using fast register operations, but division requires the use of external long functions such as __divmodhi4.
16-bit operations require more operations and registers, but are still fast, except for division.
32-bit addition looks normal, but multiplication additionally uses __mulsi3 and division uses __divmodsi4.
floats have their own operations for everything: __addsf3 __mulsf3 __divsf3

Since (from the lecture) there is a constant number of operations per second, the number of operations = speed, so those using external functions are very slow
*/