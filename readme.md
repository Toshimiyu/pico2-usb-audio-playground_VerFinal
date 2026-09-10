# Pico 2 USB Sound Card Playground (Standalone VerFinal)

This repository contains a standalone version of the original “USB Sound Card” example extracted from the Raspberry Pi Pico Playground.  
It is modified and verified to run on the **Raspberry Pi Pico 2 (RP2350)** as a stable **USB → I2S DDC**.

## Overview
This project turns a Raspberry Pi Pico 2 into a USB-DAC / DDC.  
It receives audio from a host PC using **USB Audio Class 1.0** and outputs **16‑bit PCM stereo** via I2S.

### Features
- **Audio Format:** 16-bit PCM Stereo  
- **Sample Rates:** 44.1kHz / 48kHz  
- **USB Class:** USB Audio Class 1.0  
- **Output:** I2S  
- **Compatibility:** Windows / macOS / Linux  
- **Core:** RP2350 (Cortex‑M33)

### I2S Pin Configuration (Pico 2)
- DATA: GPIO 10
- BCLK: GPIO 9
- LRCLK: GPIO 11  

## Hardware Requirements
- Raspberry Pi Pico 2 (RP2350)
- I2S DAC (PicoAudioPack PCM5100, etc.)
- USB cable

## Build Instructions
This project uses the **Pico SDK 1.5.x generation audio_i2s implementation**,  
which provides stable MCLK/BCLK/LRCLK generation for 16‑bit stereo output.  
Pico SDK 2.x is not compatible with this I2S initialization sequence.

### 1. Install Pico SDK (recommended: 1.5.1)
Set the environment variable:
```bash
setx PICO_SDK_PATH "C:\path\to\pico-sdk"
```
### 2. Configure and build
```bash
mkdir build
cd build
cmake -DPICO_BOARD=pico2 ..
make
```
