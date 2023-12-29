#include "BusyChunkWriter.h"

// TODO - handle error conditions
bool BusyChunkWriter::write (juce::File inputSampleFile, juce::File outputSampleFile, juce::MemoryBlock& busyChunkData)
{
    // open input file
    auto inputSampleStream { inputSampleFile.createInputStream () };
    jassert (inputSampleStream != nullptr && inputSampleStream->openedOk ());

    // create/open temp file
    auto outputSampleStream { outputSampleFile.createOutputStream () };
    jassert (outputSampleStream != nullptr && outputSampleStream->openedOk ());
    juce::MemoryBlock chunkData;

    while (true)
    {
        juce::Logger::outputDebugString ("offset: " + juce::String::toHexString (inputSampleStream->getPosition ()));
        auto chunk { getChunkData (inputSampleStream.get ()) };
        if (! chunk.has_value ())
            break;
        auto chunkInfo { chunk.value () };

        chunkData.setSize (chunkInfo.chunkLength);
        // read chunk data from input file
        const auto bytesRead { inputSampleStream->read (chunkData.getData (), chunkInfo.chunkLength) };
        jassert (bytesRead == static_cast<int> (chunkInfo.chunkLength));

        // write all non busy chunks (ie. skip old busy chunk, if one exists)
        if (std::memcmp (kBusyChunkType, chunkInfo.chunkType, 4) != 0)
        {
            // write chunk header
            auto writeSuccess { outputSampleStream->write (chunkInfo.chunkType, 4) };
            jassert (writeSuccess == true);
            uint32_t chunkLength { juce::ByteOrder::swapIfBigEndian (chunkInfo.chunkLength) };
            writeSuccess = outputSampleStream->write (&chunkLength, 4);
            jassert (writeSuccess == true);

            // write chunk data to output file
            writeSuccess = outputSampleStream->write (chunkData.getData (), chunkData.getSize ());
            jassert (writeSuccess == true);
        }
    }
    // write new busy chunk
    auto writeSuccess { outputSampleStream->write (kBusyChunkType, 4) };
    jassert (writeSuccess == true);
    uint32_t chunkLength { juce::ByteOrder::swapIfBigEndian (static_cast<uint32_t> (4)) };
    writeSuccess = outputSampleStream->write (&chunkLength, 4);
    jassert (writeSuccess == true);
    // write new busyChunk
    writeSuccess = outputSampleStream->write (busyChunkData.getData (), busyChunkData.getSize ());
    jassert (writeSuccess == true);
    return true;
}

std::optional<BusyChunkWriter::ChunkInfo> BusyChunkWriter::getChunkData (juce::InputStream* is)
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
