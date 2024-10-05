#include "BusyChunkReader.h"

bool BusyChunkReader::readMetaData (juce::File sampleFile, juce::MemoryBlock& busyChunkData)
{
    // TODO - handle error conditions
    auto sampleInputStream { sampleFile.createInputStream () };
    jassert (sampleInputStream != nullptr && sampleInputStream->openedOk ());
    auto busyChunkLocated { findChunk (sampleInputStream.get (), kBusyChunkType) };
    if (! busyChunkLocated.has_value ())
        return false;
    sampleInputStream->readIntoMemoryBlock (busyChunkData, busyChunkLocated.value ());
    return true;
}

BusyChunkReader::MarkerList BusyChunkReader::getMarkerList (juce::File sampleFile)
{
    auto sampleInputStream { sampleFile.createInputStream () };
    jassert (sampleInputStream != nullptr && sampleInputStream->openedOk ());
    // locate RIFF chunk (should be first?)
    auto riffChunkLocated { findChunk (sampleInputStream.get (), kRIFFChunkType) };
    if (! riffChunkLocated.has_value ())
        return {};
    // read RIFF format identifier
    char riffFormat [4];
    if (sampleInputStream->read (&riffFormat, 4) != 4)
        return {};
    // verify is WAVE format
    if (std::memcmp (kWAVEFormatId, riffFormat, 4) != 0)
        return {};
    // locate the marker list chunk
    auto markersChunkLocated { findChunk (sampleInputStream.get (), kMarkerListChunkType) };
    if (! markersChunkLocated.has_value ())
        return {};
    // read in the entire markers chunk
    juce::MemoryBlock markersChunkData;
    auto bytesRead { sampleInputStream->readIntoMemoryBlock (markersChunkData, markersChunkLocated.value ()) };
    if (bytesRead != markersChunkLocated.value ())
        return {};
    uint8_t* markersChunkDataPtr { static_cast<uint8_t*> (markersChunkData.getData ()) };
    // first 4 bytes are number of markers (little endian)
    uint32_t numMarkers { juce::ByteOrder::swapIfBigEndian (*reinterpret_cast<uint32_t*> (markersChunkDataPtr)) };
    markersChunkDataPtr += sizeof (uint32_t);
    MarkerList markerList;
    for (uint32_t curMarker { 0 }; curMarker < numMarkers; ++curMarker)
    {
        // typedef struct {
        //     long    dwIdentifier;
        //     long    dwPosition;
        //     ID      fccChunk;
        //     long    dwChunkStart;
        //     long    dwBlockStart;
        //     long    dwSampleOffset;
        // } CuePoint;
        //
        // I am assuming that the Squid only reads the most simple of WAV files, and does not use the Playlist Chunk
        // which means that all I need to grab out of here is the dwPosition (sample position) value
        // first lets skip over the dwIndentifier field
        markersChunkDataPtr += sizeof (uint32_t);
        // put sample offset into list
        markerList.emplace_back (juce::ByteOrder::swapIfBigEndian (*reinterpret_cast<uint32_t*> (markersChunkDataPtr)));
        // skip over sample offset and remaining data members
        constexpr auto kSizeOfFccChunkID { 4 };
        markersChunkDataPtr += sizeof (uint32_t) + kSizeOfFccChunkID + (sizeof (uint32_t) * 3);
    }
    return markerList; // return dummy list for test
}

std::optional<uint32_t> BusyChunkReader::findChunk (juce::InputStream* is, char* chunkType)
{
    auto chunkLocated { false };
    auto chunkLength { 0 };
    while (! chunkLocated)
    {
        const auto chunk { getChunkData (is) };
        if (! chunk.has_value ())
            break;
        const auto chunkInfo { chunk.value () };
        if (std::memcmp (chunkType, chunkInfo.chunkType, 4) != 0)
        {
            const auto bytesToSkip { (chunkInfo.chunkLength & 1) == 0 ? chunkInfo.chunkLength : chunkInfo.chunkLength + 1};
            const auto seekSuccess { is->setPosition (is->getPosition () + bytesToSkip) };
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
