# Lab 3

## Exercise 1: Buzzer melody

Build and program a system playing a chosen melody (at least half a minute) saved as a series of notes. Our definition of a note is a pair of sound *(C, D, E, F, G,...)* and length *(whole note, halfnote,...)*. Pauses can also be treated as a note. The melody must be saved in program *(flash)* memory and **cannot** be copied to RAM. The sound should be played with a buzzer (without a generator) connected to chosen GPIO pin.

> Hint: PWM can be used effectively here

### Solution E1

The solution is in [`exercise1.c`](exercise1.c)

## Exercise 2: Supply voltage measuring

Measure the microcontroller's supply voltage using ADC. To achieve this, set the reference voltage (Aref) as the supply voltage, whilst configuring the ADC multiplexer to measure an internal $1.1\text{V}$ reference. Compute the supply voltage in the program and output it through UART. It should measure to between $4.5\text{V}$ and $5\text{V}$. Expand the program to enable and disable an LED between measurements. Does it affect the measurements? *(The result might differ with computers and cables!)*

### Solution E2

The solution is in [`exercise2.c`](exercise2.c)

## Exercise 3: Gradual LED control

Write a program that adjusts the brightness of an LED depending on the position of a potentiometer. The brightness change should be implemented by changing the proportions of time the LED is enabled/disabled. **Turn the LED of when ADC is measuring the setting.** Pick the frequency of potentiometer sampling so the reaction time seems instant and no flickering is visible.

> We used a $10\text{k}\Omega$ potentiometer

Remember that a human eye sees light **logarithmically**, so if the brightness is changed linearly the differences in brightness for low settings will be greater than in high settings. Correct this effect using an exponential function: use a bit shift, tabularized values of an exponential function or a combination of the two. **Using `exp()`, floats and doubles is forbidden for this!**

### Solution E3

The solution is in [`exercise3.c`](exercise3.c)

## Exercise 4: Measuring temperature via Thermistor

An NTC thermistor changes it's resistance with temperature according to the formula $R = R_0\space e^{-B(T_0^{-1}-T^{-1})}$, where:

- $R_0$ is the resistance of the thermistor in any chosen temperature $T_0$ (in Kelvin)
- $T$ is the current temperature (in Kelvin)
- B is the thermistor's specific **constant value**

> We used a $2.7\text{k}\Omega$ thermistor

Measure the thermistor's resistance using ADC using a **voltage divider**. Remember that the thermistor's temperature-to-resistance **coefficient isn't linear**. By measurements in known temperatures determine the constant $B$ of your thermistor – the value should land in the $3000 \text{K}$ to $5000 \text{K}$ range. You may use the fact that the thermistor should possess a resistance of about $4.7\text{k}\Omega$ in $25\degree\text{C}$.

Write a program that regularly measures the thermistor's resistance and transmits the computed temperature in $\degree\text{C}$ through UART.

### Solution E4

The solution is in [`exercise4.c`](exercise4.c)

> **Source:**
> This list of problems was assigned as part of the *Embedded Systems* (SW) course in the 2025/26 Winter semester at University of Wrocław by [tilk](https://github.com/tilk)
