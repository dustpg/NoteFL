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

// DEBUG
#ifdef _DEBUG
#include <list>
#endif

// wrapal namespace
namespace WrapAL {
    // Audio Source Clip Real
    struct AudioSourceClipReal final : public IXAudio2VoiceCallback {
    public: // impl for callback
        // Called just before this voice's processing pass begins.
        void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept override;
        // Called just after this voice's processing pass ends.
        void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() noexcept override { }
        // Called when this voice has just finished playing a buffer stream
        void STDMETHODCALLTYPE OnStreamEnd() noexcept override;
        // Called when this voice is about to start processing a new buffer.
        void STDMETHODCALLTYPE OnBufferStart(void * pBufferContext)noexcept override { end_of_buffer = false; }
        // Called when this voice has just finished processing a buffer.
        // The buffer can now be reused or destroyed.
        void STDMETHODCALLTYPE OnBufferEnd(void * pBufferContext) noexcept override;
        // Called when this voice has just reached the end position of a loop.
        void STDMETHODCALLTYPE OnLoopEnd(void * pBufferContext)noexcept override { playing = false; }
        // Called in the event of a critical error during voice processing,
        // such as a failing xAPO or an error from the hardware XMA decoder.
        // The voice may have to be destroyed and re-created to recover from
        // the error.  The callback arguments report which buffer was being
        // processed when the error occurred, and its HRESULT code.
        void STDMETHODCALLTYPE OnVoiceError(void * pBufferContext, HRESULT Error)noexcept override { }
    public:
        // ctor
        AudioSourceClipReal() { end_of_buffer = false; playing = false; };
        // Release
        void Release() noexcept { WrapAL::SafeDestroyVoice(source_voice); WrapAL::SafeRelease(stream); ::free(audio_data); audio_data = nullptr; }
        // buffer the data
        auto ProcessBufferData(XAUDIO2_BUFFER&, bool = true) noexcept->HRESULT;
        // load next data and buffer it for streaming
        auto LoadAndBufferData(int id) noexcept ->HRESULT;
    public:
        // source
        IXAudio2SourceVoice*        source_voice = nullptr;
        // audio stream for streaming
        IALAudioStream*             stream = nullptr;
        // audio data
        uint8_t*                    audio_data = nullptr;
        // audio length in byte
        size_t                      buffer_length = 0;
        // buffer index for streaming
        uint32_t                    buffer_index = 0;
        // flags
        AudioClipFlag               flags;
        // wave format
        WAVEFORMATEX                wave;
        // end of buffer
        std::atomic_bool            end_of_buffer;
        // is playing
        std::atomic_bool            playing;
#if defined _M_IX86

#elif defined _M_X64

#else
#error "Unknown Target Platform"
#endif
    };
    // Audio Source Clip Handle
    class AudioSourceClip;
    // Audio Engine
    class CAudioEngine {
        // audio clip pool
        using ACPool = ObjectPool<AudioSourceClipReal, AudioStreamBucketSize>;
        // friend class
        friend class AudioSourceClip;
        // friend class
        friend class CALDefConfigure;
    public:
        // init
        auto Initialize(IALConfigure* config=nullptr) noexcept ->HRESULT;
        // un-init
        void UnInitialize() noexcept;
        // ctor
        CAudioEngine() = default;
        // dtor
        ~CAudioEngine() noexcept { if(this->configure) this->UnInitialize(); }
    public: // Audio Clip
        // create new clip with audio stream
        // if using streaming audio, do not release the stream, this clip will do it
        auto CreateClip(IALAudioStream*, AudioClipFlag) noexcept ->ClipID;
        // create new clip with file name
        auto CreateClip(AudioFormat, const wchar_t*, AudioClipFlag) noexcept->ClipID;
        // create new clip in memory with move
        auto CreateClipMove(const PCMFormat&, uint8_t*&, size_t, AudioClipFlag) noexcept->ClipID;
        // create new clip in memory
        auto CreateClip(const PCMFormat& format,const uint8_t* src, size_t size, AudioClipFlag) noexcept->ClipID;
    private: // Audio Clip
        // destroy the clip
        bool ac_destroy(ClipID) noexcept;
        // play the clip
        bool ac_play(ClipID) noexcept;
        // pause the clip
        bool ac_pause(ClipID) noexcept;
        // stop the clip
        bool ac_stop(ClipID id) noexcept { this->ac_pause(id); return this->ac_seek(id, 0.0f); }
        // seek the clip with time in sec.
        bool ac_seek(ClipID, float) noexcept;
        // tell the postion of clip in sec.
        auto ac_tell(ClipID) noexcept ->float;
        // get the duration of clip  in sec.
        auto ac_duration(ClipID) noexcept ->float;
        // set or get volume of clip
        auto ac_volume(ClipID, float = -1.f) noexcept->float;
        // set or get ratio of clip
        auto ac_ratio(ClipID id, float ratio) noexcept -> float;
    public: // Master
        // set or get master volume
        auto Master_Volume(float volume=-1.f) noexcept -> float;
    public:
        // XAudio2Create
        static decltype(&::XAudio2Create) XAudio2Create;
    private: // XAudio2 API
        // XAudio2
        HMODULE                     m_hXAudio2 = nullptr;
        // XAudio2 interface
        IXAudio2*                   m_pXAudio2Engine = nullptr;
        // XAudio2 Mastering Voice interface
        IXAudio2MasteringVoice*     m_pMasterVoice = nullptr;
    private: // OpenAL
    private: // DirectSound
    public:
        // now config
        IALConfigure*       const   configure = nullptr;
        // libmpg123.dll handle
        HMODULE             const   libmpg123 = nullptr;
    private: // Common
#ifdef _DEBUG
        // debug list
        std::list<void*>            m_listAC;
#endif
        // allocator for audio clip
        ACPool                      m_acAllocator;
#ifdef WRAPAL_INCLUDE_DEFAULT_CONFIGURE
        // default config
        CALDefConfigure             m_config;
#endif
    public:
        // instance for this
        static CAudioEngine s_instance;
    };
    // marco define
#define AudioEngine (WrapAL::CAudioEngine::s_instance)
#define CheckClip assert(this->index != ClipIDError && "this clip handle had been failed")
    // Audio Clip Handle Class, managed by engine, don't care about the releasing
    class AudioSourceClip {
    public:
        // copy ctor
        AudioSourceClip(const AudioSourceClip& clip)noexcept : index(clip.index) {}
        // move ctor
        AudioSourceClip(AudioSourceClip&& clip)noexcept : index(clip.index) { (clip.index) = ClipIDError; }
        // operator = copy
        auto operator =(const AudioSourceClip& clip) noexcept ->AudioSourceClip& { (this->index) = clip.index; return *this; }
        // operator = move
        auto operator =(AudioSourceClip&& clip) noexcept ->AudioSourceClip& { this->index = clip.index;  (clip.index) = ClipIDError; return *this; }
    public:
        // ctor
        /*explicit*/ AudioSourceClip(ClipID data) noexcept :index(data) {};
        // ctor
        ~AudioSourceClip() noexcept { this->index = ClipIDError; };
        // operatr bool()
        operator bool() const noexcept { return this->index != ClipIDError; }
        // operatr ->
        auto operator ->() const noexcept { CheckClip; return reinterpret_cast<const AudioSourceClipReal*>(index); }
        // destroy this clip, free the memory in engine
        auto Destroy()noexcept { CheckClip; AudioEngine.ac_destroy(this->index);  (this->index) = ClipIDError; }
        // play this clip
        auto Play() const noexcept { CheckClip; return AudioEngine.ac_play(this->index); }
        // stop this clip
        auto Stop() const noexcept { CheckClip; return AudioEngine.ac_stop(this->index); }
        // Pause this clip
        auto Pause() const noexcept { CheckClip; return AudioEngine.ac_pause(this->index); }
        // tell this clip
        auto Tell() const noexcept { CheckClip; return AudioEngine.ac_tell(this->index); }
        // get the duration in sec.
        auto Duration() const noexcept { CheckClip; return AudioEngine.ac_duration(this->index); }
        // set/get this volume
        auto Volume(float volume=-1.f) const noexcept { CheckClip; return AudioEngine.ac_volume(this->index, volume); }
        // set/get this ratio
        auto Ratio(float ratio = -1.f) const noexcept { CheckClip; return AudioEngine.ac_ratio(this->index, ratio); }
        // Seek this clip in sec.
        auto Seek(float time) const noexcept { CheckClip; return AudioEngine.ac_seek(this->index, time); }
    private:
        // the clip id for real clip
        ClipID          index;
    };
    // create new clip with audio stream wrapped function
    // if using streaming audio, do not release the stream, this clip will do it
    static inline auto CreateAudioClip(IALAudioStream* stream, AudioClipFlag flags = Flag_None) noexcept {
        return std::move(AudioSourceClip(AudioEngine.CreateClip(stream, flags)));
    }
    // create new clip with file name wrapped function
    static inline auto CreateAudioClip(AudioFormat f, const wchar_t* p, AudioClipFlag flags = Flag_None) noexcept{
        return std::move(AudioSourceClip(AudioEngine.CreateClip(f, p, flags)));
    }
    // create new clip in memory wrapped function
    // for this, can't be in streaming mode
    static inline auto CreateAudioClip(const PCMFormat& format, const uint8_t* src, size_t size, AudioClipFlag flags = Flag_None) noexcept {
        return std::move(AudioSourceClip(AudioEngine.CreateClip(format, src, size, flags)));
    }
    // create new clip in memory with move wrapped function
    // for this, can't be in streaming mode
    static inline auto CreateAudioClipMove(const PCMFormat& f, uint8_t*& p, size_t l, AudioClipFlag flags = Flag_None) noexcept{
        return std::move(AudioSourceClip(AudioEngine.CreateClipMove(f, p, l, flags)));
    }
    // create new clip with audio stream wrapped function
    // if using streaming audio, do not release the stream, this clip will do it
    static inline auto CreateStreamingAudioClip(IALAudioStream* stream, AudioClipFlag flags = Flag_None) noexcept {
        flags = static_cast<decltype(flags)>(flags | WrapAL::Flag_StreamingReading);
        return std::move(AudioSourceClip(AudioEngine.CreateClip(stream, flags)));
    }
    // create new clip with file name wrapped function
    static inline auto CreateStreamingAudioClip(AudioFormat f, const wchar_t* p, AudioClipFlag flags = Flag_None) noexcept {
        flags = static_cast<decltype(flags)>(flags | WrapAL::Flag_StreamingReading);
        return std::move(AudioSourceClip(AudioEngine.CreateClip(f, p, flags)));
    }
}