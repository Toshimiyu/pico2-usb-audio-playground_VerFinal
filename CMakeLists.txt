# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico2 CACHE STRING "Board type")

# 1. SDK Initialisierung (Standard für Pico Projekte)
include(pico_sdk_import.cmake)

project(PicoMidiSynth C CXX ASM)

# 2. SDK Konfiguration
pico_sdk_init()

add_subdirectory(lib/pico_util_buffer)
add_subdirectory(lib/pico_audio)
add_subdirectory(lib/usb_device)
add_subdirectory(lib/pico_audio_i2s)
add_subdirectory(lib/usb_common) # Meist von usb_device benötigt

# 3. Das Executable definieren
add_executable(usb_sound_card
    usb_sound_card.c
    usb_audio.c
    usb_descriptors.c
    # Falls du die Deskriptoren in eine eigene Datei auslagerst, hier hinzufügen:
)

# 4. Deine spezifischen Definitionen (aus dem Playground übernommen)
target_compile_definitions(usb_sound_card PRIVATE
    AUDIO_FREQ_MAX=48000
    PICO_USBDEV_USE_ZERO_BASED_INTERFACES=1
    PICO_USBDEV_MAX_DESCRIPTOR_SIZE=256
    PICO_USBDEV_ISOCHRONOUS_BUFFER_STRIDE_TYPE=1
    
    # Audio Konfiguration
    PICO_AUDIO_I2S_MONO_OUTPUT=0
    PICO_AUDIO_I2S_MONO_INPUT=0
    
    # Hilfreich für die Fehlersuche im Low-Level Bereich
    PICO_USBDEV_ENABLE_DEBUG_TRACE=1 
)



# 5. Verlinkung der Libraries
# Wir fügen hardware_dma und hardware_pio direkt hinzu, damit du für USBX/PIO bereit bist
target_link_libraries(usb_sound_card 
    pico_stdlib 
    usb_device 
    pico_audio_i2s 
    pico_multicore
    hardware_dma
    hardware_pio
)

# 6. Pico-spezifische Ausgabedateien (uf2, elf, bin)
pico_add_extra_outputs(usb_sound_card)
