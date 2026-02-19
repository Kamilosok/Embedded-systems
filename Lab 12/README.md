# Lab 12

> For both exercises You may use the Atmel provided code at their [website](https://start.atmel.com/#examples/221)

## Exercise 1: PID-regulated temperature control

Use the system from [Bang-bang temperature control](../Lab%2010/README.md#exercise-3-bang-bang-temperature-control). Implement your PID control own or use the provided library for temperature stabilization. The program should allow changing the target temperature through UART. Fine-tune the PID values in any way so there's no stable oscillation (we'll allow a slight muffled oscillation). That means after a short time the temperature should stabilize.

### Solution E1

The solution is in [`exercise1.c`](exercise1.c)

## Exercise 2: PID-regulated motor control

Build the same system from [DC motor measurements](../Lab%2011/README.md#exercise-2-dc-motor-measurements). Use the *Phase and Frequency Correct PWM* mode of Counter 1. Use easurements done when the MOSFET is closed to evaluate the rotational speed of the motor. Implement your own PID control or use the provided library for rotational speed stabilization. The PID controller should manipulate the PWM duty cycle to achieve a stable speed. Target speed is set through a potentiometer measurement, which can be done **after** the motor measurement. Fine-tune the PID values in any way so there's no stable oscillation.

A correctly programmed driver should **resist additional loads** placed on the motor by increasing the opening time of the MOSFET. In effect stopping the motor at low speeds should be harder than without rotational speed stabilization.

### Solution E2

The mostly-correct solution is in [`exercise2.c`](exercise2.c)

> **Source:**
> This list of problems was assigned as part of the *Embedded Systems* (SW) course in the 2025/26 Winter semester at University of Wrocław by [tilk](https://github.com/tilk)
