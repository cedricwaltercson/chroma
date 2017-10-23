// This file is a part of Chroma.
// Copyright (C) 2017 Matthew Murray
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

#include <memory>
#include <vector>

#include "common/CommonTypes.h"

namespace Emu { class SDLContext; }

namespace Gba {

class Memory;

class Core {
public:
    Core(Emu::SDLContext& context, const std::vector<u16>& rom);
    ~Core();

    void EmulatorLoop();
private:
    Emu::SDLContext& sdl_context;

    std::unique_ptr<Memory> mem;

    bool quit = false;
    bool pause = false;

    void RegisterCallbacks();
};

} // End namespace Gba
