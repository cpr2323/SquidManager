#pragma once

#include <JuceHeader.h>

class SquidMetaDataReader
{
public:
    SquidMetaDataReader () = default;

    void read (juce::ValueTree channelPropertiesVT, juce::File sampleFile, uint8_t channelIndex);

private:
    juce::MemoryBlock busyChunkData;

    template <int N>
    struct ReturnType {
        using type = void; // Default type, can be an error or a default type
    };

    // Template specialization for N = 1
    template <>
    struct ReturnType<1>
    {
        using type = uint8_t;

        static type process (uint8_t* data, int offset)
        {
            return *(data + offset);
        }
    };

    // Template specialization for N = 2
    template <>
    struct ReturnType<2>
    {
        using type = uint16_t;

        static type process (uint8_t* data, int offset)
        {
            const auto rawValue { *(reinterpret_cast<uint16_t*>(data + offset)) };
            return juce::ByteOrder::swapIfBigEndian (rawValue);
        }
    };

    // Template specialization for N = 4
    template <>
    struct ReturnType<4>
    {
        using type = uint32_t;

        static type process (uint8_t* data, int offset)
        {
            const auto rawValue { *(reinterpret_cast<uint32_t*>(data + offset)) };
            return juce::ByteOrder::swapIfBigEndian (rawValue);
        }
    };

    // Template specialization for N = 8
    template <>
    struct ReturnType<8>
    {
        using type = uint64_t;

        static type process (uint8_t* data, int offset)
        {
            const auto rawValue { *(reinterpret_cast<uint64_t*>(data + offset)) };
            return juce::ByteOrder::swapIfBigEndian (rawValue);
        }
    };

    // Function template using the ReturnType type
    template <int N>
    typename ReturnType<N>::type getValue (int value) {
        return ReturnType<N>::process (static_cast<uint8_t*>(busyChunkData.getData()), value);
    }
};