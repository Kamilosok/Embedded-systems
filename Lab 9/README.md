# Lab 9

> To install the installer on the Atmega328P call `make arduinoisp` in the main directory, then with it installed to compile install a program to the ATtiny84A use the Makefile in the [ATTiny](./ATTiny/) subdirectory.

## Exercise 1: ATtiny program installing

Connect the [ATtiny84A](https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny24A-44A-84A-DataSheet-DS40002269A.pdf) microcontroller to the system as follows:

| Pin num | Pin signal | Connection                |
|---------|------------|---------------------------|
| 1       | VCC        | Supply voltage (5V)       |
| 14      | GND        | Ground (GND)              |
| 4       | RESET      | PB2 (D10)                 |
| 7       | MOSI(DI)   | PB3 (D11, MOSI of ATmega) |
| 8       | MISO(DO)   | PB4 (D12, MISO of ATmega) |
| 9       | SCK        | PB5 (D13, SCK of ATmega)  |

The connections of pins 7, 8 and 9 should have $2.2\text{k}\Omega$ resistors to avoid flows of high current during any programming/installing errors. Additionally connect a $100\text{nF}$ capacitor as close to pins 1 and 14 as possible. Obviously improper connections **may damage the system!**

After installing the installer program on the ATmega328P connect a $10\text{\muF}$ between the RST pin and ground. *If You're using an electrolytic capactitor make sure to connect the $-$ leg to ground, otherwise the capacitor **may explode***. The capacitor is used to make the ATmega328P not reset when using it as a capacitor, it also prevents it from being programmable, so remove it before programming the ATmega328P.

With a connected setup check the best way to check the correctness of everything is to install a simple LED blinking program to ATtiny84A and *see* if it works.

Repeat [Press delay with interrupts](../Lab%205/README.md#exercise-1-press-delay-with-interrupts) using only ATtiny84A.

### Solution E1

The solution is in [`ATTiny/exercise1.c`](ATTiny/exercise1.c)

## Exercise 2: SPI Inter device communication

Connect an LED to pin PB2 of ATtiny84A and a button to pin PA7 *(If you haven't in the previous exercise)*. Connect a button to a chosen pin of ATmega328P.

Progra the ATtiny84A to work as SPI master regularly sending *(from DO/MISO)* the state of the button and enabling/disabling an LED based on data from SPI slave *(from DI/MOSI)*. *To check the correctness of Your program you may connect the DO/DI pins with a $2.2\text{k}\Omega$ resistor to send the signal to itself*.

After programming ATtiny84A program the ATmega328P to work as SPI slave with the same behavior as ATtiny84A: sending the state of the button and enabling/disabling an LED based on the received signal.

The whole system should work as follows: the LED connected to ATtiny84A should be controlled by the button connected to ATmega328P and the LED connected to ATmega328P should be controlled by the button connected to ATtiny84A.

### Solution E2

The solution for ATmega328P is in [`exercise2Slave.c`](exercise2Slave.c) and for ATtiny84A in [`ATTiny/exercise2Master.c`](ATTiny/exercise2Master.c)

## Exercise 3: $\text{I}^2\text{C}$ Inter device communication

Connect the ATtiny84A same as in [Exercise 1](#exercise-1-attiny-program-installing). Then program it to work as an $\text{I}^2\text{C}$ master generating write operations on address `0x7f` containing one byte of data. Transactions should occur at regular intervals. The data should be a number increasing with every transaction. You may use TWI/ $\text{I}^2\text{C}$ code for [master](https://start.atmel.com/#examples/310) and [slave](https://start.atmel.com/#examples/312) documented with these notes: [master](https://ww1.microchip.com/downloads/en/AppNotes/Atmel-2561-Using-the-USI-Module-as-a-I2C-Master_AP-Note_AVR310.pdf), [slave](https://ww1.microchip.com/downloads/en/Appnotes/Atmel-2560-Using-the-USI-Module-as-a-I2C-Slave_ApplicationNote_AVR312.pdf).

> You need to click the search solution and download the code from Atmel's website

Disconnect the MOSI and SCK pins of both microcontrollers, then make these connections with $10\text{k}\Omega$ pull-up resistors:

| Pin num | Pin signal | Connection                |
|---------|------------|---------------------------|
| 7       | SDA        | PC4 (A4, SDA of ATmega)   |
| 9       | SCL        | PC5 (A5, SCL of ATmega)   |

Program the ATmega328P to work as an $\text{I}^2\text{C}$ slave at address `0x7f` and report this communication through UART.

### Solution E3

No solution available.

> **Source:**
> This list of problems was assigned as part of the *Embedded Systems* (SW) course in the 2025/26 Winter semester at University of Wrocław by [tilk](https://github.com/tilk)
