/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Modified 2026 by Simon BEIMEL (with assistance from Gemini, Google AI)
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/usb_device.h"
#include "pico/audio.h"
#include "pico/audio_i2s.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "lufa/AudioClassCommon.h"
#include "usb_descriptors.h"
#include "usb_audio.h"


struct audio_buffer_pool *producer_pool;

static void core1_worker() {
    sleep_ms(20);  // USB が安定するまで待つ
    audio_i2s_set_enabled(true);
}

int main(void) {
   

    stdout_uart_init();

    //gpio_debug_pins_init();
    puts("USB SOUND CARD");

#ifndef NDEBUG
    for(uint i=0;i<count_of(audio_device_config.as_audio.format.freqs);i++) {
        uint freq = audio_device_config.as_audio.format.freqs[i].Byte1 |
                (audio_device_config.as_audio.format.freqs[i].Byte2 << 8u) |
                (audio_device_config.as_audio.format.freqs[i].Byte3 << 16u);
        assert(freq <= AUDIO_FREQ_MAX);
    }
#endif
    // initialize for 48k we allow changing later
    struct audio_format audio_format_48k = {
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .sample_freq = 48000,
            .channel_count = 2,
    };

    struct audio_buffer_format producer_format = {
            .format = &audio_format_48k,
            .sample_stride = 4
    };

    producer_pool = audio_new_producer_pool(&producer_format, 4, 48); // todo correct size
    bool __unused ok;
    struct audio_i2s_config config = {
            .data_pin = 9,
            .clock_pin_base = 10,
            .dma_channel = 0,
            .pio_sm = 0,
    };

    const struct audio_format *output_format;
    output_format = audio_i2s_setup(&audio_format_48k, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

	gpio_set_drive_strength(9, GPIO_DRIVE_STRENGTH_2MA);  // DATA
	gpio_set_drive_strength(10, GPIO_DRIVE_STRENGTH_2MA); // BCLK
	gpio_set_drive_strength(11, GPIO_DRIVE_STRENGTH_2MA); // LRCLK

    ok = audio_i2s_connect_extra(producer_pool, false, 2, 48, NULL);
    assert(ok);
    usb_sound_card_init();

    multicore_launch_core1(core1_worker);
    // MSD is irq driven
    while (1) __wfi();
}
