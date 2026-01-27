/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <util/delay.h>

#include "i2c.h"

#define BUF_SIZE 32
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

const uint8_t eeprom_addr = 0xa0;

// Base addr, choosing the bank, and mode
uint8_t addr_to_eeprom(uint16_t addr, uint8_t read)
{
    return eeprom_addr | ((addr & 0x100) >> 7) | (read ? 0x1 : 0x0);
}

int main()
{

    uart_init();

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    i2cInit();

    printf("Options: read addr, write addr value\r\n");

    char buffer[BUF_SIZE];

    while (1)
    {
        // Some parsing
        uint8_t i = 0;
        while (i < BUF_SIZE - 1)
        {
            int c = getchar();
            putc(c, stdout);
            buffer[i++] = c;
            if (c == '\r')
                break;
        }

        buffer[i] = '\0';

        uint8_t args_start = 0;
        for (uint8_t i = 0; i < BUF_SIZE; ++i)
        {
            if (buffer[i] == ' ' || buffer[i] == '\0')
            {
                args_start = i + 1;
                buffer[i] = '\0';
                break;
            }
        }

        uint8_t mode = 0, value = 0;
        uint16_t addr = 0;

        if (!strcmp(buffer, "read"))
            mode = 1;
        else if (!strcmp(buffer, "write"))
            mode = 0;
        else
        {
            printf("Unknown command\r\n");
            continue;
        }

        for (; args_start < BUF_SIZE; ++args_start)
        {
            if (buffer[args_start] == ' ' || buffer[args_start] == '\0' || buffer[args_start] == '\r')
            {
                args_start += 1;
                break;
            }

            addr *= 10;
            addr += buffer[args_start] - '0';
        }

        if (mode == 0)
        {
            for (; args_start < BUF_SIZE; ++args_start)
            {
                if (buffer[args_start] == ' ' || buffer[args_start] == '\0' || buffer[args_start] == '\r')
                {
                    break;
                }

                value *= 10;
                value += buffer[args_start] - '0';
            }
        }

        i2cStart();

        i2cSend(addr_to_eeprom(addr, 0));

        // Address in the bank
        i2cSend(addr & 0xff);

        if (mode == 0)
        {

            i2cSend(value);
            i2cStop();
            _delay_ms(5);
            printf("Sent value %hu to address %u\r\n", value, addr);
        }
        else // mode == 1
        {
            i2cStart();
            i2cSend(addr_to_eeprom(addr, 1));
            uint8_t data = i2cReadNoAck();
            i2cStop();
            printf("Read value %hu from address %u\r\n", data, addr);
        }
    }
}
