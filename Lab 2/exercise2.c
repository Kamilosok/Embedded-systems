#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB

#define BAUD 9600                              // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD) - 1) // zgodnie ze wzorem

#define DIT_LEN 400              // dot
#define DAH_LEN DIT_LEN * 3      // dash
#define SPACE_PAUSE DIT_LEN      // between signals
#define LETTER_PAUSE DIT_LEN * 3 // between letters
#define WORD_PAUSE DAH_LEN * 7   // between words (space)

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
// Skopiowane z poprzedniej listy,
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

char decode_morse(uint8_t inputCode, uint8_t inputLen)
{
    for (uint8_t i = 0; i < 36; i++)
    {
        uint8_t size, code;
        size = (0b11100000 & morseCodes[i]) >> 5;
        code = 0b00011111 & morseCodes[i];

        if (size == inputLen && code == inputCode)
        {
            if (i < 26)
                return 'A' + i;
            else
                return '0' + (i - 26);
        }
    }

    return '?';
}

FILE uart_file;

int main()
{
    // Port is input as default
    BTN_PORT |= _BV(BTN);

    LED_DDR |= _BV(LED);

    uart_init();

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    // cyclesUnpressed stars at 8 to not interpret the pause before the first signal as '?'
    // Also these can overflow after some time, I'll interpret it as a signal that the circuit is on, the operator knows that he hasn't pressed a button for a minute or so
    uint8_t cyclesPressed = 0, cyclesUnpressed = 8;
    uint8_t currCode = 0, currLen = 0;

    printf("Morse decoding started: ");

    while (1)
    {
        // If button is not pressed at the start of the cycle
        if (BTN_PIN & _BV(BTN))
        {
            // Turn off the LED always at unpressing
            LED_PORT &= ~_BV(LED);

            if (cyclesUnpressed == 0)
            {
                // If button was pressed 1 or 2 cycles: DIT, if 3 or more: DAH
                if (cyclesPressed >= 3)
                {
                    currCode = currCode << 1;
                    currCode += 1;
                    currLen += 1;
                }
                else if (cyclesPressed >= 1)
                {
                    currCode = currCode << 1;
                    currLen += 1;
                }

                cyclesPressed = 0;
            }
            else if (cyclesUnpressed == 3)
            {
                // Print the letter and reset the code
                printf("%c", decode_morse(currCode, currLen));
                currCode = 0;
                currLen = 0;
            } // Yes, we ignore pauses for more than 7 cycles, it's just a pause
            else if (cyclesUnpressed == 7)
            {
                printf(" ");
                // Also signal the end of a word with a one-cycle LED blink
                LED_PORT |= _BV(LED);
            }

            cyclesUnpressed += 1;
        }
        else
        {
            if (cyclesPressed == 0)
                cyclesUnpressed = 0;

            // If DAH or longer, turn on the LED
            if (cyclesPressed == 3)
                LED_PORT |= _BV(LED);

            cyclesPressed += 1;
        }

        // Because all morse lengths are standardized from the length of a dit signal, I'll interpret the input in DIT_LEN-sized cycles
        _delay_ms(DIT_LEN);
    }
}