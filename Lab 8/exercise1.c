// Kamil Zdancewicz 345320
#include "FreeRTOS.h"
#include "task.h"

#include <util/delay.h>
#include <avr/io.h>

#define pressLedDelay_TASK_PRIORITY 1

#define kitt_TASK_PRIORITY 2

static void vPressLedDelay(void *pvParameters);

static void vKitt(void *pvParameters);

int main(void)
{
    // Create task.
    xTaskHandle pressLedDelay_handle;
    xTaskHandle kitt_handle;

    xTaskCreate(
        vPressLedDelay,
        "ledDelay",
        configMINIMAL_STACK_SIZE,
        NULL,
        pressLedDelay_TASK_PRIORITY,
        &pressLedDelay_handle);

    xTaskCreate(
        vKitt,
        "kitt",
        configMINIMAL_STACK_SIZE,
        NULL,
        kitt_TASK_PRIORITY,
        &kitt_handle);

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}

void vApplicationIdleHook(void)
{
}


#define LEDK_DDR DDRD
#define LEDK_PORT PORTD

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB

#define BUF_SIZE 128
#define SAMPLE_LEN 10

uint8_t history[BUF_SIZE];

// The first 1s is blank
uint8_t read_id = 0;
uint8_t write_id = 100;

void next_cycle()
{
    // To not use modulo, we waste some memory (28 bytes) to return to the initial index of history faster
    read_id = (read_id + 1) & (BUF_SIZE - 1);
    write_id = (write_id + 1) & (BUF_SIZE - 1);
}

static void vPressLedDelay(void *pvParameters)
{

    // Port is input as default
    BTN_PORT |= _BV(BTN);

    LED_DDR |= _BV(LED);

    while (1)
    {
        // If button is not pressed at the start of the cycle
        if (BTN_PIN & _BV(BTN))
            history[write_id] = 0;
        else
            history[write_id] = 1;

        if (history[read_id])
            LED_PORT |= _BV(LED);
        else
            LED_PORT &= ~_BV(LED);

        next_cycle();
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_LEN));
    }
}

static void vKitt(void *pvParameters)
{
    // So we can use TXD and RXD for our LED
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);

    // ALL output
    LEDK_DDR = 0b11111111;

    // 0 - right, 1 - left
    uint8_t direction = 0;

    // Lighting only one LED at a time, states go from 0 to 0b10000000
    uint8_t state = 1, prevState = 1;

    while (1)
    {
        if (direction == 0)
        {
            state = state << 1;

            if (state == 0b10000000)
                direction = 1;
        }
        else
        {
            state = state >> 1;

            if (state == 0)
            {
                direction = 0;
                state = 1 << 1;
            }
        }

        LEDK_PORT |= state;
        vTaskDelay(pdMS_TO_TICKS(50));
        LEDK_PORT &= ~prevState;
        prevState = state;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
