# Pico 2 USB Sound Card Playground (Standalone)

This repository contains the original "USB Sound Card" example, extracted from the Raspberry Pi Pico Playground and modified to be a **standalone** project. It is specifically configured and tested for the **RP2350** (Raspberry Pi Pico 2).

## Overview
This project demonstrates how to turn a Raspberry Pi Pico 2 into a USB-DAC. It uses the native Pico SDK audio and USB libraries to receive audio data from a host PC and output it via I2S.

- **Core:** RP2350 (Cortex-M33)
- **Audio Format:** 16-bit PCM Stereo @ 44.1kHz / 48kHz
- **Output:** I2S (Data: GPIO 18, Clock Base: GPIO 16)
- **USB Stack:** Pico SDK Native `pico_usb_device`

## Hardware Requirements
- **Raspberry Pi Pico 2** (RP2350)
- An I2S DAC (e.g., PCM5102 or similar)
- USB Cable

## How to Build
1. Make sure you have the **Pico SDK (version 2.0.0 or higher)** installed and the `PICO_SDK_PATH` environment variable set.
2. Create a build directory:
   ```bash
   mkdir build
   cd build

3. Initialize CMake for the RP2350:
   Bash
   cmake -DPICO_BOARD=pico2 ..
   Build the project:

4. Bash
   make

## Project Structure
usb_sound_card.c: The main entry point and hardware initialization.

usb_audio.c / .h: Decoupled USB Audio Class (UAC) handling logic.

usb_descriptors.c / .h: USB device and configuration descriptors.

## Notes
* This is a "Playground" example. It serves as a stable 16-bit baseline before moving towards more advanced implementations (like 32-bit high-res support or alternative USB stacks).
* Modified 2026 by Simon BEIMEL (with assistance from Gemini, Google AI)

## License
BSD-3-Clause (consistent with Raspberry Pi Pico SDK examples)
