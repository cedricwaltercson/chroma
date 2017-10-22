// This file is a part of Chroma.
// Copyright (C) 2016-2017 Matthew Murray
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

#include <string>
#include <array>
#include <SDL.h>

#include "common/CommonTypes.h"

namespace Emu {

class SDLContext {
public:
    SDLContext(int _width, int _height, unsigned int scale, bool fullscreen);
    ~SDLContext();

    void RenderFrame(const u16* fb_ptr) noexcept;
    void ToggleFullscreen() noexcept;

    void PushBackAudio(const std::array<s16, 1600>& sample_buffer) noexcept;
    void UnpauseAudio() noexcept;
    void PauseAudio() noexcept;

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_AudioDeviceID audio_device;

    const int width;
    const int height;

    int texture_pitch;
    void* texture_pixels;

    static const std::string GetSDLErrorString(const std::string& error_function) {
        return {"SDL_" + error_function + " Error: " + SDL_GetError()};
    }
};

} // End namespace Emu
