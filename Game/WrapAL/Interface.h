#pragma once
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


// wrapal namespace
namespace WrapAL {
    // 检查
#ifndef WRAPAL_NOVTABLE
#   if (_MSC_VER >= 1100) && defined(__cplusplus)
#       define WRAPAL_NOVTABLE __declspec(novtable)
#   else
#       define WRAPAL_NOVTABLE
#   endif
#endif
    // WrapAL Interface
    class alignas(sizeof(void*)) IALInterface {
    public:
        // release this
        virtual auto Release() noexcept->int32_t = 0;
    };
    // Audio Stream
    class IALAudioStream : public IALInterface {
    public:
        // seek stream in byte, return false if out of range
        virtual auto Seek(uint32_t) noexcept->bool = 0;
        // read stream, return false if end of file/stream
        virtual auto ReadNext(uint32_t, uint8_t*) noexcept->bool = 0;
        // get last error infomation, return false if no error
        virtual auto GetLastErrorInfo(wchar_t info[/*ErrorInfoLength*/])noexcept->bool = 0;
    public:
        // ctor
        IALAudioStream() noexcept = default;
        // dtor
        ~IALAudioStream() noexcept = default;
        // get format
        auto GetFormat() const noexcept -> const PCMFormat& { return this->pcm_format; }
        // get size in byte
        auto GetSizeInByte() const noexcept { return this->total_size; }
    protected:
        // the format of pcm
        PCMFormat               pcm_format = { 0 };
        // total size in byte
        uint32_t                total_size = 0;
    };
    // Audio Configure
    class IALConfigure : public IALInterface {
    public:
        // create audio stream from file
        virtual auto CreateAudioStream(AudioFormat, const wchar_t*) noexcept ->IALAudioStream* = 0;
        // get last error infomation, return false if no error
        virtual auto GetLastErrorInfo(wchar_t info[/*ErrorInfoLength*/])noexcept->bool = 0;
        // output error infomation
        virtual auto OutputError(const wchar_t*)noexcept->void = 0;
        // get the "libmpg123.dll" path
        virtual auto GetLibmpg123_dllPath(wchar_t path[/*MAX_PATH*/])noexcept->void = 0;
    };
}