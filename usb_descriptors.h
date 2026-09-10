/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Modified 2026 by Simon BEIMEL (with assistance from Gemini, Google AI)
 */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include "pico/usb_device.h"
#include "lufa/AudioClassCommon.h"

// todo fix these
#define VENDOR_ID   0x2e8au
#define PRODUCT_ID  0xfeddu

#define AUDIO_OUT_ENDPOINT  0x01U
#define AUDIO_IN_ENDPOINT   0x82U

#undef AUDIO_SAMPLE_FREQ
#define AUDIO_SAMPLE_FREQ(frq) (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

#define AUDIO_MAX_PACKET_SIZE(freq) (uint8_t)(((freq + 999) / 1000) * 4)
#define FEATURE_MUTE_CONTROL 1u
#define FEATURE_VOLUME_CONTROL 2u

#define ENDPOINT_FREQ_CONTROL 1u

static struct usb_interface ac_interface;
static struct usb_interface as_op_interface;
static struct usb_endpoint ep_op_out;
static struct usb_endpoint ep_op_sync;

struct audio_device_config {
    struct usb_configuration_descriptor descriptor;
    struct usb_interface_descriptor ac_interface;
    struct __packed {
        USB_Audio_StdDescriptor_Interface_AC_t core;
        USB_Audio_StdDescriptor_InputTerminal_t input_terminal;
        USB_Audio_StdDescriptor_FeatureUnit_t feature_unit;
        USB_Audio_StdDescriptor_OutputTerminal_t output_terminal;
    } ac_audio;
    struct usb_interface_descriptor as_zero_interface;
    struct usb_interface_descriptor as_op_interface;
    struct __packed {
        USB_Audio_StdDescriptor_Interface_AS_t streaming;
        struct __packed {
            USB_Audio_StdDescriptor_Format_t core;
            USB_Audio_SampleFreq_t freqs[2];
        } format;
    } as_audio;
    struct __packed {
        struct usb_endpoint_descriptor_long core;
        USB_Audio_StdDescriptor_StreamEndpoint_Spc_t audio;
    } ep1;
    struct usb_endpoint_descriptor_long ep2;
};

// Externe Deklarationen für die Initialisierung
extern const struct usb_device_descriptor boot_device_descriptor;
extern const struct audio_device_config audio_device_config;
const char *_get_descriptor_string(uint index);

#endif