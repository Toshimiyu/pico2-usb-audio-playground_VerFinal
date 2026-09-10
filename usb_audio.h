/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Modified 2026 by Simon BEIMEL (with assistance from Gemini, Google AI)
 */

#ifndef USB_AUDIO_H
#define USB_AUDIO_H

#include <stdint.h>

// Initialisiert die USB-Hardware und startet den Audio-Stack
void usb_sound_card_init(void);

// Lautstärke-Makros für die Applikationslogik
#define CENTER_VOLUME_INDEX 91
#define ENCODE_DB(x) ((uint16_t)(int16_t)((x)*256))
#define DEFAULT_VOLUME       ENCODE_DB(0)

#endif