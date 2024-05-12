#pragma once

#include <JuceHeader.h>

class SquidMetaDataWriter
{
public:
    SquidMetaDataWriter () = default;

    bool write (juce::ValueTree squidMetaDataPropertiesVT, juce::File inputSampleFile, juce::File outputSampleFile);

private:
    juce::MemoryBlock busyChunkData;
    void setUInt8 (uint8_t value, int offset);
    void setUInt16 (uint16_t value, int offset);
    void setUInt32 (uint32_t value, int offset);
    void setUInt64 (uint64_t value, int offset);
};