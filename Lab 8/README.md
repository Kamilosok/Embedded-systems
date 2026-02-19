# Lab 8

> For all exercises version ≥ 10 of FreeRTOS should be used, we used one from [tilk](https://github.com/tilk/freertos_atmega328P)

## Exercise 1: Real-time multiprocessing

Write a program utilizing FreeRTOS running two tasks simultaneously:

- [Press delay](../Lab%202/README.md#exercise-1-press-delay)
- [KITT Lights](../Lab%201/README.md#exercise-2-kitt-lights)

The button state checking may be realized in any way.

### Solution E1

The solution is in [`exercise1.c`](exercise1.c)

## Exercise 2: Real-time IPC

Write a program utilizing FreeRTOS running two tasks:

- **Task one** reads integers from UART and sends them to a FreeRTOS queue *(xQueue)*
- **Task two** receives integers from the queue and enabling an LED for that number of milliseconds, there should be a visible pause between each value

You may use a **polling** version of UART, but in that case set the task priorities so that the LED logic doesn't depend on the UART waiting.

### Solution E2

No solution available.

## Exercise 3: Real-time UART handling

Active polling UART by it's nature in a RTOS has unfortunate properties – a process waiting for input will slow down the execution of processes with the same priority and prevent the execution of processes with a lower priority. Implement the functions `uart_transmit` and `uart_receive` without polling. Hints:

- use UART interrupts

- For IPC use FreeRTOS queues

- Polling and blocking is **forbidden** in interrupt handlers! Instead of `xQueueReceive`/`xQueueSend` use `xQueueReceiveFromISR`/`xQueueSendFromISR`

Check your solutions with two tasks:

- **Task one** handles inputs and outputs through UART

- **Task two** flashes an LED

In a correct solution communication through UART should not interfere with the LED flashing

### Solution E3

No solution available.

## Exercise 4: Real-time ADC sharing

Implement an ADC sharing mechanism without polling, to achieve this write a function `uint16_t readADC(uint8_t mux)`, which upon being called returns the ADC measurement on input `mux`. For the measurement time the calling task should be blocked *(change state to Blocked)*. You will also need to implement an ADC interrupt handler cooperating with `readADC` to wake up the blocked task. The function should work with **many** tasks requesting a measurement from many inputs, in that case the measurements should be done in order of request. Implement the synchronization using semaphores and/or mutexes, don't use queues.

The test program should have three tasks running, each one requesting a measurement from a different input at different intervals *(You can use a potentiometer, thermistor, photoresistor or anything else)*. Communicate the results through UART.

### Solution E4

No solution available.

> **Source:**
> This list of problems was assigned as part of the *Embedded Systems* (SW) course in the 2025/26 Winter semester at University of Wrocław by [tilk](https://github.com/tilk)
