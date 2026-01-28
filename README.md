# Embedded-Systems

This repository contains a folders of laboratory lists of exercises (with solutions) prepared as part of my Embedded systems course in 2025/26 Winter semester at UWr

## Hardware

We were working on a slightly modified **ATMega328P** and **ATtiny84A** with other significantly specific parts (like MOSFET's) distinguished in exercises lists by model.

## Structure

Each Lab folder contains the exercise list as the `README.md` file, solutions to exercises with names `exerciseX.c` (except [Lab 9](./Lab%209/)) with X being the name of the exercise and a Makefile, exercises of most Labs can be compiled and instaled using the method below.

## Standard compilation and installation

Go into `Makefile` and change **TARGET** to the program you want to compile.

`make` to compile and link the program, then `make install` to install it to the microcontroller.

`make clean` to clean residue from **TARGET**'s compilation and the program itself (`.elf`, `.hex`, `.map`).

`make screen` to optionally send and receive simple UART messages from the microcontroller.
