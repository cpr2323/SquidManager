#include "BusyChunkReader.h"

void BusyChunkReader::read (juce::File sampleFile, juce::MemoryBlock& busyChunkData)
{
    // TODO - handle error conditions
    auto sampleInputStream { sampleFile.createInputStream () };
    jassert (sampleInputStream != nullptr && sampleInputStream->openedOk ());
    auto busyChunkLocated { findChunk (sampleInputStream.get (), kBusyChunkType) };
    jassert (busyChunkLocated.has_value ());
    sampleInputStream->readIntoMemoryBlock (busyChunkData, busyChunkLocated.value ());
}

std::optional<uint32_t> BusyChunkReader::findChunk (juce::InputStream* is, char* chunkType)
{
    auto chunkLocated { false };
    auto chunkLength { 0 };
    while (! chunkLocated)
    {
        auto chunk { getChunkData (is) };
        if (! chunk.has_value ())
            break;
        auto chunkInfo { chunk.value () };
        if (std::memcmp (chunkType, chunkInfo.chunkType, 4) != 0)
        {
            auto seekSuccess { is->setPosition (is->getPosition () + chunkInfo.chunkLength) };
            jassert (seekSuccess);
            continue;
        }
        chunkLength = chunkInfo.chunkLength;
        chunkLocated = true;
    }
    return chunkLocated ? std::optional<uint32_t> { chunkLength } : std::nullopt;
}

std::optional<BusyChunkReader::ChunkInfo> BusyChunkReader::getChunkData (juce::InputStream* is)
{
    ChunkInfo chunkInfo;
    if (is->read (&chunkInfo.chunkType, 4) != 4)
        return std::nullopt;
    uint32_t chunkLength { 0 };
    if (is->read (&chunkLength, 4) != 4)
        return std::nullopt;
    chunkInfo.chunkLength = juce::ByteOrder::swapIfBigEndian (chunkLength);

    return chunkInfo;
}
