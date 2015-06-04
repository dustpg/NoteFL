#pragma once
#ifndef _XAUDIO2_DIY_H_
#define _XAUDIO2_DIY_H_
// mingw-w64(MinGW-W64-builds-4.2.0) no "XAudio.h", so, do it myself

#include <objbase.h>            // Windows COM declarations
#include <sal.h>                // Markers for documenting API semantics

/**************************************************************************
*
*  WAVEFORMATEXTENSIBLE: Extended version of WAVEFORMATEX that should be
*  used as a basis for all new audio formats.  The format tag is replaced
*  with a GUID, allowing new formats to be defined without registering a
*  format tag with Microsoft.  There are also new fields that can be used
*  to specify the spatial positions for each channel and the bit packing
*  used for wide samples (e.g. 24-bit PCM samples in 32-bit containers).
*
***************************************************************************/

#ifndef _WAVEFORMATEXTENSIBLE_

#define _WAVEFORMATEXTENSIBLE_
typedef struct
{
    WAVEFORMATEX Format;          // Base WAVEFORMATEX data
    union
    {
        WORD wValidBitsPerSample; // Valid bits in each sample container
        WORD wSamplesPerBlock;    // Samples per block of audio data; valid
                                  // if wBitsPerSample=0 (but rarely used).
        WORD wReserved;           // Zero if neither case above applies.
    } Samples;
    DWORD dwChannelMask;          // Positions of the audio channels
    GUID SubFormat;               // Format identifier GUID
} WAVEFORMATEXTENSIBLE;

#endif

typedef WAVEFORMATEXTENSIBLE *PWAVEFORMATEXTENSIBLE, *LPWAVEFORMATEXTENSIBLE;
typedef const WAVEFORMATEXTENSIBLE *PCWAVEFORMATEXTENSIBLE, *LPCWAVEFORMATEXTENSIBLE;

// All structures defined in this file use tight field packing
#pragma pack(push, 1)


/**************************************************************************
*
* XAudio2 constants, flags and error codes.
*
**************************************************************************/

// Numeric boundary values
#define XAUDIO2_MAX_BUFFER_BYTES        0x80000000    // Maximum bytes allowed in a source buffer
#define XAUDIO2_MAX_QUEUED_BUFFERS      64            // Maximum buffers allowed in a voice queue
#define XAUDIO2_MAX_BUFFERS_SYSTEM      2             // Maximum buffers allowed for system threads (Xbox 360 only)
#define XAUDIO2_MAX_AUDIO_CHANNELS      64            // Maximum channels in an audio stream
#define XAUDIO2_MIN_SAMPLE_RATE         1000          // Minimum audio sample rate supported
#define XAUDIO2_MAX_SAMPLE_RATE         200000        // Maximum audio sample rate supported
#define XAUDIO2_MAX_VOLUME_LEVEL        16777216.0f   // Maximum acceptable volume level (2^24)
#define XAUDIO2_MIN_FREQ_RATIO          (1/1024.0f)   // Minimum SetFrequencyRatio argument
#define XAUDIO2_MAX_FREQ_RATIO          1024.0f       // Maximum MaxFrequencyRatio argument
#define XAUDIO2_DEFAULT_FREQ_RATIO      2.0f          // Default MaxFrequencyRatio argument
#define XAUDIO2_MAX_FILTER_ONEOVERQ     1.5f          // Maximum XAUDIO2_FILTER_PARAMETERS.OneOverQ
#define XAUDIO2_MAX_FILTER_FREQUENCY    1.0f          // Maximum XAUDIO2_FILTER_PARAMETERS.Frequency
#define XAUDIO2_MAX_LOOP_COUNT          254           // Maximum non-infinite XAUDIO2_BUFFER.LoopCount
#define XAUDIO2_MAX_INSTANCES           8             // Maximum simultaneous XAudio2 objects on Xbox 360

// For XMA voices on Xbox 360 there is an additional restriction on the MaxFrequencyRatio
// argument and the voice's sample rate: the product of these numbers cannot exceed 600000
// for one-channel voices or 300000 for voices with more than one channel.
#define XAUDIO2_MAX_RATIO_TIMES_RATE_XMA_MONO         600000
#define XAUDIO2_MAX_RATIO_TIMES_RATE_XMA_MULTICHANNEL 300000

// Numeric values with special meanings
#define XAUDIO2_COMMIT_NOW              0             // Used as an OperationSet argument
#define XAUDIO2_COMMIT_ALL              0             // Used in IXAudio2::CommitChanges
#define XAUDIO2_INVALID_OPSET           (UINT32)(-1)  // Not allowed for OperationSet arguments
#define XAUDIO2_NO_LOOP_REGION          0             // Used in XAUDIO2_BUFFER.LoopCount
#define XAUDIO2_LOOP_INFINITE           255           // Used in XAUDIO2_BUFFER.LoopCount
#define XAUDIO2_DEFAULT_CHANNELS        0             // Used in CreateMasteringVoice
#define XAUDIO2_DEFAULT_SAMPLERATE      0             // Used in CreateMasteringVoice

// Flags
#define XAUDIO2_VOICE_NOPITCH           0x0002        // Used in IXAudio2::CreateSourceVoice
#define XAUDIO2_VOICE_NOSRC             0x0004        // Used in IXAudio2::CreateSourceVoice
#define XAUDIO2_VOICE_USEFILTER         0x0008        // Used in IXAudio2::CreateSource/SubmixVoice
#define XAUDIO2_PLAY_TAILS              0x0020        // Used in IXAudio2SourceVoice::Stop
#define XAUDIO2_END_OF_STREAM           0x0040        // Used in XAUDIO2_BUFFER.Flags
#define XAUDIO2_SEND_USEFILTER          0x0080        // Used in XAUDIO2_SEND_DESCRIPTOR.Flags
#define XAUDIO2_VOICE_NOSAMPLESPLAYED   0x0100        // Used in IXAudio2SourceVoice::GetState

// Default parameters for the built-in filter
#define XAUDIO2_DEFAULT_FILTER_TYPE     LowPassFilter
#define XAUDIO2_DEFAULT_FILTER_FREQUENCY XAUDIO2_MAX_FILTER_FREQUENCY
#define XAUDIO2_DEFAULT_FILTER_ONEOVERQ 1.0f

// Internal XAudio2 constants
#define XAUDIO2_QUANTUM_NUMERATOR   1                 // On Windows, XAudio2 processes audio
#define XAUDIO2_QUANTUM_DENOMINATOR 100               //  in 10ms chunks (= 1/100 seconds)
#define XAUDIO2_QUANTUM_MS (1000.0f * XAUDIO2_QUANTUM_NUMERATOR / XAUDIO2_QUANTUM_DENOMINATOR)

// XAudio2 error codes
#define FACILITY_XAUDIO2 0x896
#define XAUDIO2_E_INVALID_CALL          0x88960001    // An API call or one of its arguments was illegal
#define XAUDIO2_E_XMA_DECODER_ERROR     0x88960002    // The XMA hardware suffered an unrecoverable error
#define XAUDIO2_E_XAPO_CREATION_FAILED  0x88960003    // XAudio2 failed to initialize an XAPO effect
#define XAUDIO2_E_DEVICE_INVALIDATED    0x88960004    // An audio device became unusable (unplugged, etc)


/**************************************************************************
*
* Forward declarations for the XAudio2 interfaces.
*
**************************************************************************/

#ifdef __cplusplus
#define FWD_DECLARE(x) interface x
#else
#define FWD_DECLARE(x) typedef interface x x
#endif

FWD_DECLARE(IXAudio2);
FWD_DECLARE(IXAudio2Voice);
FWD_DECLARE(IXAudio2SourceVoice);
FWD_DECLARE(IXAudio2SubmixVoice);
FWD_DECLARE(IXAudio2MasteringVoice);
FWD_DECLARE(IXAudio2EngineCallback);
FWD_DECLARE(IXAudio2VoiceCallback);


/**************************************************************************
*
* XAudio2 structures and enumerations.
*
**************************************************************************/

// Used in XAudio2Create, specifies which CPU(s) to use.
typedef UINT32 XAUDIO2_PROCESSOR;
#define Processor1  0x00000001
#define Processor2  0x00000002
#define Processor3  0x00000004
#define Processor4  0x00000008
#define Processor5  0x00000010
#define Processor6  0x00000020
#define Processor7  0x00000040
#define Processor8  0x00000080
#define Processor9  0x00000100
#define Processor10 0x00000200
#define Processor11 0x00000400
#define Processor12 0x00000800
#define Processor13 0x00001000
#define Processor14 0x00002000
#define Processor15 0x00004000
#define Processor16 0x00008000
#define Processor17 0x00010000
#define Processor18 0x00020000
#define Processor19 0x00040000
#define Processor20 0x00080000
#define Processor21 0x00100000
#define Processor22 0x00200000
#define Processor23 0x00400000
#define Processor24 0x00800000
#define Processor25 0x01000000
#define Processor26 0x02000000
#define Processor27 0x04000000
#define Processor28 0x08000000
#define Processor29 0x10000000
#define Processor30 0x20000000
#define Processor31 0x40000000
#define Processor32 0x80000000
#define XAUDIO2_ANY_PROCESSOR 0xffffffff
#define XAUDIO2_DEFAULT_PROCESSOR Processor1

// Returned by IXAudio2Voice::GetVoiceDetails
typedef struct XAUDIO2_VOICE_DETAILS
{
    UINT32 CreationFlags;               // Flags the voice was created with.
    UINT32 ActiveFlags;                 // Flags currently active.
    UINT32 InputChannels;               // Channels in the voice's input audio.
    UINT32 InputSampleRate;             // Sample rate of the voice's input audio.
} XAUDIO2_VOICE_DETAILS;

// Used in XAUDIO2_VOICE_SENDS below
typedef struct XAUDIO2_SEND_DESCRIPTOR
{
    UINT32 Flags;                       // Either 0 or XAUDIO2_SEND_USEFILTER.
    IXAudio2Voice* pOutputVoice;        // This send's destination voice.
} XAUDIO2_SEND_DESCRIPTOR;

// Used in the voice creation functions and in IXAudio2Voice::SetOutputVoices
typedef struct XAUDIO2_VOICE_SENDS
{
    UINT32 SendCount;                   // Number of sends from this voice.
    XAUDIO2_SEND_DESCRIPTOR* pSends;    // Array of SendCount send descriptors.
} XAUDIO2_VOICE_SENDS;

// Used in XAUDIO2_EFFECT_CHAIN below
typedef struct XAUDIO2_EFFECT_DESCRIPTOR
{
    IUnknown* pEffect;                  // Pointer to the effect object's IUnknown interface.
    BOOL InitialState;                  // TRUE if the effect should begin in the enabled state.
    UINT32 OutputChannels;              // How many output channels the effect should produce.
} XAUDIO2_EFFECT_DESCRIPTOR;

// Used in the voice creation functions and in IXAudio2Voice::SetEffectChain
typedef struct XAUDIO2_EFFECT_CHAIN
{
    UINT32 EffectCount;                 // Number of effects in this voice's effect chain.
    XAUDIO2_EFFECT_DESCRIPTOR* pEffectDescriptors; // Array of effect descriptors.
} XAUDIO2_EFFECT_CHAIN;

// Used in XAUDIO2_FILTER_PARAMETERS below
typedef enum XAUDIO2_FILTER_TYPE
{
    LowPassFilter,                      // Attenuates frequencies above the cutoff frequency (state-variable filter).
    BandPassFilter,                     // Attenuates frequencies outside a given range      (state-variable filter).
    HighPassFilter,                     // Attenuates frequencies below the cutoff frequency (state-variable filter).
    NotchFilter,                        // Attenuates frequencies inside a given range       (state-variable filter).
    LowPassOnePoleFilter,               // Attenuates frequencies above the cutoff frequency (one-pole filter, XAUDIO2_FILTER_PARAMETERS.OneOverQ has no effect)
    HighPassOnePoleFilter               // Attenuates frequencies below the cutoff frequency (one-pole filter, XAUDIO2_FILTER_PARAMETERS.OneOverQ has no effect)
} XAUDIO2_FILTER_TYPE;

// Used in IXAudio2Voice::Set/GetFilterParameters and Set/GetOutputFilterParameters
typedef struct XAUDIO2_FILTER_PARAMETERS
{
    XAUDIO2_FILTER_TYPE Type;           // Filter type.
    float Frequency;                    // Filter coefficient.
                                        //  must be >= 0 and <= XAUDIO2_MAX_FILTER_FREQUENCY
                                        //  See XAudio2CutoffFrequencyToRadians() for state-variable filter types and
                                        //  XAudio2CutoffFrequencyToOnePoleCoefficient() for one-pole filter types.
    float OneOverQ;                     // Reciprocal of the filter's quality factor Q;
                                        //  must be > 0 and <= XAUDIO2_MAX_FILTER_ONEOVERQ.
                                        //  Has no effect for one-pole filters.
} XAUDIO2_FILTER_PARAMETERS;

// Used in IXAudio2SourceVoice::SubmitSourceBuffer
typedef struct XAUDIO2_BUFFER
{
    UINT32 Flags;                       // Either 0 or XAUDIO2_END_OF_STREAM.
    UINT32 AudioBytes;                  // Size of the audio data buffer in bytes.
    const BYTE* pAudioData;             // Pointer to the audio data buffer.
    UINT32 PlayBegin;                   // First sample in this buffer to be played.
    UINT32 PlayLength;                  // Length of the region to be played in samples,
                                        //  or 0 to play the whole buffer.
    UINT32 LoopBegin;                   // First sample of the region to be looped.
    UINT32 LoopLength;                  // Length of the desired loop region in samples,
                                        //  or 0 to loop the entire buffer.
    UINT32 LoopCount;                   // Number of times to repeat the loop region,
                                        //  or XAUDIO2_LOOP_INFINITE to loop forever.
    void* pContext;                     // Context value to be passed back in callbacks.
} XAUDIO2_BUFFER;

// Used in IXAudio2SourceVoice::SubmitSourceBuffer when submitting XWMA data.
// NOTE: If an XWMA sound is submitted in more than one buffer, each buffer's
// pDecodedPacketCumulativeBytes[PacketCount-1] value must be subtracted from
// all the entries in the next buffer's pDecodedPacketCumulativeBytes array.
// And whether a sound is submitted in more than one buffer or not, the final
// buffer of the sound should use the XAUDIO2_END_OF_STREAM flag, or else the
// client must call IXAudio2SourceVoice::Discontinuity after submitting it.
typedef struct XAUDIO2_BUFFER_WMA
{
    const UINT32* pDecodedPacketCumulativeBytes; // Decoded packet's cumulative size array.
                                                 //  Each element is the number of bytes accumulated
                                                 //  when the corresponding XWMA packet is decoded in
                                                 //  order.  The array must have PacketCount elements.
    UINT32 PacketCount;                          // Number of XWMA packets submitted. Must be >= 1 and
                                                 //  divide evenly into XAUDIO2_BUFFER.AudioBytes.
} XAUDIO2_BUFFER_WMA;

// Returned by IXAudio2SourceVoice::GetState
typedef struct XAUDIO2_VOICE_STATE
{
    void* pCurrentBufferContext;        // The pContext value provided in the XAUDIO2_BUFFER
                                        //  that is currently being processed, or NULL if
                                        //  there are no buffers in the queue.
    UINT32 BuffersQueued;               // Number of buffers currently queued on the voice
                                        //  (including the one that is being processed).
    UINT64 SamplesPlayed;               // Total number of samples produced by the voice since
                                        //  it began processing the current audio stream.
                                        //  If XAUDIO2_VOICE_NOSAMPLESPLAYED is specified
                                        //  in the call to IXAudio2SourceVoice::GetState,
                                        //  this member will not be calculated, saving CPU.
} XAUDIO2_VOICE_STATE;

// Returned by IXAudio2::GetPerformanceData
typedef struct XAUDIO2_PERFORMANCE_DATA
{
    // CPU usage information
    UINT64 AudioCyclesSinceLastQuery;   // CPU cycles spent on audio processing since the
                                        //  last call to StartEngine or GetPerformanceData.
    UINT64 TotalCyclesSinceLastQuery;   // Total CPU cycles elapsed since the last call
                                        //  (only counts the CPU XAudio2 is running on).
    UINT32 MinimumCyclesPerQuantum;     // Fewest CPU cycles spent processing any one
                                        //  audio quantum since the last call.
    UINT32 MaximumCyclesPerQuantum;     // Most CPU cycles spent processing any one
                                        //  audio quantum since the last call.

                                        // Memory usage information
    UINT32 MemoryUsageInBytes;          // Total heap space currently in use.

                                        // Audio latency and glitching information
    UINT32 CurrentLatencyInSamples;     // Minimum delay from when a sample is read from a
                                        //  source buffer to when it reaches the speakers.
    UINT32 GlitchesSinceEngineStarted;  // Audio dropouts since the engine was started.

                                        // Data about XAudio2's current workload
    UINT32 ActiveSourceVoiceCount;      // Source voices currently playing.
    UINT32 TotalSourceVoiceCount;       // Source voices currently existing.
    UINT32 ActiveSubmixVoiceCount;      // Submix voices currently playing/existing.

    UINT32 ActiveResamplerCount;        // Resample xAPOs currently active.
    UINT32 ActiveMatrixMixCount;        // MatrixMix xAPOs currently active.

                                        // Usage of the hardware XMA decoder (Xbox 360 only)
    UINT32 ActiveXmaSourceVoices;       // Number of source voices decoding XMA data.
    UINT32 ActiveXmaStreams;            // A voice can use more than one XMA stream.
} XAUDIO2_PERFORMANCE_DATA;

// Used in IXAudio2::SetDebugConfiguration
typedef struct XAUDIO2_DEBUG_CONFIGURATION
{
    UINT32 TraceMask;                   // Bitmap of enabled debug message types.
    UINT32 BreakMask;                   // Message types that will break into the debugger.
    BOOL LogThreadID;                   // Whether to log the thread ID with each message.
    BOOL LogFileline;                   // Whether to log the source file and line number.
    BOOL LogFunctionName;               // Whether to log the function name.
    BOOL LogTiming;                     // Whether to log message timestamps.
} XAUDIO2_DEBUG_CONFIGURATION;

// Values for the TraceMask and BreakMask bitmaps.  Only ERRORS and WARNINGS
// are valid in BreakMask.  WARNINGS implies ERRORS, DETAIL implies INFO, and
// FUNC_CALLS implies API_CALLS.  By default, TraceMask is ERRORS and WARNINGS
// and all the other settings are zero.
#define XAUDIO2_LOG_ERRORS     0x0001   // For handled errors with serious effects.
#define XAUDIO2_LOG_WARNINGS   0x0002   // For handled errors that may be recoverable.
#define XAUDIO2_LOG_INFO       0x0004   // Informational chit-chat (e.g. state changes).
#define XAUDIO2_LOG_DETAIL     0x0008   // More detailed chit-chat.
#define XAUDIO2_LOG_API_CALLS  0x0010   // Public API function entries and exits.
#define XAUDIO2_LOG_FUNC_CALLS 0x0020   // Internal function entries and exits.
#define XAUDIO2_LOG_TIMING     0x0040   // Delays detected and other timing data.
#define XAUDIO2_LOG_LOCKS      0x0080   // Usage of critical sections and mutexes.
#define XAUDIO2_LOG_MEMORY     0x0100   // Memory heap usage information.
#define XAUDIO2_LOG_STREAMING  0x1000   // Audio streaming information.



//-------------------------------------------------------------------------
// Description: AudioClient share mode
//                                   
//     AUDCLNT_SHAREMODE_SHARED -    The device will be opened in shared mode and use the 
//                                   WAS format.
//     AUDCLNT_SHAREMODE_EXCLUSIVE - The device will be opened in exclusive mode and use the 
//                                   application specified format.
//
typedef enum _AUDCLNT_SHAREMODE
{
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_SHAREMODE_EXCLUSIVE
} AUDCLNT_SHAREMODE;

//-------------------------------------------------------------------------
// Description: Audio stream categories
//
// BackgroundCapableMedia  - Music, Streaming audio
// ForegroundOnlyMedia     - Video with audio
// Communications          - VOIP, chat, phone call
// Alerts                  - Alarm, Ring tones
// SoundEffects            - Sound effects, clicks, dings
// GameEffects             - Game sound effects
// GameMedia               - Background audio for games
// Other                   - All other streams (default)
//
typedef enum _AUDIO_STREAM_CATEGORY {
    AudioCategory_Other = 0,
    AudioCategory_ForegroundOnlyMedia,
    AudioCategory_BackgroundCapableMedia,
    AudioCategory_Communications,
    AudioCategory_Alerts,
    AudioCategory_SoundEffects,
    AudioCategory_GameEffects,
    AudioCategory_GameMedia,
} AUDIO_STREAM_CATEGORY;

//-------------------------------------------------------------------------
// Description: AudioClient stream flags
//
// Can be a combination of AUDCLNT_STREAMFLAGS and AUDCLNT_SYSFXFLAGS:
//
// AUDCLNT_STREAMFLAGS (this group of flags uses the high word,
// w/exception of high-bit which is reserved, 0x7FFF0000):
//
//                                  
//     AUDCLNT_STREAMFLAGS_CROSSPROCESS - Audio policy control for this stream will be shared with 
//                                        with other process sessions that use the same audio session 
//                                        GUID.
//     AUDCLNT_STREAMFLAGS_LOOPBACK -     Initializes a renderer endpoint for a loopback audio application. 
//                                        In this mode, a capture stream will be opened on the specified 
//                                        renderer endpoint. Shared mode and a renderer endpoint is required.
//                                        Otherwise the IAudioClient::Initialize call will fail. If the 
//                                        initialize is successful, a capture stream will be available 
//                                        from the IAudioClient object.
//
//     AUDCLNT_STREAMFLAGS_EVENTCALLBACK - An exclusive mode client will supply an event handle that will be
//                                         signaled when an IRP completes (or a waveRT buffer completes) telling
//                                         it to fill the next buffer
//
//     AUDCLNT_STREAMFLAGS_NOPERSIST -     Session state will not be persisted
//
//     AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED -       Session expires when there are no streams and no owning
//                                                    session controls.
//
//     AUDCLNT_SESSIONFLAGS_DISPLAY_HIDE -            Don't show volume control in the Volume Mixer.
//
//     AUDCLNT_SESSIONFLAGS_DISPLAY_HIDEWHENEXPIRED - Don't show volume control in the Volume Mixer after the
//                                                    session expires.
//
// AUDCLNT_SYSFXFLAGS (these flags use low word 0x0000FFFF):
//
//     none defined currently
//
#define AUDCLNT_STREAMFLAGS_CROSSPROCESS             0x00010000
#define AUDCLNT_STREAMFLAGS_LOOPBACK                 0x00020000
#define AUDCLNT_STREAMFLAGS_EVENTCALLBACK            0x00040000
#define AUDCLNT_STREAMFLAGS_NOPERSIST                0x00080000
#define AUDCLNT_STREAMFLAGS_RATEADJUST               0x00100000
#define AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED       0x10000000
#define AUDCLNT_SESSIONFLAGS_DISPLAY_HIDE            0x20000000
#define AUDCLNT_SESSIONFLAGS_DISPLAY_HIDEWHENEXPIRED 0x40000000

//-------------------------------------------------------------------------
// Description: AudioSession State.
//
//      AudioSessionStateInactive - The session has no active audio streams.
//      AudioSessionStateActive - The session has active audio streams.
//      AudioSessionStateExpired - The session is dormant.
typedef enum _AudioSessionState
{
    AudioSessionStateInactive = 0,
    AudioSessionStateActive = 1,
    AudioSessionStateExpired = 2
} AudioSessionState;

/**************************************************************************
*
* IXAudio2: Top-level XAudio2 COM interface.
*
**************************************************************************/


#ifdef __cplusplus
// IXAudio2
interface IXAudio2 : IUnknown {
    //STDMETHOD(QueryInterface) (REFIID riid,  void** ppvInterface) PURE;
    //STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    //STDMETHOD_(ULONG, Release) (THIS) PURE;
    STDMETHOD(RegisterForCallbacks) (IXAudio2EngineCallback* pCallback) PURE;
    STDMETHOD_(void, UnregisterForCallbacks) (IXAudio2EngineCallback* pCallback) PURE;

    STDMETHOD(CreateSourceVoice) (IXAudio2SourceVoice** ppSourceVoice,
        const WAVEFORMATEX* pSourceFormat,
        UINT32 Flags =(0),
        float MaxFrequencyRatio =(XAUDIO2_DEFAULT_FREQ_RATIO),
        IXAudio2VoiceCallback* pCallback =(NULL),
        const XAUDIO2_VOICE_SENDS* pSendList =(NULL),
        const XAUDIO2_EFFECT_CHAIN* pEffectChain =(NULL)) PURE;

    STDMETHOD(CreateSubmixVoice) (IXAudio2SubmixVoice** ppSubmixVoice,
        UINT32 InputChannels, UINT32 InputSampleRate,
        UINT32 Flags =(0), UINT32 ProcessingStage =(0),
        const XAUDIO2_VOICE_SENDS* pSendList =(NULL),
        const XAUDIO2_EFFECT_CHAIN* pEffectChain =(NULL)) PURE;

    STDMETHOD(CreateMasteringVoice) (IXAudio2MasteringVoice** ppMasteringVoice,
        UINT32 InputChannels =(XAUDIO2_DEFAULT_CHANNELS),
        UINT32 InputSampleRate =(XAUDIO2_DEFAULT_SAMPLERATE),
        UINT32 Flags =(0), LPCWSTR szDeviceId =(NULL),
        const XAUDIO2_EFFECT_CHAIN* pEffectChain =(NULL),
        AUDIO_STREAM_CATEGORY StreamCategory =(AudioCategory_GameEffects)) PURE;

    STDMETHOD(StartEngine) (THIS) PURE;
    STDMETHOD_(void, StopEngine) (THIS) PURE;
    STDMETHOD(CommitChanges) (UINT32 OperationSet) PURE;
    STDMETHOD_(void, GetPerformanceData) (XAUDIO2_PERFORMANCE_DATA* pPerfData) PURE;

    STDMETHOD_(void, SetDebugConfiguration) (const XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration,
        void* pReserved =(NULL)) PURE;
};


// IXAudio2Voice
interface IXAudio2Voice {
    STDMETHOD_(void, GetVoiceDetails) (XAUDIO2_VOICE_DETAILS* pVoiceDetails) PURE;
    STDMETHOD(SetOutputVoices) (const XAUDIO2_VOICE_SENDS* pSendList) PURE;
    STDMETHOD(SetEffectChain) (const XAUDIO2_EFFECT_CHAIN* pEffectChain) PURE;
    STDMETHOD(EnableEffect) (UINT32 EffectIndex,
        UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD(DisableEffect) (UINT32 EffectIndex,
        UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD_(void, GetEffectState) (UINT32 EffectIndex, BOOL* pEnabled) PURE;
    STDMETHOD(SetEffectParameters) (UINT32 EffectIndex,
        const void* pParameters,
        UINT32 ParametersByteSize,
        UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD(GetEffectParameters) (UINT32 EffectIndex,
        void* pParameters,
        UINT32 ParametersByteSize) PURE;
    STDMETHOD(SetFilterParameters) (const XAUDIO2_FILTER_PARAMETERS* pParameters,
        UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD_(void, GetFilterParameters) (XAUDIO2_FILTER_PARAMETERS* pParameters) PURE;
    STDMETHOD(SetOutputFilterParameters) (IXAudio2Voice* pDestinationVoice,
        const XAUDIO2_FILTER_PARAMETERS* pParameters,
        UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD_(void, GetOutputFilterParameters) (IXAudio2Voice* pDestinationVoice,
        XAUDIO2_FILTER_PARAMETERS* pParameters) PURE;
    STDMETHOD(SetVolume) (float Volume,
        UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD_(void, GetVolume) (float* pVolume) PURE;
    STDMETHOD(SetChannelVolumes) (UINT32 Channels, const float* pVolumes,
        UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD_(void, GetChannelVolumes) (UINT32 Channels, float* pVolumes) PURE;
    STDMETHOD(SetOutputMatrix) (IXAudio2Voice* pDestinationVoice,
        UINT32 SourceChannels, UINT32 DestinationChannels,
        const float* pLevelMatrix,
        UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD_(void, GetOutputMatrix) (IXAudio2Voice* pDestinationVoice,
        UINT32 SourceChannels, UINT32 DestinationChannels,
        float* pLevelMatrix) PURE;
    STDMETHOD_(void, DestroyVoice) (THIS) PURE;
};

// IXAudio2SourceVoice

interface IXAudio2SourceVoice : IXAudio2Voice {
    STDMETHOD(Start) (UINT32 Flags =(0), UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD(Stop) (UINT32 Flags =(0), UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD(SubmitSourceBuffer) (const XAUDIO2_BUFFER* pBuffer, const XAUDIO2_BUFFER_WMA* pBufferWMA =(NULL)) PURE;
    STDMETHOD(FlushSourceBuffers) (THIS) PURE;
    STDMETHOD(Discontinuity) (THIS) PURE;
    STDMETHOD(ExitLoop) (UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD_(void, GetState) (XAUDIO2_VOICE_STATE* pVoiceState, UINT32 Flags =(0)) PURE;
    STDMETHOD(SetFrequencyRatio) (float Ratio,
        UINT32 OperationSet =(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD_(void, GetFrequencyRatio) (float* pRatio) PURE;
    STDMETHOD(SetSourceSampleRate) (UINT32 NewSourceSampleRate) PURE;
};

// IXAudio2SubmixVoice 
interface IXAudio2SubmixVoice : IXAudio2Voice {

};

// IXAudio2MasteringVoice
interface IXAudio2MasteringVoice : IXAudio2Voice {
    STDMETHOD(GetChannelMask) (DWORD* pChannelmask) PURE;
};

interface IXAudio2EngineCallback {
    // Called by XAudio2 just before an audio processing pass begins.
    STDMETHOD_(void, OnProcessingPassStart) (THIS) PURE;
    // Called just after an audio processing pass ends.
    STDMETHOD_(void, OnProcessingPassEnd) (THIS) PURE;
    // Called in the event of a critical system error which requires XAudio2
    // to be closed down and restarted.  The error code is given in Error.
    STDMETHOD_(void, OnCriticalError) (HRESULT Error) PURE;
};


interface IXAudio2VoiceCallback {
    // Called just before this voice's processing pass begins.
    STDMETHOD_(void, OnVoiceProcessingPassStart) (UINT32 BytesRequired) PURE;
    // Called just after this voice's processing pass ends.
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) PURE;
    // Called when this voice has just finished playing a buffer stream
    // (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
    STDMETHOD_(void, OnStreamEnd) (THIS) PURE;
    // Called when this voice is about to start processing a new buffer.
    STDMETHOD_(void, OnBufferStart) (void* pBufferContext) PURE;
    // Called when this voice has just finished processing a buffer.
    // The buffer can now be reused or destroyed.
    STDMETHOD_(void, OnBufferEnd) (void* pBufferContext) PURE;
    // Called when this voice has just reached the end position of a loop.
    STDMETHOD_(void, OnLoopEnd) (void* pBufferContext) PURE;
    // Called in the event of a critical error during voice processing,
    // such as a failing xAPO or an error from the hardware XMA decoder.
    // The voice may have to be destroyed and re-created to recover from
    // the error.  The callback arguments report which buffer was being
    // processed when the error occurred, and its HRESULT code.
    STDMETHOD_(void, OnVoiceError) (void* pBufferContext, HRESULT Error) PURE;
};
#else
#error NO C declaration
/* FIXME: Add full C declaration */

/**************************************************************************
*
* Macros to make it easier to use the XAudio2 COM interfaces in C code.
*
**************************************************************************/

// IXAudio2
#define IXAudio2_QueryInterface(This,riid,ppvInterface) ((This)->lpVtbl->QueryInterface(This,riid,ppvInterface))
#define IXAudio2_AddRef(This) ((This)->lpVtbl->AddRef(This))
#define IXAudio2_Release(This) ((This)->lpVtbl->Release(This))
#define IXAudio2_CreateSourceVoice(This,ppSourceVoice,pSourceFormat,Flags,MaxFrequencyRatio,pCallback,pSendList,pEffectChain) ((This)->lpVtbl->CreateSourceVoice(This,ppSourceVoice,pSourceFormat,Flags,MaxFrequencyRatio,pCallback,pSendList,pEffectChain))
#define IXAudio2_CreateSubmixVoice(This,ppSubmixVoice,InputChannels,InputSampleRate,Flags,ProcessingStage,pSendList,pEffectChain) ((This)->lpVtbl->CreateSubmixVoice(This,ppSubmixVoice,InputChannels,InputSampleRate,Flags,ProcessingStage,pSendList,pEffectChain))
#define IXAudio2_CreateMasteringVoice(This,ppMasteringVoice,InputChannels,InputSampleRate,Flags,DeviceId,pEffectChain,StreamCategory) ((This)->lpVtbl->CreateMasteringVoice(This,ppMasteringVoice,InputChannels,InputSampleRate,Flags,DeviceId,pEffectChain,StreamCategory))
#define IXAudio2_StartEngine(This) ((This)->lpVtbl->StartEngine(This))
#define IXAudio2_StopEngine(This) ((This)->lpVtbl->StopEngine(This))
#define IXAudio2_CommitChanges(This,OperationSet) ((This)->lpVtbl->CommitChanges(This,OperationSet))
#define IXAudio2_GetPerformanceData(This,pPerfData) ((This)->lpVtbl->GetPerformanceData(This,pPerfData))
#define IXAudio2_SetDebugConfiguration(This,pDebugConfiguration,pReserved) ((This)->lpVtbl->SetDebugConfiguration(This,pDebugConfiguration,pReserved))

// IXAudio2Voice
#define IXAudio2Voice_GetVoiceDetails(This,pVoiceDetails) ((This)->lpVtbl->GetVoiceDetails(This,pVoiceDetails))
#define IXAudio2Voice_SetOutputVoices(This,pSendList) ((This)->lpVtbl->SetOutputVoices(This,pSendList))
#define IXAudio2Voice_SetEffectChain(This,pEffectChain) ((This)->lpVtbl->SetEffectChain(This,pEffectChain))
#define IXAudio2Voice_EnableEffect(This,EffectIndex,OperationSet) ((This)->lpVtbl->EnableEffect(This,EffectIndex,OperationSet))
#define IXAudio2Voice_DisableEffect(This,EffectIndex,OperationSet) ((This)->lpVtbl->DisableEffect(This,EffectIndex,OperationSet))
#define IXAudio2Voice_GetEffectState(This,EffectIndex,pEnabled) ((This)->lpVtbl->GetEffectState(This,EffectIndex,pEnabled))
#define IXAudio2Voice_SetEffectParameters(This,EffectIndex,pParameters,ParametersByteSize, OperationSet) ((This)->lpVtbl->SetEffectParameters(This,EffectIndex,pParameters,ParametersByteSize,OperationSet))
#define IXAudio2Voice_GetEffectParameters(This,EffectIndex,pParameters,ParametersByteSize) ((This)->lpVtbl->GetEffectParameters(This,EffectIndex,pParameters,ParametersByteSize))
#define IXAudio2Voice_SetFilterParameters(This,pParameters,OperationSet) ((This)->lpVtbl->SetFilterParameters(This,pParameters,OperationSet))
#define IXAudio2Voice_GetFilterParameters(This,pParameters) ((This)->lpVtbl->GetFilterParameters(This,pParameters))
#define IXAudio2Voice_SetOutputFilterParameters(This,pDestinationVoice,pParameters,OperationSet) ((This)->lpVtbl->SetOutputFilterParameters(This,pDestinationVoice,pParameters,OperationSet))
#define IXAudio2Voice_GetOutputFilterParameters(This,pDestinationVoice,pParameters) ((This)->lpVtbl->GetOutputFilterParameters(This,pDestinationVoice,pParameters))
#define IXAudio2Voice_SetVolume(This,Volume,OperationSet) ((This)->lpVtbl->SetVolume(This,Volume,OperationSet))
#define IXAudio2Voice_GetVolume(This,pVolume) ((This)->lpVtbl->GetVolume(This,pVolume))
#define IXAudio2Voice_SetChannelVolumes(This,Channels,pVolumes,OperationSet) ((This)->lpVtbl->SetChannelVolumes(This,Channels,pVolumes,OperationSet))
#define IXAudio2Voice_GetChannelVolumes(This,Channels,pVolumes) ((This)->lpVtbl->GetChannelVolumes(This,Channels,pVolumes))
#define IXAudio2Voice_SetOutputMatrix(This,pDestinationVoice,SourceChannels,DestinationChannels,pLevelMatrix,OperationSet) ((This)->lpVtbl->SetOutputMatrix(This,pDestinationVoice,SourceChannels,DestinationChannels,pLevelMatrix,OperationSet))
#define IXAudio2Voice_GetOutputMatrix(This,pDestinationVoice,SourceChannels,DestinationChannels,pLevelMatrix) ((This)->lpVtbl->GetOutputMatrix(This,pDestinationVoice,SourceChannels,DestinationChannels,pLevelMatrix))
#define IXAudio2Voice_DestroyVoice(This) ((This)->lpVtbl->DestroyVoice(This))

// IXAudio2SourceVoice
#define IXAudio2SourceVoice_GetVoiceDetails IXAudio2Voice_GetVoiceDetails
#define IXAudio2SourceVoice_SetOutputVoices IXAudio2Voice_SetOutputVoices
#define IXAudio2SourceVoice_SetEffectChain IXAudio2Voice_SetEffectChain
#define IXAudio2SourceVoice_EnableEffect IXAudio2Voice_EnableEffect
#define IXAudio2SourceVoice_DisableEffect IXAudio2Voice_DisableEffect
#define IXAudio2SourceVoice_GetEffectState IXAudio2Voice_GetEffectState
#define IXAudio2SourceVoice_SetEffectParameters IXAudio2Voice_SetEffectParameters
#define IXAudio2SourceVoice_GetEffectParameters IXAudio2Voice_GetEffectParameters
#define IXAudio2SourceVoice_SetFilterParameters IXAudio2Voice_SetFilterParameters
#define IXAudio2SourceVoice_GetFilterParameters IXAudio2Voice_GetFilterParameters
#define IXAudio2SourceVoice_SetOutputFilterParameters IXAudio2Voice_SetOutputFilterParameters
#define IXAudio2SourceVoice_GetOutputFilterParameters IXAudio2Voice_GetOutputFilterParameters
#define IXAudio2SourceVoice_SetVolume IXAudio2Voice_SetVolume
#define IXAudio2SourceVoice_GetVolume IXAudio2Voice_GetVolume
#define IXAudio2SourceVoice_SetChannelVolumes IXAudio2Voice_SetChannelVolumes
#define IXAudio2SourceVoice_GetChannelVolumes IXAudio2Voice_GetChannelVolumes
#define IXAudio2SourceVoice_SetOutputMatrix IXAudio2Voice_SetOutputMatrix
#define IXAudio2SourceVoice_GetOutputMatrix IXAudio2Voice_GetOutputMatrix
#define IXAudio2SourceVoice_DestroyVoice IXAudio2Voice_DestroyVoice
#define IXAudio2SourceVoice_Start(This,Flags,OperationSet) ((This)->lpVtbl->Start(This,Flags,OperationSet))
#define IXAudio2SourceVoice_Stop(This,Flags,OperationSet) ((This)->lpVtbl->Stop(This,Flags,OperationSet))
#define IXAudio2SourceVoice_SubmitSourceBuffer(This,pBuffer,pBufferWMA) ((This)->lpVtbl->SubmitSourceBuffer(This,pBuffer,pBufferWMA))
#define IXAudio2SourceVoice_FlushSourceBuffers(This) ((This)->lpVtbl->FlushSourceBuffers(This))
#define IXAudio2SourceVoice_Discontinuity(This) ((This)->lpVtbl->Discontinuity(This))
#define IXAudio2SourceVoice_ExitLoop(This,OperationSet) ((This)->lpVtbl->ExitLoop(This,OperationSet))
#define IXAudio2SourceVoice_GetState(This,pVoiceState,Flags) ((This)->lpVtbl->GetState(This,pVoiceState,Flags))
#define IXAudio2SourceVoice_SetFrequencyRatio(This,Ratio,OperationSet) ((This)->lpVtbl->SetFrequencyRatio(This,Ratio,OperationSet))
#define IXAudio2SourceVoice_GetFrequencyRatio(This,pRatio) ((This)->lpVtbl->GetFrequencyRatio(This,pRatio))
#define IXAudio2SourceVoice_SetSourceSampleRate(This,NewSourceSampleRate) ((This)->lpVtbl->SetSourceSampleRate(This,NewSourceSampleRate))

// IXAudio2SubmixVoice
#define IXAudio2SubmixVoice_GetVoiceDetails IXAudio2Voice_GetVoiceDetails
#define IXAudio2SubmixVoice_SetOutputVoices IXAudio2Voice_SetOutputVoices
#define IXAudio2SubmixVoice_SetEffectChain IXAudio2Voice_SetEffectChain
#define IXAudio2SubmixVoice_EnableEffect IXAudio2Voice_EnableEffect
#define IXAudio2SubmixVoice_DisableEffect IXAudio2Voice_DisableEffect
#define IXAudio2SubmixVoice_GetEffectState IXAudio2Voice_GetEffectState
#define IXAudio2SubmixVoice_SetEffectParameters IXAudio2Voice_SetEffectParameters
#define IXAudio2SubmixVoice_GetEffectParameters IXAudio2Voice_GetEffectParameters
#define IXAudio2SubmixVoice_SetFilterParameters IXAudio2Voice_SetFilterParameters
#define IXAudio2SubmixVoice_GetFilterParameters IXAudio2Voice_GetFilterParameters
#define IXAudio2SubmixVoice_SetOutputFilterParameters IXAudio2Voice_SetOutputFilterParameters
#define IXAudio2SubmixVoice_GetOutputFilterParameters IXAudio2Voice_GetOutputFilterParameters
#define IXAudio2SubmixVoice_SetVolume IXAudio2Voice_SetVolume
#define IXAudio2SubmixVoice_GetVolume IXAudio2Voice_GetVolume
#define IXAudio2SubmixVoice_SetChannelVolumes IXAudio2Voice_SetChannelVolumes
#define IXAudio2SubmixVoice_GetChannelVolumes IXAudio2Voice_GetChannelVolumes
#define IXAudio2SubmixVoice_SetOutputMatrix IXAudio2Voice_SetOutputMatrix
#define IXAudio2SubmixVoice_GetOutputMatrix IXAudio2Voice_GetOutputMatrix
#define IXAudio2SubmixVoice_DestroyVoice IXAudio2Voice_DestroyVoice

// IXAudio2MasteringVoice
#define IXAudio2MasteringVoice_GetVoiceDetails IXAudio2Voice_GetVoiceDetails
#define IXAudio2MasteringVoice_SetOutputVoices IXAudio2Voice_SetOutputVoices
#define IXAudio2MasteringVoice_SetEffectChain IXAudio2Voice_SetEffectChain
#define IXAudio2MasteringVoice_EnableEffect IXAudio2Voice_EnableEffect
#define IXAudio2MasteringVoice_DisableEffect IXAudio2Voice_DisableEffect
#define IXAudio2MasteringVoice_GetEffectState IXAudio2Voice_GetEffectState
#define IXAudio2MasteringVoice_SetEffectParameters IXAudio2Voice_SetEffectParameters
#define IXAudio2MasteringVoice_GetEffectParameters IXAudio2Voice_GetEffectParameters
#define IXAudio2MasteringVoice_SetFilterParameters IXAudio2Voice_SetFilterParameters
#define IXAudio2MasteringVoice_GetFilterParameters IXAudio2Voice_GetFilterParameters
#define IXAudio2MasteringVoice_SetOutputFilterParameters IXAudio2Voice_SetOutputFilterParameters
#define IXAudio2MasteringVoice_GetOutputFilterParameters IXAudio2Voice_GetOutputFilterParameters
#define IXAudio2MasteringVoice_SetVolume IXAudio2Voice_SetVolume
#define IXAudio2MasteringVoice_GetVolume IXAudio2Voice_GetVolume
#define IXAudio2MasteringVoice_SetChannelVolumes IXAudio2Voice_SetChannelVolumes
#define IXAudio2MasteringVoice_GetChannelVolumes IXAudio2Voice_GetChannelVolumes
#define IXAudio2MasteringVoice_SetOutputMatrix IXAudio2Voice_SetOutputMatrix
#define IXAudio2MasteringVoice_GetOutputMatrix IXAudio2Voice_GetOutputMatrix
#define IXAudio2MasteringVoice_DestroyVoice IXAudio2Voice_DestroyVoice
#define IXAudio2MasteringVoice_GetChannelMask(This,pChannelMask) ((This)->lpVtbl->GetChannelMask(This,pChannelMask))

#endif // #ifndef __cplusplus


/**************************************************************************
*
* Utility functions used to convert from pitch in semitones and volume
* in decibels to the frequency and amplitude ratio units used by XAudio2.
* These are only defined if the client #defines XAUDIO2_HELPER_FUNCTIONS
* prior to #including xaudio2.h.
*
**************************************************************************/

#ifdef XAUDIO2_HELPER_FUNCTIONS

#define _USE_MATH_DEFINES   // Make math.h define M_PI
#include <math.h>           // For powf, log10f, sinf and asinf

// Calculate the argument to SetVolume from a decibel value
__inline float XAudio2DecibelsToAmplitudeRatio(float Decibels)
{
    return powf(10.0f, Decibels / 20.0f);
}

// Recover a volume in decibels from an amplitude factor
__inline float XAudio2AmplitudeRatioToDecibels(float Volume)
{
    if (Volume == 0)
    {
        return -3.402823466e+38f; // Smallest float value (-FLT_MAX)
    }
    return 20.0f * log10f(Volume);
}

// Calculate the argument to SetFrequencyRatio from a semitone value
__inline float XAudio2SemitonesToFrequencyRatio(float Semitones)
{
    // FrequencyRatio = 2 ^ Octaves
    //                = 2 ^ (Semitones / 12)
    return powf(2.0f, Semitones / 12.0f);
}

// Recover a pitch in semitones from a frequency ratio
__inline float XAudio2FrequencyRatioToSemitones(float FrequencyRatio)
{
    // Semitones = 12 * log2(FrequencyRatio)
    //           = 12 * log2(10) * log10(FrequencyRatio)
    return 39.86313713864835f * log10f(FrequencyRatio);
}

// Convert from filter cutoff frequencies expressed in Hertz to the radian
// frequency values used in XAUDIO2_FILTER_PARAMETERS.Frequency, state-variable
// filter types only.  Use XAudio2CutoffFrequencyToOnePoleCoefficient() for one-pole filter types.
// Note that the highest CutoffFrequency supported is SampleRate/6.
// Higher values of CutoffFrequency will return XAUDIO2_MAX_FILTER_FREQUENCY.
__inline float XAudio2CutoffFrequencyToRadians(float CutoffFrequency, UINT32 SampleRate)
{
    if ((UINT32)(CutoffFrequency * 6.0f) >= SampleRate)
    {
        return XAUDIO2_MAX_FILTER_FREQUENCY;
    }
    return 2.0f * sinf((float)M_PI * CutoffFrequency / SampleRate);
}

// Convert from radian frequencies back to absolute frequencies in Hertz
__inline float XAudio2RadiansToCutoffFrequency(float Radians, float SampleRate)
{
    return SampleRate * asinf(Radians / 2.0f) / (float)M_PI;
}

// Convert from filter cutoff frequencies expressed in Hertz to the filter
// coefficients used with XAUDIO2_FILTER_PARAMETERS.Frequency,
// LowPassOnePoleFilter and HighPassOnePoleFilter filter types only.
// Use XAudio2CutoffFrequencyToRadians() for state-variable filter types.
__inline float XAudio2CutoffFrequencyToOnePoleCoefficient(float CutoffFrequency, UINT32 SampleRate)
{
    if ((UINT32)CutoffFrequency >= SampleRate)
    {
        return XAUDIO2_MAX_FILTER_FREQUENCY;
    }
    return (1.0f - powf(1.0f - 2.0f * CutoffFrequency / SampleRate, 2.0f));
}


#endif // #ifdef XAUDIO2_HELPER_FUNCTIONS


/**************************************************************************
*
* XAudio2Create: Top-level function that creates an XAudio2 instance.
*
* ARGUMENTS:
*
*  Flags - Flags specifying the XAudio2 object's behavior.  Currently
*          unused, must be set to 0.
*
*  XAudio2Processor - An XAUDIO2_PROCESSOR value that specifies the
*          hardware threads (Xbox) or processors (Windows) that XAudio2
*          will use.  Note that XAudio2 supports concurrent processing on
*          multiple threads, using any combination of XAUDIO2_PROCESSOR
*          flags.  The values are platform-specific; platform-independent
*          code can use XAUDIO2_DEFAULT_PROCESSOR to use the default on
*          each platform.
*
**************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
HRESULT __stdcall XAudio2Create(IXAudio2** ppXAudio2, UINT32 Flags =(0),
        XAUDIO2_PROCESSOR XAudio2Processor =(XAUDIO2_DEFAULT_PROCESSOR));

#ifdef __cplusplus
}
#endif
// Undo the #pragma pack(push, 1) directive at the top of this file
#pragma pack(pop)

#endif