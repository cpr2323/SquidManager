#include "BusyChunkWriter.h"

bool BusyChunkWriter::write (juce::File sampleFile, juce::MemoryBlock& busyChunkData)
{
    const auto tempName { "_output_" + sampleFile.getFileNameWithoutExtension () };
    auto outputFile { sampleFile.getParentDirectory ().getChildFile (tempName) };
    {
        // open input file
        // TODO - handle error conditions
        auto sampleInputStream { sampleFile.createInputStream () };
        jassert (sampleInputStream != nullptr && sampleInputStream->openedOk ());

        // create/open temp file
        auto sampleOutputStream { outputFile.createOutputStream () };
        jassert (sampleOutputStream != nullptr && sampleOutputStream->openedOk ());

        while (true)
        {
            auto chunk { getChunkData (sampleInputStream.get ()) };
            if (!chunk.has_value ())
                break;
            auto chunkInfo { chunk.value () };

            // write chunk header
            auto writeSuccess { sampleOutputStream->write (chunkInfo.chunkType, 4) };
            jassert (writeSuccess == true);
            uint32_t chunkLength { juce::ByteOrder::swapIfBigEndian (chunkInfo.chunkLength) };
            writeSuccess = sampleOutputStream->write (&chunkLength, 4);
            jassert (writeSuccess == true);

            if (std::memcmp (kBusyChunkType, chunkInfo.chunkType, 4) == 0)
            {
                // write new busyChunk
                writeSuccess = sampleOutputStream->write (busyChunkData.getData (), busyChunkData.getSize ());
                jassert (writeSuccess == true);
            }
            else
            {
                juce::MemoryBlock chunkData;
                chunkData.setSize (chunkInfo.chunkLength);
                // read chunk data from input file
                const auto bytesRead { sampleInputStream->read (&chunkData, chunkInfo.chunkLength) };
                jassert (bytesRead == static_cast<int> (chunkInfo.chunkLength));
                // write chunk data to output file
                writeSuccess = sampleOutputStream->write (chunkData.getData (), chunkData.getSize ());
                jassert (writeSuccess == true);
            }
        }
    }

    // input and output streams have been closed by exiting scope

    // rename input file
    const auto originalName { sampleFile.getFullPathName () };
    const auto backupName { "_backup_" + sampleFile.getFileNameWithoutExtension () };
    auto success { sampleFile.moveFileTo (sampleFile.getParentDirectory ().getChildFile (backupName)) };
    jassert (success == true);
    // rename output file
    auto success2 { outputFile.moveFileTo (sampleFile.getParentDirectory ().getChildFile (originalName)) };
    jassert (success2 == true);
    // delete renamed input file
    sampleFile.deleteFile ();

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
