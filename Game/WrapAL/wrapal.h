#pragma once

/*
WrapAL designed for static link, because of lightweight
enough, It's not recommended to export to dll

Version: 0.0.1
*/

/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/


// M$C
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

// Namespace wrapped std-lib
#include <cstdlib>
// default define
#include <cstddef>
// for using [u]intXX_t
#include <cstdint>
// for using assert
#include <cassert>
// for using FILE
#include <cstdio>
// for using atomic for thread safety
#include <atomic>
// for using replacement new
#include <memory>

// XAudio
#include <xaudio2.h>
//#include "XAudio2_diy.h"

// include the config
#include "wrapalconf.h"

// wrapal namespace
namespace WrapAL {
    // clip
    struct AudioSourceClipReal;
    // Audio Clip ID, managed by engine, don't care about the releasing
    using ClipID = size_t;
    // error clip id
    static constexpr ClipID ClipIDError = 0;
    // pcm format, WAVE_FORMAT_PCM
    struct PCMFormat {
        // make wave format
        auto MakeWave(WAVEFORMATEX& wave) const {
            wave.wFormatTag = WAVE_FORMAT_PCM;
            wave.nChannels = this->nChannels;
            wave.nSamplesPerSec = this->nSamplesPerSec;
            wave.nAvgBytesPerSec = this->nAvgBytesPerSec;
            wave.nBlockAlign = this->nBlockAlign;
            wave.wBitsPerSample = wave.nBlockAlign / wave.nChannels * 8;
            wave.cbSize = 0;
        }
        // sample rate
        uint32_t    nSamplesPerSec;
        // for buffer estimation
        uint32_t    nAvgBytesPerSec;
        // block size of data
        uint16_t    nBlockAlign;
        // number of channels (i.e. mono, stereo...)
        uint16_t    nChannels;
    };
    // audio format
    enum class AudioFormat : uint32_t {
        // stream from memory
        Format_ByteStream = 0,
        // stream from *.wav file
        Format_Wave,
        // stream from *.ogg file
        Format_OggVorbis,
        // stream from *.mp3 or some file, mpg123 will use
        // stderr to display error infomation, be careful
        Format_Mpg123,
        // stream for user defined
        Format_User,
    };
    // Audio Clip Flag
    enum AudioClipFlag : uint32_t {
        // none flag
        Flag_None = 0,
        // all flags
        Flag_All = uint32_t(-1),
        // streaming reading from stream
        Flag_StreamingReading = 1 << 0,
        // loop forever
        Flag_LoopInfinite = 1 << 1,
    };
    // safe release interface
    template<class T>
    auto SafeRelease(T*& pointer) noexcept {
        if (pointer) {
            pointer->Release();
            pointer = nullptr;
        }
    }
    // destroy voice interface
    template<class T>
    auto SafeDestroyVoice(T*& pointer) noexcept {
        if (pointer) {
            pointer->DestroyVoice();
            pointer = nullptr;
        }
    }
    // force cast
    template<typename T> T& force_cast(const T&t) { return const_cast<T&>(t); }
    // API level
    enum class APILevel {
        XAudio2 = 0,
    };
    // now api level
    static const APILevel $WrapALAPILevel = APILevel::XAudio2;
}
#include "Interface.h"
#include "Util.h"
#include "AudioEngine.h"
