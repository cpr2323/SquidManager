#pragma once

#include <JuceHeader.h>

class BusyChunkReader
{
public:
    using MarkerList = std::vector<uint32_t>;
    BusyChunkReader () = default;

    bool readMetaData (juce::File sampleFile, juce::MemoryBlock& busyChunkData);
    MarkerList getMarkerList (juce::File sampleFile);

private:
    static inline char kBusyChunkType [4] { 'b', 'u', 's', 'y' };
    static inline char kRIFFChunkType [4] { 'R', 'I', 'F', 'F' };
    static inline char kWAVEFormatId [4] { 'W', 'A', 'V', 'E' };
    static inline char kMarkerListChunkType [4] { 'c', 'u', 'e', ' ' };
    struct ChunkInfo
    {
        char chunkType [4];
        uint32_t chunkLength { 0 };
    };

    std::optional<uint32_t> findChunk (juce::InputStream* is, char* chunkType);
    std::optional<ChunkInfo> getChunkData (juce::InputStream* is);
};