/*Kamil Zdancewicz 345320*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#define NUM_5V 1024UL

void pwm_init(void)
{
    // Also leds are out
    DDRB |= _BV(PB1) | _BV(PB3);
    DDRD |= _BV(PD6);

    TCCR0A = _BV(COM0A1) | _BV(COM0A0) | _BV(WGM01) | _BV(WGM00);
    TCCR0B = _BV(CS01);

    // 8 bit
    TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM10);
    TCCR1B = _BV(WGM12) | _BV(CS11);

    TCCR2A = _BV(COM2A1) | _BV(COM2A0) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS21);
}

void set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    OCR0A = r; // Timer0 (R)
    OCR1A = g; // Timer1 (G)
    OCR2A = b; // Timer2 (B)
}

#define LEN_CYCLE 256

const uint8_t sin_table[LEN_CYCLE] PROGMEM = {
    0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 44, 47,
    50, 53, 56, 59, 62, 65, 68, 71, 74, 77, 80, 83, 86, 89, 92, 95,
    98, 101, 104, 107, 110, 113, 115, 118, 121, 124, 127, 130, 132, 135, 138, 141,
    143, 146, 149, 151, 154, 157, 159, 162, 164, 167, 169, 172, 174, 177, 179, 181,
    184, 186, 188, 190, 193, 195, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215,
    217, 218, 220, 222, 224, 225, 227, 228, 230, 231, 233, 234, 235, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 246, 247, 248, 248, 249, 249, 250, 250, 251,
    251, 251, 251, 252, 252, 252, 252, 252, 252, 251, 251, 251, 251, 250, 250, 249,
    249, 248, 248, 247, 246, 246, 245, 244, 243, 242, 241, 240, 239, 238, 237, 235,
    234, 233, 231, 230, 228, 227, 225, 224, 222, 220, 218, 217, 215, 213, 211, 209,
    207, 205, 203, 201, 199, 197, 195, 193, 190, 188, 186, 184, 181, 179, 177, 174,
    172, 169, 167, 164, 162, 159, 157, 154, 151, 149, 146, 143, 141, 138, 135, 132,
    130, 127, 124, 121, 118, 115, 113, 110, 107, 104, 101, 98, 95, 92, 89, 86,
    83, 80, 77, 74, 71, 68, 65, 62, 59, 56, 53, 50, 47, 44, 40, 37,
    34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3, 0};

// wg. https://www.rapidtables.com/convert/color/hsv-to-rgb.html
void hsv_to_rgb(uint8_t h, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t region = h / 43; // 5 regions
    uint8_t remainder = (h - (region * 43)) * 6;
    uint8_t p = 0;
    uint8_t q = 255 - ((remainder * 255) / 255);
    uint8_t t = (remainder * 255) / 255;

    switch (region)
    {
    case 0:
        *r = 255;
        *g = t;
        *b = p;
        break;
    case 1:
        *r = q;
        *g = 255;
        *b = p;
        break;
    case 2:
        *r = p;
        *g = 255;
        *b = t;
        break;
    case 3:
        *r = p;
        *g = q;
        *b = 255;
        break;
    case 4:
        *r = t;
        *g = p;
        *b = 255;
        break;
    default:
        *r = 255;
        *g = p;
        *b = q;
        break;
    }
}

int main()
{
    pwm_init();

    uint8_t h;
    uint8_t r, g, b;

    uint16_t seed = 123; // Może jakiś szum
    srand(seed);

    while (1)
    {
        h = rand() % 256;
        hsv_to_rgb(h, &r, &g, &b);
        for (uint16_t i = 0; i < LEN_CYCLE; ++i)
        {
            uint8_t brightness = pgm_read_byte(&sin_table[i]);
            set_rgb(((uint16_t)r * (uint16_t)brightness) / 255, ((uint16_t)g * (uint16_t)brightness) / 255, ((uint16_t)b * (uint16_t)brightness) / 255);
            _delay_us(500);
        }
    }
}
