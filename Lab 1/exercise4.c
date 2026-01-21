#include <avr/io.h>
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

FILE uart_file;

union calcNum
{
    int8_t d8;
    int16_t d16;
    int32_t d32;
    int32_t d64;
    float f;
};

//  dodawania, mnożenia i dzielenia czyli wszystkie

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
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
            // `można dla tych typów danych użyć 32-bitowego wejścia/wyjścia` wybrałem po prostu pole 32-bitowe zamiast ignorowania warningów, chyba ze chodziło o scanf %ld
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
8-bitowe są realizowane szybkimi operajami na rejestrach, ale dzielenie wymaga użycia zewnętrznych długich funkcji jak __divmodhi4
16-bitowe wymagają więcej operacji i rejestrów, ale ciągle są szybkie, oprócz dzielenia
32-bitowe dodawanie wygląda normalnie, ale mnożenie korzysta dodatkowo z __mulsi3 a dzielenie z __divmodsi4
floaty mają do wszystkiego własne operajcje: __addsf3 __mulsf3 __divsf3

Ponieważ (z wykładu) jest stała ilość operacji na sekundę, ilość operacji = prędkość, więc te używające zewnętrznych funkcji są bardzo wolne
*/