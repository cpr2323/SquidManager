#pragma once

#include <JuceHeader.h>
#include "BusyChunkReader.h"
#include "SquidMetaDataProperties.h"

class SquidMetaDataReader
{
public:
    SquidMetaDataReader () = default;

    juce::ValueTree read (juce::File sampleFile);

private:
    juce::MemoryBlock busyChunkData;
    uint8_t getUInt8 (int offset);
    uint16_t getUInt16 (int offset);
    uint32_t getUInt32 (int offset);
    uint64_t getUInt64 (int offset);
};