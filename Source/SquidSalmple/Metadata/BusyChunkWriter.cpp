#include "BusyChunkWriter.h"

// TODO - handle error conditions
bool BusyChunkWriter::write (juce::AudioBuffer<float>& audioBuffer, juce::File outputSampleFile, juce::MemoryBlock& busyChunkData)
{
    // write audio data
    {
        // create/open temp file
        auto outputSampleStream { outputSampleFile.createOutputStream () };
        jassert (outputSampleStream != nullptr && outputSampleStream->openedOk ());
        outputSampleStream->setPosition (0);
        outputSampleStream->truncate ();
        juce::WavAudioFormat wavAudioFormat;
        std::unique_ptr<juce::AudioFormatWriter> writer { wavAudioFormat.createWriterFor (outputSampleStream.release (), 44100.0, 1, 16, {}, 0) };
        jassert (writer != nullptr);
        if (writer == nullptr)
            return false;
        auto writeSuccess { writer->writeFromAudioSampleBuffer (audioBuffer, 0, audioBuffer.getNumSamples ()) };
        jassert (writeSuccess == true);
        if (writeSuccess == false)
            return false;
    }

    // write metadata
    auto outputSampleStream { outputSampleFile.createOutputStream () };
    jassert (outputSampleStream != nullptr && outputSampleStream->openedOk ());
    auto positionAtEndSuccess { outputSampleStream->setPosition (outputSampleFile.getSize()) };
    jassert (positionAtEndSuccess == true);
    // write new busy chunk identifier
    auto writeSuccess = outputSampleStream->write (kBusyChunkType, 4);
    jassert (writeSuccess == true);
    // write length
    uint32_t chunkLength { juce::ByteOrder::swapIfBigEndian (static_cast<uint32_t> (busyChunkData.getSize ())) };
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
