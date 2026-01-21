/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <util/delay.h>

#include "i2c.h"

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

#define BUF_SIZE 256ul
#define MAX_RECORD_SIZE 255ul
#define DATA_SIZE 512ul

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

const uint8_t eeprom_addr = 0xa0;

// Base addr, choosing the bank, and mode
uint8_t addr_to_eeprom(uint16_t addr, uint8_t read)
{
    return eeprom_addr | ((addr & 0x100) >> 7) | (read ? 0x1 : 0x0);
}

// Nie da się buforem
void generate_record(uint16_t start_addr, uint8_t data_len, uint8_t *data_ptr)
{
    uint8_t checksum_sum = 0;
    uint8_t record_type = 0x00;

    // START
    putchar(':');

    // BYTECOUNT
    printf("%02X", data_len);
    checksum_sum += data_len;

    // ADDRESS
    printf("%02X%02X", (start_addr >> 8) & 0xFF, start_addr & 0xFF);
    checksum_sum += (start_addr >> 8) & 0xFF;
    checksum_sum += start_addr & 0xFF;

    // RECORD TYPE
    printf("%02X", record_type);
    checksum_sum += record_type;

    // DATA
    for (int i = 0; i < data_len; i++)
    {
        uint8_t b = data_ptr[i];
        printf("%02X", b);
        checksum_sum += b;
    }

    // CHECKSUM
    uint8_t checksum = (uint8_t)(~checksum_sum + 1);
    printf("%02X\r\n", checksum);
}

void print_eeprom(uint16_t start_address,
                  uint16_t data_length,
                  uint8_t *data)
{

    // Rekord 1
    uint16_t len1 = data_length;
    if (data_length > MAX_RECORD_SIZE)
    {
        len1 = MAX_RECORD_SIZE;
    }
    // printf("RECORD ONE:\r\n");
    generate_record(start_address, (uint8_t)len1, data);
    // Potencjalnie rekord 2
    if (data_length > MAX_RECORD_SIZE)
    {

        uint16_t len2 = data_length - MAX_RECORD_SIZE - 1;
        uint16_t addr2 = start_address + MAX_RECORD_SIZE + 1;
        uint8_t *data_ptr_2 = (uint8_t *)data + MAX_RECORD_SIZE + 1;

        if (addr2 + len2 > DATA_SIZE)
            len2 = DATA_SIZE - addr2;
        // printf("RECORD TWO:\r\n");
        generate_record(addr2, (uint8_t)len2, data_ptr_2);
    }

    // EOF
    // printf("EOF:\r\n");
    printf(":00000001FF\r\n");
}

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj I2C
    i2cInit();

    printf("Options: read addr, read addr length, write, write addr value\r\n");

    char buffer[BUF_SIZE];

    while (1)
    {
        // I tak trzeba będzie napisać parser trudniejszego formatu więc rozgrzewka
        uint8_t i = 0, end = 0;
        while (i < BUF_SIZE - 1)
        {
            int c = getchar();
            putc(c, stdout);
            buffer[i++] = c;
            if (c == '\r' || c == EOF)
                break;
        }

        buffer[i] = '\0';

        uint8_t args_start = 0;
        for (uint8_t i = 0; i < BUF_SIZE; ++i)
        {
            if (buffer[i] == ' ')
            {
                args_start = i + 1;
                buffer[i] = '\0';
                break;
            }

            if (buffer[i] == '\r')
            {
                buffer[i] = '\0';
                end = 1;
                break;
            }
        }

        uint8_t mode = 0;
        uint16_t addr = 0, value = 0;

        if (!strcmp(buffer, "read"))
            mode = 1;
        else if (!strcmp(buffer, "write"))
            mode = 0;
        else
        {
            printf("Unknown command\r\n");
            continue;
        }

        if (mode == 0 && end)
        {
            // printf("Long write\r\n");

            uint8_t byte_count;
            uint16_t address;
            uint8_t record_type;
            uint8_t data[BUF_SIZE];
            uint8_t checksum;

            // Zmienne pomocnicze
            unsigned int temp_val;  // Do bezpiecznego wczytywania przez scanf
            unsigned int temp_addr; // Do adresu
            uint8_t i;
            uint8_t calc_sum; // Do obliczania sumy kontrolnej
            char start_code = 0;

            while (1)
            {
                calc_sum = 0;
                i = 0;
                start_code = 0;
                while (start_code != ':')
                {
                    start_code = getchar();
                }

                // BYTE COUNT
                scanf("%2x", &temp_val);
                byte_count = (uint8_t)temp_val;
                calc_sum += byte_count;

                // ADDRESS
                scanf("%4x", &temp_addr);
                address = (uint16_t)temp_addr;
                calc_sum += (address >> 8) + (address & 0xFF);

                // RECORD TYPE
                scanf("%2x", &temp_val);
                record_type = (uint8_t)temp_val;
                calc_sum += record_type;

                int input_char;
                // printf("RECORD TYPE:%hu\r\n", record_type);
                if (record_type == 0x01)
                {

                    // Czyszczenie
                    while ((input_char = getchar()) != '\r' && input_char != EOF)
                        ;

                    // printf("End of long write\r\n");
                    break;
                }

                // DATA
                for (i = 0; i < byte_count; i++)
                {
                    scanf("%2x", &temp_val);

                    data[i] = (uint8_t)temp_val;
                    calc_sum += data[i];
                }

                // CHECKSUM
                scanf("%2x", &temp_val);
                checksum = (uint8_t)temp_val;

                calc_sum = (~calc_sum) + 1;

                // Czyszczenie
                while ((input_char = getchar()) != '\r' && input_char != EOF)
                    ;

                // printf("ADR: %04X, LEN: %02X, TYP: %02X\r\n", address, byte_count, record_type);

                // || 1 for debug
                if (calc_sum == checksum)
                {
                    // Sending

                    uint8_t page_counter = address & 0xF;
                    uint8_t data_counter = 0;

                    // Adres w stronie
                    uint8_t i = page_counter;

                    printf("Starting at %u\r\n", address);

                    i2cStart();

                    i2cSend(addr_to_eeprom(address, 0));

                    // address in the bank
                    i2cSend(address & 0xff);

                    while (data_counter < byte_count && (address & ~0xF) + i < DATA_SIZE)
                    {
                        if (i == 0x10)
                        {

                            // Do nowej strony, dodatkowo wyzerowanie adresu w stronie
                            address += 0x10;
                            address = address & ~0xF;

                            i2cStop();
                            _delay_ms(5);

                            i2cStart();

                            i2cSend(addr_to_eeprom(address, 0));

                            // address in the bank
                            i2cSend(address & 0xff);

                            printf("Addr set to:%u\r\n", address);
                            i = 0;
                        }

                        // printf("Sent value %hu to address %u\r\n", data[data_counter], (address & ~0xF) + i);
                        i2cSend(data[data_counter]);

                        data_counter += 1;
                        i += 1;
                    }

                    i2cStop();
                    _delay_ms(5);
                    // printf("Sent value %hu to address %u\r\n", value, addr);
                }
                else
                {
                    break;
                }
            }
        }
        else
        {

            for (; args_start < BUF_SIZE; ++args_start)
            {
                if (buffer[args_start] == ' ')
                {
                    args_start += 1;
                    break;
                }

                if (buffer[args_start] == '\r')
                {
                    end = 1;
                    break;
                }

                addr *= 10;
                addr += buffer[args_start] - '0';
            }

            if (mode == 1 && end)
            {
                i2cStart();

                i2cSend(addr_to_eeprom(addr, 0));

                // address in the bank
                i2cSend(addr & 0xff);

                i2cStart();
                i2cSend(addr_to_eeprom(addr, 1));
                uint8_t data = i2cReadNoAck();
                i2cStop();
                printf("Read value %hu from address %u\r\n", data, addr);
            }
            else
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

                // Must be the end
                if (mode == 0)
                {
                    i2cStart();

                    i2cSend(addr_to_eeprom(addr, 0));

                    // address in the bank
                    i2cSend(addr & 0xff);

                    i2cSend(value);
                    i2cStop();
                    _delay_ms(5);
                    printf("Sent value %hu to address %u\r\n", value, addr);
                }
                else
                {
                    // printf("Long read\r\n");

                    i2cStart();

                    i2cSend(addr_to_eeprom(addr, 0));

                    // address in the bank
                    i2cSend(addr & 0xff);

                    i2cStart();
                    i2cSend(addr_to_eeprom(addr, 1));

                    uint8_t data_arr[DATA_SIZE];

                    // Najwyraźniej poprawnie inkrementuje A8 aby przejść do drugiego banku, więc nie trzeba tego handlować, 1 to 512 w nocie sequence reada
                    for (uint16_t i = 0; i < value; i++)
                    {
                        if (addr + i >= DATA_SIZE)
                            break;

                        if (i < value - 1)
                            data_arr[i] = i2cReadAck(); // ACK after each byte except last
                        else
                            data_arr[i] = i2cReadNoAck(); // NACK after last byte

                        // Debugowe
                        // printf("Read value %hu from address %u\r\n", data_arr[i], addr + i);
                    }

                    i2cStop();
                    print_eeprom(addr, value, data_arr);
                }
            }
        }
    }
}
