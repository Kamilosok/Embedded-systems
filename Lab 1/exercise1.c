#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

#define DIT_LEN 200
#define DAH_LEN DIT_LEN * 3
#define SPACE_PAUSE DIT_LEN
#define LETTER_PAUSE DIT_LEN * 3
#define WORD_PAUSE DAH_LEN * 7

#define BUF_SIZE 32

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

// . 1, - 3, przerwy między znakami 1, między literami 3
// Max 5 długości, kodowanie: największe 3 bity oznaczają długość kodu (bo <8), następnie max 5 znaków oznacza 0 -> . , 1 -> - , dopasowane do prawej
// PROGMEM ?
const uint8_t morseCodes[36] = {
    0b01000001, // A .-
    0b10001000, // B -...
    0b10001010, // C -.-.
    0b01100100, // D -..
    0b00100000, // E .
    0b10000010, // F ..-.
    0b01100110, // G --.
    0b10000000, // H ....
    0b01000000, // I ..
    0b10000111, // J .---
    0b01100101, // K -.-
    0b10000100, // L .-..
    0b01000011, // M --
    0b01000010, // N -.
    0b01100111, // O ---
    0b10000110, // P .--.
    0b10001101, // Q --.-
    0b01100010, // R .-.
    0b01100000, // S ...
    0b00100001, // T -
    0b01100001, // U ..-
    0b10000001, // V ...-
    0b01100011, // W .--
    0b10001001, // X -..-
    0b10001011, // Y -.--
    0b10001100, // Z --..

    0b10111111, // 0 -----
    0b10101111, // 1 .----
    0b10100111, // 2 ..---
    0b10100011, // 3 ...--
    0b10100001, // 4 ....-
    0b10100000, // 5 .....
    0b10110000, // 6 -....
    0b10111000, // 7 --...
    0b10111100, // 8 ---..
    0b10111110  // 9 ----.
};

// Converts a morse-compatible character to it's representative id in the morseCodes array, should normalize size
uint8_t charToId(char character)
{
    if (character - '0' >= 0 && character - '0' <= 9)
        return 26 + character - '0';

    if (character - 'A' >= 0 && character - 'A' <= 25)
        return character - 'A';

    if (character - 'a' >= 0 && character - 'a' <= 25)
        return character - 'a';

    return 0;
}

// Signal ONLY the character by diode, inter-character and inter-word pauses handled outside, expects the diode to be 0, at the start, and leaves it at 0
void blinkById(uint8_t id)
{
    uint8_t size, code;
    size = (0b11100000 & morseCodes[id]) >> 5;
    code = 0b00011111 & morseCodes[id];

    for (uint8_t i = 0; i < size; i++)
    {
        LED_PORT |= _BV(LED);
        if (code & (1 << (size - 1)))
            _delay_ms(DAH_LEN);
        else
            _delay_ms(DIT_LEN);

        LED_PORT &= ~_BV(LED);
        code = code << 1;

        // Signal absence
        if (i != size - 1)
            _delay_ms(SPACE_PAUSE);
    }
}

FILE uart_file;

int main()
{
    // Set to output
    LED_DDR |= _BV(LED);
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    char buffer[BUF_SIZE];

    while (1)
    {
        uint8_t i = 0;
        printf("Enter string (only a-z, 0-9): \r\n");
        while (i < BUF_SIZE - 1)
        {
            char c;
            c = getchar();
            if (c == '\r' || c == '\n')
                break;
            else
                buffer[i++] = c;
        }

        buffer[i] = 0;
        printf("Received %s\r\n", buffer);

        for (uint8_t a = 0; a < i; a++)
        {
            if (buffer[a] == ' ')
                _delay_ms(WORD_PAUSE);
            else
            {
                uint8_t id = charToId(buffer[a]);
                blinkById(id);

                if (a != i - 1)
                    _delay_ms(LETTER_PAUSE);
            }
        }
    }

    return 0;
}