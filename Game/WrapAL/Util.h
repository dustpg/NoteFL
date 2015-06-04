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

// mpg123
#include <sys\types.h>
#ifdef _MSC_VER
using ssize_t = SSIZE_T;
#endif
// MY CONFIGURE
#if 0
#define NO_ICY 1
#define NO_STRING 1
#define NO_ID3V2 1
#define NO_WARNING 1
#define NO_ERRORMSG 1
#define NO_ERETURN 1
#define NO_FEEDER 1
#define USE_NEW_HUFFTABLE 1
#endif
#define MPG123_NO_CONFIGURE
#include "../3rdparty/mpg123/mpg123.h.in"

// the _wrapal32
constexpr uint32_t operator"" _wrapal32(const char* src, size_t len) {
    return len == 2 ?
        static_cast<uint32_t>(src[0]) << (8 * 0) |
        static_cast<uint32_t>(src[1]) << (8 * 1) :
        static_cast<uint32_t>(src[0]) << (8 * 0) |
        static_cast<uint32_t>(src[1]) << (8 * 1) |
        static_cast<uint32_t>(src[2]) << (8 * 2) |
        static_cast<uint32_t>(src[3]) << (8 * 3);
}

// wrapal namespace
namespace WrapAL {
    // Mpg123 dll, using dynamic-linking to avoid LGPL
    class Mpg123 {
    public:
        // init
        static void Init(HMODULE)  noexcept;
    public: // function zone
        // mpg123 : init
        static decltype(&::mpg123_init) mpg123_init;
        // mpg123 : exit
        static decltype(&::mpg123_exit) mpg123_exit;
        // mpg123 : new
        static decltype(&::mpg123_new) mpg123_new;
        // mpg123 : delete
        static decltype(&::mpg123_delete) mpg123_delete;
        // mpg123 : read
        static decltype(&::mpg123_read) mpg123_read;
        // mpg123 : seek
        static decltype(&::mpg123_seek) mpg123_seek;
        // mpg123 : tell
        static decltype(&::mpg123_tell) mpg123_tell;
        // mpg123 : length
        static decltype(&::mpg123_length) mpg123_length;
        // mpg123 : format
        static decltype(&::mpg123_format) mpg123_format;
        // mpg123 : get format
        static decltype(&::mpg123_getformat) mpg123_getformat;
        // mpg123 : format none
        static decltype(&::mpg123_format_none) mpg123_format_none;
        // mpg123 : open with handle
        static decltype(&::mpg123_open_handle) mpg123_open_handle;
        // mpg123 : replace reader handle
        static decltype(&::mpg123_replace_reader_handle) mpg123_replace_reader_handle;
    };
#ifdef WRAPAL_INCLUDE_DEFAULT_CONFIGURE
    // default al configure
    class CALDefConfigure final : public IALConfigure {
    public:
        // default audio stream creation func
        static auto DefCreateAudioStream(AudioFormat, const wchar_t*, wchar_t info[/*ErrorInfoLength*/])noexcept->IALAudioStream*;
    public:
        // cotr
        CALDefConfigure() { m_szLastError[0] = 0; };
        // dotr
        ~CALDefConfigure() = default;
        // set hwnd for this
        auto SetHwnd(HWND hwnd) noexcept { m_hwnd = hwnd; }
    public: // infterface impl for IALConfigure
        // release this
        virtual auto Release() noexcept ->int32_t override { return 1; }
        // create audio stream from file
        virtual auto CreateAudioStream(AudioFormat, const wchar_t*) noexcept->IALAudioStream* override;
        // get last error infomation, return false if no error
        virtual auto GetLastErrorInfo(wchar_t info[/*ErrorInfoLength*/])noexcept->bool override;
        // output error infomation
        virtual auto OutputError(const wchar_t* err)noexcept->void override { ::MessageBoxW(m_hwnd, err, L"Error!", MB_ICONERROR); }
        // get the "libmpg123.dll" path
        virtual auto GetLibmpg123_dllPath(wchar_t path[/*MAX_PATH*/])noexcept->void;
    private:
        // HWND for mainwindow
        HWND                m_hwnd = nullptr;
        // last error infomation
        wchar_t             m_szLastError[ErrorInfoLength];
    };
#endif
    // object pool, to store SIMPLE struct(can using memcpy to move), 
    // AND MUST EXIST VIRTUAL FUNCTION ---> because *(size_t*)(XXX) > 0 will be true when alive
    template<typename T, size_t BucketSize, size_t StackLength = ObjectPoolLength>
    class ObjectPool {
        // pool bucket
        struct Bucket {
            // next bucket
            Bucket*     next;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4200)
            T           buffer[0];
#pragma warning(pop)
#else
            T           buffer[0];

#endif
        };
    public:
        // ctor
        ObjectPool() noexcept;
        // dtor
        ~ObjectPool() noexcept;
        // alloc object
        auto Alloc() noexcept ->T*;
        // alloc object
        auto Free(T*) noexcept ->void;
        // release 
        template<typename lambda_t> void Release(lambda_t lambda);
    private:
        // first bucket
        Bucket*             m_p1stBucket;
        // now bucket used space
        uint32_t            m_uBucketUsed = 0;
        // stack length
        uint32_t            m_uStackLength;
        // free space stack
        T**                 m_ppFreeStack;
        // the top of stack
        T**                 m_ppStackTop;
    };
    // impl for ctor
    template<typename T, size_t BucketSize, size_t StackLength>
    WrapAL::ObjectPool<T, BucketSize, StackLength>::ObjectPool() noexcept :
    m_p1stBucket(reinterpret_cast<Bucket*>(::malloc(BucketSize))),
        m_uStackLength(StackLength),
        m_ppFreeStack(reinterpret_cast<T**>(::malloc(StackLength * sizeof(void*)))),
        m_ppStackTop(m_ppFreeStack) {
        m_p1stBucket->next = nullptr;
        // out of memory is really bad for allocator :(
        assert(m_p1stBucket && m_ppFreeStack && "out of memory");
    }
    // impl for dtor
    template<typename T, size_t BucketSize, size_t StackLength>
    WrapAL::ObjectPool<T, BucketSize, StackLength>::~ObjectPool() noexcept {
        // release the data
        auto tmp = m_p1stBucket;
        while (tmp) {
            register auto next = tmp->next;
            ::free(tmp);
            tmp = next;
        }
        // release the data
        if (m_ppFreeStack) {
            ::free(m_ppFreeStack);
            m_ppFreeStack = nullptr;
            m_ppStackTop = nullptr;
        }
    }
    // impl for alloc
    template<typename T, size_t BucketSize, size_t StackLength>
    auto WrapAL::ObjectPool<T, BucketSize, StackLength>::Alloc() noexcept -> T* {
        T* buf = nullptr;
        // check the "FREE STACK"
        assert(m_ppStackTop >= m_ppFreeStack && "amazing!");
        if (m_ppStackTop > m_ppFreeStack) {
            --m_ppStackTop;
            buf = *m_ppStackTop;
        }
        else {
            // greater than max?
            constexpr uint32_t max_count = (BucketSize - sizeof(void*)) / sizeof(T);
            if (m_uBucketUsed >= max_count) {
                // malloc new space
                auto* new_bucket = reinterpret_cast<Bucket*>(::malloc(BucketSize));
                assert(new_bucket && "out of memory");
                if (new_bucket) {
                    new_bucket->next = m_p1stBucket;
                    m_p1stBucket = new_bucket;
                    m_uBucketUsed = 0;
                }
            }
            // OK?
            if (m_uBucketUsed < max_count) {
                buf = m_p1stBucket->buffer + m_uBucketUsed;
                ++m_uBucketUsed;
            }
        }
        return reinterpret_cast<T*>(buf);
    }
    // impl for free
    template<typename T, size_t BucketSize, size_t StackLength>
    auto WrapAL::ObjectPool<T, BucketSize, StackLength>::Free(T* stream) noexcept -> void {
#ifdef _DEBUG
        // check
        wchar_t buffer[ErrorInfoLength];
        bool ok = false;
        auto tmp = m_p1stBucket;
        while (tmp) {
            if (stream > static_cast<void*>(tmp) &&
                static_cast<void*>(stream) < reinterpret_cast<uint8_t*>(tmp) + BucketSize) {
                ok = true;
                break;
            }
            tmp = tmp->next;
        }
        if (!ok) {
            ::swprintf(buffer, ErrorInfoLength, L"Invalid Address @ 0x%08X\n", stream);
            ::MessageBoxW(nullptr, buffer, L"Error", MB_ICONERROR);
            assert(!"invalid address");
        }
        // check
        for (auto itr = m_ppFreeStack; itr < m_ppStackTop; ++itr) {
            // repeated?
            if (stream == static_cast<void*>(*itr)) {
                ::swprintf(buffer, ErrorInfoLength, L"Address @ 0x%08X, had been freed\n", stream);
                ::MessageBoxW(nullptr, buffer, L"Error", MB_ICONERROR);
                assert(!"address had been freed");
                break;
            }
        }
#endif
        // check free stack
        if (m_ppFreeStack + m_uStackLength <= m_ppStackTop) {
            auto new_length = m_uStackLength * 2;
            auto new_stack = reinterpret_cast<T**>(::malloc(new_length * sizeof(void*)));
            assert(new_stack && "out of memory");
            if (new_stack) {
                ::memcpy(new_stack, m_ppFreeStack, sizeof(void*) * m_uStackLength);
                ::free(m_ppFreeStack);
                m_ppFreeStack = new_stack;
                m_ppStackTop = m_ppFreeStack + m_uStackLength;
                m_uStackLength = new_length;
            }
        }
        // if ok
        if (m_ppFreeStack + m_uStackLength > m_ppStackTop) {
            *reinterpret_cast<size_t*>(stream) = 0;
            *m_ppStackTop = reinterpret_cast<T*>(stream);
            ++m_ppStackTop;
        }
    }
    // impl for release
    template<typename T, size_t BucketSize, size_t StackLength>
    template <typename lambda_t>
    void WrapAL::ObjectPool<T, BucketSize, StackLength>::Release(lambda_t lambda) {
        // max
        constexpr uint32_t max_count = (BucketSize - sizeof(void*)) / sizeof(T);
        // init
        auto now_count = m_uBucketUsed;
        auto now_bucket = m_p1stBucket;
        while (now_bucket) {
            // release
            for (auto itr = now_bucket->buffer; itr < now_bucket->buffer + now_count; ++itr) {
#if 1
                // check first size_t(vtable > 0 -> alive, ==0 -> dead)
                if (*reinterpret_cast<size_t*>(itr)) {
                    lambda(itr);
                }
#else
                bool release_it = true;
                // check in free stack, not in, release it
                for (auto itr2 = m_ppFreeStack; itr2 < m_ppStackTop; ++itr) {
                    if (*itr2 == itr) {
                        release_it = false;
                        break;
                    }
                }
                // release it
                if (release_it) {
                    lambda(itr);
                }
#endif

            }
            now_bucket = now_bucket->next;
            now_count = max_count;
        }
    }
}