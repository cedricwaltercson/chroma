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

#include <array>

#include "common/CommonTypes.h"
#include "common/CommonEnums.h"

namespace Core {

enum class Generator : u8 {Square1=0x01, Square2=0x02, Wave=0x04, Noise=0x08};

class Channel {
public:
    Channel(Generator gen, std::array<u8, 0x10>& wave_ram_ref, u8 NRx0, u8 NRx1, u8 NRx2, u8 NRx3, u8 NRx4)
        : sweep(NRx0)
        , sound_length(NRx1)
        , volume_envelope(NRx2)
        , frequency_lo(NRx3)
        , frequency_hi(NRx4)
        , wave_ram(wave_ram_ref)
        , channel_enabled(gen == Generator::Square1)
        , gen_type(gen)
        , left_enable_mask(static_cast<u8>(gen_type) << 4)
        , right_enable_mask(static_cast<u8>(gen_type)) {}

    u8 sweep;
    u8 sound_length;
    u8 volume_envelope;
    u8 frequency_lo;
    u8 frequency_hi;

    // Wave channel
    u8& channel_on = sweep;
    bool reading_sample = false;
    std::array<u8, 0x10>& wave_ram;

    bool channel_enabled;
    unsigned int wave_pos = 0;

    u8 GenSample() const {
        if (gen_type == Generator::Wave) {
            u8 volume_shift = (WaveVolumeShift()) ? WaveVolumeShift() - 1 : 4;
            return current_sample >> volume_shift;
        } else {
            return DutyCycle((sound_length & 0xC0) >> 6)[wave_pos] * volume;
        }
    }

    u8 EnabledFlag() const {
        if (channel_enabled) {
            return static_cast<u8>(gen_type);
        } else {
            return 0x00;
        }
    }

    bool EnabledLeft(const u8 sound_select) const { return channel_enabled && (sound_select & left_enable_mask); }
    bool EnabledRight(const u8 sound_select) const { return channel_enabled && (sound_select & right_enable_mask); }

    void PowerOn() { wave_pos = 0x00; current_sample = 0x00; }
    void SweepWriteHandler();

    void CheckTrigger(const Console console);
    void TimerTick();
    void LengthCounterTick(const unsigned int frame_seq_counter);
    void EnvelopeTick(const unsigned int frame_seq_counter);
    void SweepTick(const unsigned int frame_seq_counter);
    void ReloadLengthCounter();
    void ClearRegisters(const Console console);
private:
    const Generator gen_type;
    const u8 left_enable_mask;
    const u8 right_enable_mask;

    unsigned int period_timer = 0;

    // Length Counter
    unsigned int length_counter = 0;
    bool prev_length_counter_dec = false;

    // Volume Envelope
    u8 volume = 0x00;
    unsigned int envelope_counter = 0;
    bool prev_envelope_inc = false;
    bool envelope_enabled = false;

    // Frequency Sweep
    u16 shadow_frequency = 0x0000;
    unsigned int sweep_counter = 0;
    bool prev_sweep_inc = false;
    bool sweep_enabled = false;
    bool performed_negative_calculation = false;

    // Wave Sample Buffer
    u8 current_sample = 0x00;
    u8 last_played_sample = 0x00;

    void ReloadPeriod() {
        if (gen_type == Generator::Wave) {
            period_timer = (2048 - (frequency_lo | ((frequency_hi & 0x07) << 8)));
        } else {
            period_timer = (2048 - (frequency_lo | ((frequency_hi & 0x07) << 8))) << 1;
        }
    }

    std::array<unsigned int, 8> DutyCycle(const u8 cycle) const;

    u8 EnvelopePeriod() const { return volume_envelope & 0x07; }
    u8 EnvelopeDirection() const { return (volume_envelope & 0x08) >> 3; }
    u8 EnvelopeInitialVolume() const { return (volume_envelope & 0xF0) >> 4; }

    u16 CalculateSweepFrequency();
    u8 SweepPeriod() const { return (sweep & 0x70) >> 4; }
    u8 SweepDirection() const { return (sweep & 0x08) >> 3; }
    u8 SweepShift() const { return sweep & 0x07; }

    bool WaveChannelOn() const { return (channel_on & 0x80) >> 7; }
    u8 WaveVolumeShift() const { return (volume_envelope & 0x60) >> 5; }
    u8 GetNextSample() const {
        u8 sample_byte = wave_ram[(wave_pos & 0x1E) >> 1];
        return (wave_pos & 0x01) ? (sample_byte & 0x0F) : ((sample_byte & 0xF0) >> 4);
    }

    void CorruptWaveRAM();
};

} // End namespace Core
