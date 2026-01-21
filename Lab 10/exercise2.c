/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include "hd44780.h"

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

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
/*
SET CGRAM ADDRESS bity 5-3 to numer znaku, 2-0 numer wiersza
*/

// num_char < 8, num_bars < 6
void upload_bar_char(uint8_t num_char, uint8_t num_bars)
{
    LCD_WriteCommand(HD44780_CGRAM_SET | ((num_char) << 3) | 0);

    uint8_t row_img = 0;
    for (uint8_t paint = 0; paint < 5; ++paint)
    {
        row_img = row_img << 1;

        if (paint < num_bars)
            row_img |= 1;
    }

    for (uint8_t row = 0; row < 7; ++row)
    {
        LCD_WriteData(row_img);
    }

    LCD_WriteData(0x00); // Cursor
}

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

    for (uint8_t i = 0; i <= 5; ++i)
    {
        upload_bar_char(i, i);
    }

    LCD_WriteCommand(HD44780_DDRAM_SET);

    uint8_t progress = 0; // <= 80

    while (1)
    {
        uint8_t col = progress / 6;
        uint8_t fill = progress % 6;

        if (col >= 16)
        {
            LCD_Clear();
            progress = 0;
            continue;
        }

        LCD_GoTo(col, 0);
        LCD_WriteData(fill);

        progress++;
        _delay_ms(100);
    }
}
