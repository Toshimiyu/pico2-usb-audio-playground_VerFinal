/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Modified 2026 by Simon BEIMEL (with assistance from Gemini, Google AI)
 */

#include "usb_descriptors.h"
#include "pico/usb_device.h"
#include "lufa/AudioClassCommon.h"

static char *descriptor_strings[] =
        {
                "Raspberry Pi",
                "Pico Examples Sound Card",
                "0123456789AB"
        };


const struct audio_device_config audio_device_config = {
        .descriptor = {
                .bLength             = sizeof(audio_device_config.descriptor),
                .bDescriptorType     = DTYPE_Configuration,
                .wTotalLength        = sizeof(audio_device_config),
                .bNumInterfaces      = 2,
                .bConfigurationValue = 0x01,
                .iConfiguration      = 0x00,
                .bmAttributes        = 0x80,
                .bMaxPower           = 0x32,
        },
        .ac_interface = {
                .bLength            = sizeof(audio_device_config.ac_interface),
                .bDescriptorType    = DTYPE_Interface,
                .bInterfaceNumber   = 0x00,
                .bAlternateSetting  = 0x00,
                .bNumEndpoints      = 0x00,
                .bInterfaceClass    = AUDIO_CSCP_AudioClass,
                .bInterfaceSubClass = AUDIO_CSCP_ControlSubclass,
                .bInterfaceProtocol = AUDIO_CSCP_ControlProtocol,
                .iInterface         = 0x00,
        },
        .ac_audio = {
                .core = {
                        .bLength = sizeof(audio_device_config.ac_audio.core),
                        .bDescriptorType = AUDIO_DTYPE_CSInterface,
                        .bDescriptorSubtype = AUDIO_DSUBTYPE_CSInterface_Header,
                        .bcdADC = VERSION_BCD(1, 0, 0),
                        .wTotalLength = sizeof(audio_device_config.ac_audio),
                        .bInCollection = 1,
                        .bInterfaceNumbers = 1,
                },
                .input_terminal = {
                        .bLength = sizeof(audio_device_config.ac_audio.input_terminal),
                        .bDescriptorType = AUDIO_DTYPE_CSInterface,
                        .bDescriptorSubtype = AUDIO_DSUBTYPE_CSInterface_InputTerminal,
                        .bTerminalID = 1,
                        .wTerminalType = AUDIO_TERMINAL_STREAMING,
                        .bAssocTerminal = 0,
                        .bNrChannels = 2,
                        .wChannelConfig = AUDIO_CHANNEL_LEFT_FRONT | AUDIO_CHANNEL_RIGHT_FRONT,
                        .iChannelNames = 0,
                        .iTerminal = 0,
                },
                .feature_unit = {
                        .bLength = sizeof(audio_device_config.ac_audio.feature_unit),
                        .bDescriptorType = AUDIO_DTYPE_CSInterface,
                        .bDescriptorSubtype = AUDIO_DSUBTYPE_CSInterface_Feature,
                        .bUnitID = 2,
                        .bSourceID = 1,
                        .bControlSize = 1,
                        .bmaControls = {AUDIO_FEATURE_MUTE | AUDIO_FEATURE_VOLUME, 0, 0},
                        .iFeature = 0,
                },
                .output_terminal = {
                        .bLength = sizeof(audio_device_config.ac_audio.output_terminal),
                        .bDescriptorType = AUDIO_DTYPE_CSInterface,
                        .bDescriptorSubtype = AUDIO_DSUBTYPE_CSInterface_OutputTerminal,
                        .bTerminalID = 3,
                        .wTerminalType = AUDIO_TERMINAL_OUT_SPEAKER,
                        .bAssocTerminal = 0,
                        .bSourceID = 2,
                        .iTerminal = 0,
                },
        },
        .as_zero_interface = {
                .bLength            = sizeof(audio_device_config.as_zero_interface),
                .bDescriptorType    = DTYPE_Interface,
                .bInterfaceNumber   = 0x01,
                .bAlternateSetting  = 0x00,
                .bNumEndpoints      = 0x00,
                .bInterfaceClass    = AUDIO_CSCP_AudioClass,
                .bInterfaceSubClass = AUDIO_CSCP_AudioStreamingSubclass,
                .bInterfaceProtocol = AUDIO_CSCP_ControlProtocol,
                .iInterface         = 0x00,
        },
        .as_op_interface = {
                .bLength            = sizeof(audio_device_config.as_op_interface),
                .bDescriptorType    = DTYPE_Interface,
                .bInterfaceNumber   = 0x01,
                .bAlternateSetting  = 0x01,
                .bNumEndpoints      = 0x02,
                .bInterfaceClass    = AUDIO_CSCP_AudioClass,
                .bInterfaceSubClass = AUDIO_CSCP_AudioStreamingSubclass,
                .bInterfaceProtocol = AUDIO_CSCP_ControlProtocol,
                .iInterface         = 0x00,
        },
        .as_audio = {
                .streaming = {
                        .bLength = sizeof(audio_device_config.as_audio.streaming),
                        .bDescriptorType = AUDIO_DTYPE_CSInterface,
                        .bDescriptorSubtype = AUDIO_DSUBTYPE_CSInterface_General,
                        .bTerminalLink = 1,
                        .bDelay = 1,
                        .wFormatTag = 1, // PCM
                },
                .format = {
                        .core = {
                                .bLength = sizeof(audio_device_config.as_audio.format),
                                .bDescriptorType = AUDIO_DTYPE_CSInterface,
                                .bDescriptorSubtype = AUDIO_DSUBTYPE_CSInterface_FormatType,
                                .bFormatType = 1,
                                .bNrChannels = 2,
                                .bSubFrameSize = 2,
                                .bBitResolution = 16,
                                .bSampleFrequencyType = count_of(audio_device_config.as_audio.format.freqs),
                        },
                        .freqs = {
                                AUDIO_SAMPLE_FREQ(44100),
                                AUDIO_SAMPLE_FREQ(48000)
                        },
                },
        },
        .ep1 = {
                .core = {
                        .bLength          = sizeof(audio_device_config.ep1.core),
                        .bDescriptorType  = DTYPE_Endpoint,
                        .bEndpointAddress = AUDIO_OUT_ENDPOINT,
                        .bmAttributes     = 5,
                        .wMaxPacketSize   = AUDIO_MAX_PACKET_SIZE(AUDIO_FREQ_MAX),
                        .bInterval        = 1,
                        .bRefresh         = 0,
                        .bSyncAddr        = AUDIO_IN_ENDPOINT,
                },
                .audio = {
                        .bLength = sizeof(audio_device_config.ep1.audio),
                        .bDescriptorType = AUDIO_DTYPE_CSEndpoint,
                        .bDescriptorSubtype = AUDIO_DSUBTYPE_CSEndpoint_General,
                        .bmAttributes = 1,
                        .bLockDelayUnits = 0,
                        .wLockDelay = 0,
                }
        },
        .ep2 = {
                .bLength          = sizeof(audio_device_config.ep2),
                .bDescriptorType  = 0x05,
                .bEndpointAddress = AUDIO_IN_ENDPOINT,
                .bmAttributes     = 0x11,
                .wMaxPacketSize   = 3,
                .bInterval        = 0x01,
                .bRefresh         = 2,
                .bSyncAddr        = 0,
        },
};

static struct usb_interface ac_interface;
static struct usb_interface as_op_interface;
static struct usb_endpoint ep_op_out, ep_op_sync;

const struct usb_device_descriptor boot_device_descriptor = {
        .bLength            = 18,
        .bDescriptorType    = 0x01,
        .bcdUSB             = 0x0110,
        .bDeviceClass       = 0x00,
        .bDeviceSubClass    = 0x00,
        .bDeviceProtocol    = 0x00,
        .bMaxPacketSize0    = 0x40,
        .idVendor           = VENDOR_ID,
        .idProduct          = PRODUCT_ID,
        .bcdDevice          = 0x0200,
        .iManufacturer      = 0x01,
        .iProduct           = 0x02,
        .iSerialNumber      = 0x03,
        .bNumConfigurations = 0x01,
};

const char *_get_descriptor_string(uint index) {
    if (index <= count_of(descriptor_strings)) {
        return descriptor_strings[index - 1];
    } else {
        return "";
    }
}
