#include "BusyChunkWriter.h"

// TODO - handle error conditions
bool BusyChunkWriter::write (juce::AudioBuffer<float>& audioBuffer, juce::File outputSampleFile, juce::MemoryBlock& busyChunkData)
{
    // write audio data
    {
        // create/open temp file. openedOk () and truncate () are FileOutputStream only, so the
        // setup has to happen while the pointer still has that type
        auto sampleFileStream { outputSampleFile.createOutputStream () };
        jassert (sampleFileStream != nullptr && sampleFileStream->openedOk ());
        sampleFileStream->setPosition (0);
        sampleFileStream->truncate ();
        // createWriterFor takes a unique_ptr<OutputStream>&, and a unique_ptr<FileOutputStream>
        // cannot bind to a reference to a different type, so the stream is moved into a base typed
        // pointer to hand over. this is the same stream: sampleFileStream is null from here on
        std::unique_ptr<juce::OutputStream> streamForWriter { std::move (sampleFileStream) };
        juce::WavAudioFormat wavAudioFormat;
        // on success, the writer takes ownership of the output stream, and will delete it when done
        auto writer { wavAudioFormat.createWriterFor (streamForWriter, juce::AudioFormatWriterOptions {}.withSampleRate (44100.0)
                                                                                                       .withNumChannels (1)
                                                                                                       .withBitsPerSample (16)) };
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
    auto positionAtEndSuccess { outputSampleStream->setPosition (outputSampleFile.getSize ()) };
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
