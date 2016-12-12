// This file is a part of Chroma.
// Copyright (C) 2016 Matthew Murray
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <vector>

#include "common/CommonTypes.h"
#include "common/CommonEnums.h"
#include "core/memory/Memory.h"
#include "core/Timer.h"
#include "core/LCD.h"
#include "core/Serial.h"
#include "core/cpu/CPU.h"
#include "emu/SDL_Utils.h"

namespace Core {

struct CartridgeHeader;

class GameBoy {
public:
    GameBoy(const Console gb_type, const CartridgeHeader& header, Emu::SDLContext& context, std::vector<u8> rom);

    Emu::SDLContext& sdl_context;

    // Game Boy hardware components.
    Timer timer;
    Serial serial;
    LCD lcd;
    Memory mem;
    CPU cpu;

    void EmulatorLoop();
    void HardwareTick(unsigned int cycles);
};

} // End namespace Core
