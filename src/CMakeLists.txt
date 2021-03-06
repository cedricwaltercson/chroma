# Specify includes relative to the src directory
include_directories(.)

set(SOURCES
    gb/core/GameBoy.cpp
    gb/cpu/CPU.cpp
    gb/cpu/Ops.cpp
    gb/audio/Audio.cpp
    gb/audio/Channel.cpp
    gb/hardware/Joypad.cpp
    gb/hardware/Serial.cpp
    gb/hardware/Timer.cpp
    gb/lcd/LCD.cpp
    gb/lcd/Debug.cpp
    gb/memory/Memory.cpp
    gb/memory/DMA.cpp
    gb/memory/MBC.cpp
    gb/memory/RTC.cpp
    gb/memory/CartridgeHeader.cpp
    gb/logging/Disassembler.cpp
    gb/logging/Logging.cpp

    gba/core/Core.cpp
    gba/memory/Memory.cpp
    gba/memory/CartridgeHeader.cpp
    gba/memory/Save.cpp
    gba/cpu/Cpu.cpp
    gba/cpu/Instruction.cpp
    gba/cpu/ArmOps.cpp
    gba/cpu/ThumbOps.cpp
    gba/cpu/Disassembler.cpp
    gba/cpu/ArmDisasm.cpp
    gba/cpu/ThumbDisasm.cpp
    gba/lcd/Lcd.cpp
    gba/lcd/Bg.cpp
    gba/lcd/Debug.cpp
    gba/hardware/Timer.cpp
    gba/hardware/Dma.cpp
    gba/hardware/Keypad.cpp

    common/Screenshot.cpp

    emu/main.cpp
    emu/SDLContext.cpp
    emu/ParseOptions.cpp
   )

set(HEADERS
    gb/core/GameBoy.h
    gb/core/Enums.h
    gb/cpu/CPU.h
    gb/audio/Audio.h
    gb/audio/Channel.h
    gb/hardware/Joypad.h
    gb/hardware/Serial.h
    gb/hardware/Timer.h
    gb/lcd/LCD.h
    gb/memory/Memory.h
    gb/memory/RTC.h
    gb/memory/CartridgeHeader.h
    gb/logging/Logging.h

    gba/core/Core.h
    gba/core/Enums.h
    gba/memory/Memory.h
    gba/cpu/Cpu.h
    gba/cpu/Instruction.h
    gba/cpu/Disassembler.h
    gba/lcd/Lcd.h
    gba/lcd/Bg.h
    gba/hardware/Timer.h
    gba/hardware/Dma.h
    gba/hardware/Keypad.h
    gba/hardware/Serial.h

    common/CommonTypes.h
    common/CommonFuncs.h
    common/CommonEnums.h
    common/Screenshot.h

    emu/SDLContext.h
    emu/ParseOptions.h
   )

add_executable(chroma ${SOURCES} ${HEADERS})

target_link_libraries(chroma PRIVATE ${SDL2_LIBRARY} fmt::fmt)
