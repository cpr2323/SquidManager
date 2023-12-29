#pragma once

#include <JuceHeader.h>
#include "BusyChunkReader.h"

class BusyChunkWriter
{
public:
    BusyChunkWriter () = default;

    bool write (juce::File sampleFile, juce::MemoryBlock& busyChunkData);

private:
    static inline char kBusyChunkType [4] { 'b', 'u', 's', 'y' };
    struct ChunkInfo
    {
        char chunkType [4];
        uint32_t chunkLength { 0 };
    };

    std::optional<uint32_t> findChunk (juce::InputStream* is, char* chunkType);
    std::optional<ChunkInfo> getChunkData (juce::InputStream* is);
};