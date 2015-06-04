#pragma once
// THIS FILE IS NONE-LICENSE

// include CALDefConfigure
#define WRAPAL_INCLUDE_DEFAULT_CONFIGURE

// name
namespace WrapAL {
    // some config
    enum WrapALConstant : unsigned int {
        // object pool length in defaultly
        ObjectPoolLength = 1024,
        // error text defaultly length
        ErrorInfoLength = 1024,
        // each audio stream bucket size
        AudioStreamBucketSize = 32 * 1024,
        // streaming buffer size
        StreamingBufferSize = 16 * 1024,
        // streaming buffer count
        StreamingBufferCount = 3,
    };
}