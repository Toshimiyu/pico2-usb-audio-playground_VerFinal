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
#include "lufa/AudioClassCommon.h"
#include "usb_descriptors.h"

// todo forget why this is using core 1 for sound: presumably not necessary
// todo noop when muted

CU_REGISTER_DEBUG_PINS(audio_timing)

// ---- select at most one ---
//CU_SELECT_DEBUG_PINS(audio_timing)


static struct {
    uint32_t freq;
    int16_t volume;
    int16_t vol_mul;
    bool mute;
} audio_state = {
        .freq = 44100,
};

extern struct audio_buffer_pool *producer_pool;

static void _as_audio_packet(struct usb_endpoint *ep) {
    assert(ep->current_transfer);
    struct usb_buffer *usb_buffer = usb_current_out_packet_buffer(ep);
    DEBUG_PINS_SET(audio_timing, 1);
    // todo deal with blocking correctly
    struct audio_buffer *audio_buffer = take_audio_buffer(producer_pool, true);
    DEBUG_PINS_CLR(audio_timing, 1);
    assert(!(usb_buffer->data_len & 3u));
    audio_buffer->sample_count = usb_buffer->data_len / 4;
    assert(audio_buffer->sample_count);
    assert(audio_buffer->max_sample_count >= audio_buffer->sample_count);
    uint16_t vol_mul = audio_state.vol_mul;
    int16_t *out = (int16_t *) audio_buffer->buffer->bytes;
    int16_t *in = (int16_t *) usb_buffer->data;
    for (int i = 0; i < audio_buffer->sample_count * 2; i++) {
        out[i] = (int16_t) ((in[i] * vol_mul) >> 15u);
    }

    give_audio_buffer(producer_pool, audio_buffer);
    // keep on truckin'
    usb_grow_transfer(ep->current_transfer, 1);
    usb_packet_done(ep);
}

static void _as_sync_packet(struct usb_endpoint *ep) {
    assert(ep->current_transfer);
    DEBUG_PINS_SET(audio_timing, 2);
    DEBUG_PINS_CLR(audio_timing, 2);
    struct usb_buffer *buffer = usb_current_in_packet_buffer(ep);
    assert(buffer->data_max >= 3);
    buffer->data_len = 3;

    // todo lie thru our teeth for now
    uint feedback = (audio_state.freq << 14u) / 1000u;

    buffer->data[0] = feedback;
    buffer->data[1] = feedback >> 8u;
    buffer->data[2] = feedback >> 16u;

    // keep on truckin'
    usb_grow_transfer(ep->current_transfer, 1);
    usb_packet_done(ep);
}

static const struct usb_transfer_type as_transfer_type = {
        .on_packet = _as_audio_packet,
        .initial_packet_count = 1,
};

static const struct usb_transfer_type as_sync_transfer_type = {
        .on_packet = _as_sync_packet,
        .initial_packet_count = 1,
};

static struct usb_transfer as_transfer;
static struct usb_transfer as_sync_transfer;

static bool do_get_current(struct usb_setup_packet *setup) {
    usb_debug("AUDIO_REQ_GET_CUR\n");

    if ((setup->bmRequestType & USB_REQ_TYPE_RECIPIENT_MASK) == USB_REQ_TYPE_RECIPIENT_INTERFACE) {
        switch (setup->wValue >> 8u) {
            case FEATURE_MUTE_CONTROL: {
                usb_start_tiny_control_in_transfer(audio_state.mute, 1);
                return true;
            }
            case FEATURE_VOLUME_CONTROL: {
                /* Current volume. See UAC Spec 1.0 p.77 */
                usb_start_tiny_control_in_transfer(audio_state.volume, 2);
                return true;
            }
        }
    } else if ((setup->bmRequestType & USB_REQ_TYPE_RECIPIENT_MASK) == USB_REQ_TYPE_RECIPIENT_ENDPOINT) {
        if ((setup->wValue >> 8u) == ENDPOINT_FREQ_CONTROL) {
            /* Current frequency */
            usb_start_tiny_control_in_transfer(audio_state.freq, 3);
            return true;
        }
    }
    return false;
}

// todo this seemed like aood guess, but is not correct
uint16_t db_to_vol[91] = {
        0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002,
        0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0004, 0x0005, 0x0005,
        0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000d, 0x000e,
        0x0010, 0x0012, 0x0014, 0x0017, 0x001a, 0x001d, 0x0020, 0x0024,
        0x0029, 0x002e, 0x0033, 0x003a, 0x0041, 0x0049, 0x0052, 0x005c,
        0x0067, 0x0074, 0x0082, 0x0092, 0x00a4, 0x00b8, 0x00ce, 0x00e7,
        0x0104, 0x0124, 0x0147, 0x016f, 0x019c, 0x01ce, 0x0207, 0x0246,
        0x028d, 0x02dd, 0x0337, 0x039b, 0x040c, 0x048a, 0x0518, 0x05b7,
        0x066a, 0x0732, 0x0813, 0x090f, 0x0a2a, 0x0b68, 0x0ccc, 0x0e5c,
        0x101d, 0x1214, 0x1449, 0x16c3, 0x198a, 0x1ca7, 0x2026, 0x2413,
        0x287a, 0x2d6a, 0x32f5, 0x392c, 0x4026, 0x47fa, 0x50c3, 0x5a9d,
        0x65ac, 0x7214, 0x7fff
};

// actually windows doesn't seem to like this in the middle, so set top range to 0db
#define CENTER_VOLUME_INDEX 91

#define ENCODE_DB(x) ((uint16_t)(int16_t)((x)*256))

#define MIN_VOLUME           ENCODE_DB(-CENTER_VOLUME_INDEX)
#define DEFAULT_VOLUME       ENCODE_DB(0)
#define MAX_VOLUME           ENCODE_DB(count_of(db_to_vol)-CENTER_VOLUME_INDEX)
#define VOLUME_RESOLUTION    ENCODE_DB(1)

static bool do_get_minimum(struct usb_setup_packet *setup) {
    usb_debug("AUDIO_REQ_GET_MIN\n");
    if ((setup->bmRequestType & USB_REQ_TYPE_RECIPIENT_MASK) == USB_REQ_TYPE_RECIPIENT_INTERFACE) {
        switch (setup->wValue >> 8u) {
            case FEATURE_VOLUME_CONTROL: {
                usb_start_tiny_control_in_transfer(MIN_VOLUME, 2);
                return true;
            }
        }
    }
    return false;
}

static bool do_get_maximum(struct usb_setup_packet *setup) {
    usb_debug("AUDIO_REQ_GET_MAX\n");
    if ((setup->bmRequestType & USB_REQ_TYPE_RECIPIENT_MASK) == USB_REQ_TYPE_RECIPIENT_INTERFACE) {
        switch (setup->wValue >> 8u) {
            case FEATURE_VOLUME_CONTROL: {
                usb_start_tiny_control_in_transfer(MAX_VOLUME, 2);
                return true;
            }
        }
    }
    return false;
}

static bool do_get_resolution(struct usb_setup_packet *setup) {
    usb_debug("AUDIO_REQ_GET_RES\n");
    if ((setup->bmRequestType & USB_REQ_TYPE_RECIPIENT_MASK) == USB_REQ_TYPE_RECIPIENT_INTERFACE) {
        switch (setup->wValue >> 8u) {
            case FEATURE_VOLUME_CONTROL: {
                usb_start_tiny_control_in_transfer(VOLUME_RESOLUTION, 2);
                return true;
            }
        }
    }
    return false;
}

static struct audio_control_cmd {
    uint8_t cmd;
    uint8_t type;
    uint8_t cs;
    uint8_t cn;
    uint8_t unit;
    uint8_t len;
} audio_control_cmd_t;

static void _audio_reconfigure() {
    switch (audio_state.freq) {
        case 44100:
        case 48000:
            break;
        default:
            audio_state.freq = 44100;
    }
    // todo hack overwriting const
    ((struct audio_format *) producer_pool->format)->sample_freq = audio_state.freq;
}

static void audio_set_volume(int16_t volume) {
    audio_state.volume = volume;
    // todo interpolate
    volume += CENTER_VOLUME_INDEX * 256;
    if (volume < 0) volume = 0;
    if (volume >= count_of(db_to_vol) * 256) volume = count_of(db_to_vol) * 256 - 1;
    audio_state.vol_mul = db_to_vol[((uint16_t)volume) >> 8u];
//    printf("VOL MUL %04x\n", audio_state.vol_mul);
}

static void audio_cmd_packet(struct usb_endpoint *ep) {
    assert(audio_control_cmd_t.cmd == AUDIO_REQ_SetCurrent);
    struct usb_buffer *buffer = usb_current_out_packet_buffer(ep);
    audio_control_cmd_t.cmd = 0;
    if (buffer->data_len >= audio_control_cmd_t.len) {
        if (audio_control_cmd_t.type == USB_REQ_TYPE_RECIPIENT_INTERFACE) {
            switch (audio_control_cmd_t.cs) {
                case FEATURE_MUTE_CONTROL: {
                    audio_state.mute = buffer->data[0];
                    usb_warn("Set Mute %d\n", buffer->data[0]);
                    break;
                }
                case FEATURE_VOLUME_CONTROL: {
                    audio_set_volume(*(int16_t *) buffer->data);
                    break;
                }
            }

        } else if (audio_control_cmd_t.type == USB_REQ_TYPE_RECIPIENT_ENDPOINT) {
            if (audio_control_cmd_t.cs == ENDPOINT_FREQ_CONTROL) {
                uint32_t new_freq = (*(uint32_t *) buffer->data) & 0x00ffffffu;
                usb_warn("Set freq %d\n", new_freq == 0xffffffu ? -1 : (int) new_freq);

                if (audio_state.freq != new_freq) {
                    audio_state.freq = new_freq;
                    _audio_reconfigure();
                }
            }
        }
    }
    usb_start_empty_control_in_transfer_null_completion();
    // todo is there error handling?
}


static const struct usb_transfer_type _audio_cmd_transfer_type = {
        .on_packet = audio_cmd_packet,
        .initial_packet_count = 1,
};

static bool as_set_alternate(struct usb_interface *interface, uint alt) {
    assert(interface == &as_op_interface);
    usb_warn("SET ALTERNATE %d\n", alt);
    return alt < 2;
}

static bool do_set_current(struct usb_setup_packet *setup) {
#ifndef NDEBUG
    usb_warn("AUDIO_REQ_SET_CUR\n");
#endif

    if (setup->wLength && setup->wLength < 64) {
        audio_control_cmd_t.cmd = AUDIO_REQ_SetCurrent;
        audio_control_cmd_t.type = setup->bmRequestType & USB_REQ_TYPE_RECIPIENT_MASK;
        audio_control_cmd_t.len = (uint8_t) setup->wLength;
        audio_control_cmd_t.unit = setup->wIndex >> 8u;
        audio_control_cmd_t.cs = setup->wValue >> 8u;
        audio_control_cmd_t.cn = (uint8_t) setup->wValue;
        usb_start_control_out_transfer(&_audio_cmd_transfer_type);
        return true;
    }
    return false;
}

static bool ac_setup_request_handler(__unused struct usb_interface *interface, struct usb_setup_packet *setup) {
    setup = __builtin_assume_aligned(setup, 4);
    if (USB_REQ_TYPE_TYPE_CLASS == (setup->bmRequestType & USB_REQ_TYPE_TYPE_MASK)) {
        switch (setup->bRequest) {
            case AUDIO_REQ_SetCurrent:
                return do_set_current(setup);

            case AUDIO_REQ_GetCurrent:
                return do_get_current(setup);

            case AUDIO_REQ_GetMinimum:
                return do_get_minimum(setup);

            case AUDIO_REQ_GetMaximum:
                return do_get_maximum(setup);

            case AUDIO_REQ_GetResolution:
                return do_get_resolution(setup);

            default:
                break;
        }
    }
    return false;
}

bool _as_setup_request_handler(__unused struct usb_endpoint *ep, struct usb_setup_packet *setup) {
    setup = __builtin_assume_aligned(setup, 4);
    if (USB_REQ_TYPE_TYPE_CLASS == (setup->bmRequestType & USB_REQ_TYPE_TYPE_MASK)) {
        switch (setup->bRequest) {
            case AUDIO_REQ_SetCurrent:
                return do_set_current(setup);

            case AUDIO_REQ_GetCurrent:
                return do_get_current(setup);

            case AUDIO_REQ_GetMinimum:
                return do_get_minimum(setup);

            case AUDIO_REQ_GetMaximum:
                return do_get_maximum(setup);

            case AUDIO_REQ_GetResolution:
                return do_get_resolution(setup);

            default:
                break;
        }
    }
    return false;
}

void usb_sound_card_init() {
    //msd_interface.setup_request_handler = msd_setup_request_handler;
    usb_interface_init(&ac_interface, &audio_device_config.ac_interface, NULL, 0, true);
    ac_interface.setup_request_handler = ac_setup_request_handler;

    static struct usb_endpoint *const op_endpoints[] = {
            &ep_op_out, &ep_op_sync
    };
    usb_interface_init(&as_op_interface, &audio_device_config.as_op_interface, op_endpoints, count_of(op_endpoints),
                       true);
    as_op_interface.set_alternate_handler = as_set_alternate;
    ep_op_out.setup_request_handler = _as_setup_request_handler;
    as_transfer.type = &as_transfer_type;
    usb_set_default_transfer(&ep_op_out, &as_transfer);
    as_sync_transfer.type = &as_sync_transfer_type;
    usb_set_default_transfer(&ep_op_sync, &as_sync_transfer);

    static struct usb_interface *const boot_device_interfaces[] = {
            &ac_interface,
            &as_op_interface,
    };
    __unused struct usb_device *device = usb_device_init(&boot_device_descriptor, &audio_device_config.descriptor,
                                                         boot_device_interfaces, count_of(boot_device_interfaces),
                                                         _get_descriptor_string);
    assert(device);
    audio_set_volume(DEFAULT_VOLUME);
    _audio_reconfigure();
//    device->on_configure = _on_configure;
    usb_device_start();
}
