/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include "hd44780.h"

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

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

int hd44780_transmit(char data, FILE *stream)
{
    LCD_WriteData(data);
    return 0;
}

FILE hd44780_file;
FILE uart_file;

int main()
{
    // skonfiguruj wyświetlacz
    LCD_Initialize();
    uart_init();

    LCD_Clear();
    // skonfiguruj strumienie
    fdev_setup_stream(&hd44780_file, hd44780_transmit, NULL, _FDEV_SETUP_WRITE);
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    // Cursor visible + blinking
    LCD_WriteCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_ON | HD44780_CURSOR_BLINK);

    char second_row[17];
    char in_c;

    uint8_t row_num = 0;
    uint8_t col_num = 0;

    while (1)
    {
        in_c = getchar();

        // printf("Row %hu Col %hu '%c'\r\n", row_num, col_num, in_c);

        if (in_c == '\r')
        {
            second_row[col_num] = '\0';
            row_num += 1;
            col_num = 0;
            if (row_num >= 2)
            {
                row_num = 0;
                LCD_WriteCommand(HD44780_CLEAR);
                _delay_ms(2);
                LCD_GoTo(col_num, row_num);

                fprintf(&hd44780_file, "%s", second_row);
                second_row[0] = '\0';

                row_num += 1;
            }
        }
        else
        {
            if (row_num == 1)
            {
                second_row[col_num] = in_c;
            }

            fprintf(&hd44780_file, "%c", in_c);
            col_num += 1;
        }

        // Trochę powtórzone
        if (col_num >= 16)
        {
            second_row[16] = '\0';
            row_num += 1;
            col_num = 0;
            if (row_num >= 2)
            {
                row_num = 0;
                LCD_WriteCommand(HD44780_CLEAR);
                _delay_ms(2);
                LCD_GoTo(col_num, row_num);

                fprintf(&hd44780_file, "%s", second_row);
                second_row[0] = '\0';

                row_num += 1;
            }
        }

        LCD_GoTo(col_num, row_num);
    }
}
