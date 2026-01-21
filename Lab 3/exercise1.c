#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define BUZZ PB5
#define BUZZ_DDR DDRB
#define BUZZ_PORT PORTB

#define TEMPO 120UL
#define WHOLE_NOTE (240000UL / TEMPO)
#define HALF_NOTE (120000UL / TEMPO)
#define QUARTER_NOTE (60000UL / TEMPO)
#define EIGHTH_NOTE (30000UL / TEMPO)
#define SIXTEENTH_NOTE (15000UL / TEMPO)
#define THIRTYSECOND_NOTE (7500UL / TEMPO)
#define SIXTYFOURTH_NOTE (3750UL / TEMPO)
#define HUNDREDTWENTYEIGHTH_NOTE (1875UL / TEMPO)

#define DOTTED_EIGHTH_NOTE (EIGHTH_NOTE + SIXTEENTH_NOTE)
#define DOTTED_QUARTER_NOTE (QUARTER_NOTE + EIGHTH_NOTE)
#define DOTTED_HALF_NOTE (HALF_NOTE + QUARTER_NOTE)

// In C major, from 4th octave
#define FREQ_C4 262
#define FREQ_D4 294
#define FREQ_E4 330
#define FREQ_F4 349
#define FREQ_G4 392
#define FREQ_A4 440
#define FREQ_B4 494
#define FREQ_C5 523

#define FREQ_PAUSE 0

typedef struct
{
    uint16_t freq;
    uint32_t dur; // Duration in uss
} note_t;
/*
 * Melodia: "Computer Blue" (Solo, ok. taktu 82)
 *
 * UWAGA: Solo zostało mocno uproszczone i transponowane
 * diatonicznie do skali C-dur (z C4 do C5), aby pasowało
 * do BARDZO ograniczonych definicji częstotliwości.
 * Rytm i nuty są jedynie przybliżeniem.
 */
static const note_t melody[] PROGMEM = {

    // Takt 82: Długa nuta (oryg. D5 z bendem)
    // Transpozycja: C5
    {FREQ_C5, DOTTED_QUARTER_NOTE},
    {FREQ_PAUSE, EIGHTH_NOTE},

    // Takt 83: Szybki przebieg w dół (oryg. D-C#-B-A-C#-B-A-G)
    // Transpozycja: C-B-A-G-B-A-G-F
    {FREQ_C5, SIXTEENTH_NOTE},
    {FREQ_B4, SIXTEENTH_NOTE},
    {FREQ_A4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_B4, SIXTEENTH_NOTE},
    {FREQ_A4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_F4, SIXTEENTH_NOTE},

    // Takt 84: Szybki przebieg "wahadłowy" (oryg. F#-G-A-B-A-G-F#-G)
    // Transpozycja: E-F-G-A-G-F-E-F
    {FREQ_E4, SIXTEENTH_NOTE},
    {FREQ_F4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_A4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_F4, SIXTEENTH_NOTE},
    {FREQ_E4, SIXTEENTH_NOTE},
    {FREQ_F4, SIXTEENTH_NOTE},

    // --- NOWA SEKCJA ---

    // Takt 85: Przebieg w górę (oryg. G-A-B-C# i D-D-D-D)
    // Transpozycja: F-G-A-B i C-C-C-C
    {FREQ_F4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_A4, SIXTEENTH_NOTE},
    {FREQ_B4, SIXTEENTH_NOTE},
    {FREQ_C5, SIXTEENTH_NOTE},
    {FREQ_C5, SIXTEENTH_NOTE},
    {FREQ_C5, SIXTEENTH_NOTE},
    {FREQ_C5, SIXTEENTH_NOTE},

    // Takt 86: Przebieg w dół (taki sam jak takt 83)
    // Transpozycja: C-B-A-G-B-A-G-F
    {FREQ_C5, SIXTEENTH_NOTE},
    {FREQ_B4, SIXTEENTH_NOTE},
    {FREQ_A4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_B4, SIXTEENTH_NOTE},
    {FREQ_A4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_F4, SIXTEENTH_NOTE},

    // Takt 87: Przebieg "wahadłowy" (taki sam jak takt 84)
    // Transpozycja: E-F-G-A-G-F-E-F
    {FREQ_E4, SIXTEENTH_NOTE},
    {FREQ_F4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_A4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_F4, SIXTEENTH_NOTE},
    {FREQ_E4, SIXTEENTH_NOTE},
    {FREQ_F4, SIXTEENTH_NOTE},

    // Takt 88: Szybki bieg chromatyczny (oryg. A-B-C#-D-E-F#-G#-A#)
    // Transpozycja: Jako substytut gramy po prostu całą dostępną skalę C-dur w górę
    {FREQ_C4, SIXTEENTH_NOTE},
    {FREQ_D4, SIXTEENTH_NOTE},
    {FREQ_E4, SIXTEENTH_NOTE},
    {FREQ_F4, SIXTEENTH_NOTE},
    {FREQ_G4, SIXTEENTH_NOTE},
    {FREQ_A4, SIXTEENTH_NOTE},
    {FREQ_B4, SIXTEENTH_NOTE},
    {FREQ_C5, SIXTEENTH_NOTE},

    // Takt 89: Długa nuta kończąca solo (oryg. B5 z bendem)
    // Transpozycja: Najwyższa dostępna nuta C5
    {FREQ_C5, DOTTED_HALF_NOTE},

    // Takt 90: Pauza
    {FREQ_PAUSE, WHOLE_NOTE},
    // Takt 91: (oryg. B-A-G-A)
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_A4, EIGHTH_NOTE},
    {FREQ_G4, EIGHTH_NOTE},
    {FREQ_A4, EIGHTH_NOTE},

    // Takt 92: (powtórzenie)
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_A4, EIGHTH_NOTE},
    {FREQ_G4, EIGHTH_NOTE},
    {FREQ_A4, EIGHTH_NOTE},

    // Takt 93: (oryg. G-F#-E-F#)
    // Transpozycja: G-F-E-F
    {FREQ_G4, EIGHTH_NOTE},
    {FREQ_F4, EIGHTH_NOTE},
    {FREQ_E4, EIGHTH_NOTE},
    {FREQ_F4, EIGHTH_NOTE},

    // Takt 94: (powtórzenie)
    {FREQ_G4, EIGHTH_NOTE},
    {FREQ_F4, EIGHTH_NOTE},
    {FREQ_E4, EIGHTH_NOTE},
    {FREQ_F4, EIGHTH_NOTE},

    // Takt 95: (powtórzenie taktu 91)
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_A4, EIGHTH_NOTE},
    {FREQ_G4, EIGHTH_NOTE},
    {FREQ_A4, EIGHTH_NOTE},

    // Takt 96: (powtórzenie taktu 92)
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_A4, EIGHTH_NOTE},
    {FREQ_G4, EIGHTH_NOTE},
    {FREQ_A4, EIGHTH_NOTE},

    // Takt 97: (powtórzenie taktu 93)
    {FREQ_G4, EIGHTH_NOTE},
    {FREQ_F4, EIGHTH_NOTE},
    {FREQ_E4, EIGHTH_NOTE},
    {FREQ_F4, EIGHTH_NOTE},

    // Takt 98: (powtórzenie taktu 94)
    {FREQ_G4, EIGHTH_NOTE},
    {FREQ_F4, EIGHTH_NOTE},
    {FREQ_E4, EIGHTH_NOTE},
    {FREQ_F4, EIGHTH_NOTE},

    // Takt 99: Riff D5 (transponowany na C5), ósemki
    // Oryginalnie: D5
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},

    // Takt 100: Riff B4
    // Oryginalnie: B4
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},

    // Takt 101: Riff A4 (powrót do D5 w oryginalnej tonacji, tu C5)
    // Oryginalnie: D5
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},
    {FREQ_C5, EIGHTH_NOTE},

    // Takt 102: Riff B4
    // Oryginalnie: B4
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},

    // Takt 103: Zmiana riffu - C# (transponowany na D4)
    // Oryginalnie: C#4
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},

    // Takt 104: Powrót do B4
    // Oryginalnie: B4
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},

    // Takt 105: Zmiana riffu - C# (transponowany na D4)
    // Oryginalnie: C#4
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},
    {FREQ_D4, EIGHTH_NOTE},

    // Takt 106: Powrót do B4
    // Oryginalnie: B4
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},
    {FREQ_B4, EIGHTH_NOTE},

    // Koniec sekcji
    {FREQ_PAUSE, WHOLE_NOTE}};

// I swear this is the only option without external timer
void delay_us_runtime(uint32_t us)
{
    while (us--)
    {
        _delay_us(1);
    }
}

void play_tone(uint16_t freq, uint32_t duration_ms)
{
    if (freq == 0)
    {
        delay_us_runtime(1000UL * duration_ms);
        return;
    }
    uint16_t period_us = 1000000UL / freq;
    uint16_t half_period = period_us / 2;
    uint32_t cycles = (uint32_t)duration_ms * 1000UL / period_us;

    for (uint32_t i = 0; i < cycles; i++)
    {
        BUZZ_PORT |= _BV(BUZZ);
        delay_us_runtime(half_period);
        BUZZ_PORT &= ~_BV(BUZZ);
        delay_us_runtime(half_period);
    }
}

int main()
{
    BUZZ_DDR |= _BV(BUZZ);

    for (uint8_t i = 0; i < sizeof(melody) / sizeof(note_t); ++i)
    {
        uint16_t freq = pgm_read_word(&melody[i].freq);

        uint32_t dur = pgm_read_dword(&melody[i].dur);
        play_tone(freq, dur);
    }
}