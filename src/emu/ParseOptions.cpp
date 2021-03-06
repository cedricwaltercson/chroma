// This file is a part of Chroma.
// Copyright (C) 2016-2018 Matthew Murray
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

#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <fmt/format.h>
#include <sys/stat.h>

#include "gb/memory/CartridgeHeader.h"
#include "gba/memory/Memory.h"
#include "emu/ParseOptions.h"

namespace Emu {

std::vector<std::string> GetTokens(char** begin, char** end) {
    std::vector<std::string> tokens;
    for (; begin != end; ++begin) {
        tokens.emplace_back(*begin);
    }

    return tokens;
}

bool ContainsOption(const std::vector<std::string>& tokens, const std::string& option) {
    return std::find(tokens.cbegin(), tokens.cend(), option) != tokens.cend();
}

std::string GetOptionParam(const std::vector<std::string>& tokens, const std::string& option) {
    auto itr = std::find(tokens.cbegin(), tokens.cend(), option);
    if (itr != tokens.cend() && ++itr != tokens.cend()) {
        return *itr;
    }

    return "";
}

void DisplayHelp() {
    fmt::print("Usage: chroma [options] <path/to/rom>\n\n");
    fmt::print("Options:\n");
    fmt::print("  -h                           display help\n");
    fmt::print("  -m [dmg, cgb, agb]           specify device to emulate\n");
    fmt::print("  -l [trace, regs, timer, lcd] specify log level (default: none)\n");
    fmt::print("  -s [1-15]                    specify resolution scale (default: 2)\n");
    fmt::print("  -f                           activate fullscreen mode\n");
    fmt::print("  --filter [iir, nearest]      choose audio filtering method (default: iir)\n");
    fmt::print("                                   IIR (slow, better quality)\n");
    fmt::print("                                   nearest-neighbour (fast, lesser quality)\n");
    fmt::print("  --multicart                  emulate this game using an MBC1M\n");
}

Gb::Console GetGameBoyType(const std::vector<std::string>& tokens) {
    const std::string gb_string = Emu::GetOptionParam(tokens, "-m");
    if (!gb_string.empty()) {
        if (gb_string == "dmg") {
            return Gb::Console::DMG;
        } else if (gb_string == "cgb") {
            return Gb::Console::CGB;
        } else if (gb_string == "agb") {
            return Gb::Console::AGB;
        } else {
            throw std::invalid_argument("Invalid console specified: " + gb_string);
        }
    } else {
        // If no console specified, the console type will default to the cart type.
        return Gb::Console::Default;
    }
}

LogLevel GetLogLevel(const std::vector<std::string>& tokens) {
    const std::string log_string = Emu::GetOptionParam(tokens, "-l");
    if (!log_string.empty()) {
        if (log_string == "trace") {
            return LogLevel::Trace;
        } else if (log_string == "regs" || log_string == "registers") {
            return LogLevel::Registers;
        } else if (log_string == "timer") {
            return LogLevel::Timer;
        } else if (log_string == "lcd") {
            return LogLevel::LCD;
        } else {
            // Passing the "-l" argument by itself defaults to instruction trace logging.
            return LogLevel::Trace;
        }
    } else {
        // If no log level specified, then no logging by default.
        return LogLevel::None;
    }
}

unsigned int GetPixelScale(const std::vector<std::string>& tokens) {
    const std::string scale_string = Emu::GetOptionParam(tokens, "-s");
    if (!scale_string.empty()) {
        unsigned int scale = std::stoi(scale_string);
        if (scale > 15) {
            throw std::invalid_argument("Invalid scale value specified: " + scale_string);
        }

        return scale;
    } else {
        // If no resolution scale specified, default to 2x native resolution.
        return 2;
    }
}

bool GetFilterEnable(const std::vector<std::string>& tokens) {
    const std::string filter_string = Emu::GetOptionParam(tokens, "--filter");
    if (!filter_string.empty()) {
        if (filter_string == "iir") {
            return true;
        } else if (filter_string == "nearest") {
            return false;
        } else {
            throw std::invalid_argument("Invalid filter method specified: " + filter_string);
        }
    } else {
        // If no filter specified, default to using IIR filter.
        return true;
    }
}

std::size_t GetFileSize(std::ifstream& filestream) {
    filestream.seekg(0, std::ios_base::end);
    auto size = filestream.tellg();
    filestream.seekg(0, std::ios_base::beg);

    return size;
}

Gb::Console CheckRomFile(const std::string& filename) {
    std::ifstream rom_file(filename);
    if (!rom_file) {
        throw std::runtime_error("Error when attempting to open " + filename);
    }

    CheckPathIsRegularFile(filename);

    const auto rom_size = GetFileSize(rom_file);

    if (rom_size > 0x2000000) {
        // 32MB is the largest possible GBA game.
        throw std::runtime_error("Rom size of " + std::to_string(rom_size)
                                 + " bytes is too large to be a GB or GBA game.");
    } else if (rom_size < 0x134) {
        // Provided file is not large enough to contain a DMG Nintendo logo.
        throw std::runtime_error("Rom size of " + std::to_string(rom_size)
                                 + " bytes is too small to be a GB or GBA game.");
    }

    // Read the first 0x134 bytes to check for the Nintendo logos.
    std::vector<u8> rom_header(0x134);
    rom_file.read(reinterpret_cast<char*>(rom_header.data()), rom_header.size());

    if (Gba::Memory::CheckNintendoLogo(rom_header)) {
        return Gb::Console::AGB;
    } else if (Gb::CartridgeHeader::CheckNintendoLogo(Gb::Console::CGB, rom_header)) {
        if (rom_size < 0x8000) {
            // 32KB is the smallest possible GB game.
            throw std::runtime_error("Rom size of " + std::to_string(rom_size)
                                     + " bytes is too small to be a GB game.");
        }

        return Gb::Console::CGB;
    } else {
        throw std::runtime_error("Provided ROM is neither a GB or GBA game. No valid Nintendo logo found.");
    }

    return Gb::Console::CGB;
}

template<typename T>
std::vector<T> LoadRom(const std::string& filename, Gb::Console console) {
    std::ifstream rom_file(filename);
    if (!rom_file) {
        throw std::runtime_error("Error when attempting to open " + filename);
    }

    const auto rom_size = GetFileSize(rom_file);

    // AGB ROMs vary between 1 to 32MB in size. We expand all ROMs to at least 16MB to avoid requiring a bounds
    // check before every low ROM access. This isn't an issue on CGB because only 32KB of ROM is mapped at a time.
    const auto rom_vector_size = (console == Gb::Console::AGB) ? std::max(rom_size, 0x0100'0000ul) : rom_size;

    std::vector<T> rom_contents(rom_vector_size / sizeof(T));
    rom_file.read(reinterpret_cast<char*>(rom_contents.data()), rom_size);

    return rom_contents;
}

template std::vector<u8> LoadRom<u8>(const std::string& filename, Gb::Console console);
template std::vector<u16> LoadRom<u16>(const std::string& filename, Gb::Console console);

std::string SaveGamePath(const std::string& rom_path) {
    std::size_t last_dot = rom_path.rfind('.');

    if (last_dot == std::string::npos) {
        throw std::runtime_error("No file extension found.");
    }

    if (rom_path.substr(last_dot, rom_path.size()) == ".sav") {
        throw std::runtime_error("You tried to run a save file instead of a ROM.");
    }

    return rom_path.substr(0, last_dot) + ".sav";
}

std::vector<u8> LoadSaveGame(const Gb::CartridgeHeader& cart_header, const std::string& save_path) {
    std::vector<u8> save_game;
    if (cart_header.ext_ram_present) {
        save_game = Emu::ReadSaveFile(save_path);

        if (save_game.size() != 0) {
            unsigned int cart_ram_size = cart_header.ram_size;
            if (cart_header.rtc_present) {
                // Account for size of RTC save data, if present at the end of the save file.
                if (save_game.size() % 0x400 == 0x30) {
                    cart_ram_size += 0x30;
                }
            }

            if (cart_ram_size != save_game.size()) {
                throw std::runtime_error("Save game size does not match external RAM size given in cartridge header.");
            }
        } else {
            // No preexisting save game.
            save_game = std::vector<u8>(cart_header.ram_size);
        }
    }

    return save_game;
}

std::vector<u8> ReadSaveFile(const std::string& filename) {
    std::ifstream save_file(filename);
    if (!save_file) {
        // Save file doesn't exist.
        return std::vector<u8>();
    }

    CheckPathIsRegularFile(filename);

    const auto save_size = GetFileSize(save_file);

    if (save_size > 0x20030) {
        throw std::runtime_error("Save game size of " + std::to_string(save_size)
                                 + " bytes is too large to be a Game Boy save.");
    }

    std::vector<u8> save_contents(save_size);
    save_file.read(reinterpret_cast<char*>(save_contents.data()), save_size);

    return save_contents;
}

std::vector<u32> LoadGbaBios() {
    std::string bios_path = "gba_bios.bin";
    std::ifstream bios_file(bios_path);
    for (int i = 0; i < 2; ++i) {
        if (bios_file) {
            break;
        }

        bios_file.close();
        bios_path = "../" + bios_path;
        bios_file.open(bios_path);
    }

    if (!bios_file) {
        throw std::runtime_error("Error when attempting to open gba_bios.bin");
    }

    CheckPathIsRegularFile(bios_path);

    const auto bios_size = GetFileSize(bios_file);

    if (bios_size != 0x4000) {
        throw std::runtime_error("GBA BIOS must be 16KB. Provided file is " + std::to_string(bios_size) + " bytes.");
    }

    std::vector<u32> bios_contents(bios_size / sizeof(u32));
    bios_file.read(reinterpret_cast<char*>(bios_contents.data()), bios_size);

    return bios_contents;
}

void CheckPathIsRegularFile(const std::string& filename) {
    // Check that the path points to a regular file.
    struct stat stat_info;
    if (stat(filename.c_str(), &stat_info) == 0) {
        if (stat_info.st_mode & S_IFDIR) {
            throw std::runtime_error("Provided path is a directory: " + filename);
        } else if (!(stat_info.st_mode & S_IFREG)) {
            throw std::runtime_error("Provided path is not a regular file: " + filename);
        }
    }
}

} // End namespace Emu
