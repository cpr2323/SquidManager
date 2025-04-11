#include "SquidMetaDataWriter.h"
#include "BusyChunkWriter.h"
#include "SquidSalmpleDefs.h"
#include "../CvParameterProperties.h"
#include "../SquidChannelProperties.h"

bool SquidMetaDataWriter::write (juce::ValueTree squidChannelPropertiesVT, juce::File inputSampleFile, juce::File outputSampleFile)
{
    jassert (inputSampleFile != outputSampleFile);

    busyChunkData.setSize (SquidSalmple::DataLayout_190::kEndOfData, true);

    SquidChannelProperties squidChannelProperties { squidChannelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no };
    setUInt32 (static_cast<uint32_t> (kSignatureAndVersionCurrent), SquidSalmple::DataLayout_190::kBusyChunkSignatureAndVersionOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getAttack ()), SquidSalmple::DataLayout_190::kAttackOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getChannelFlags ()), SquidSalmple::DataLayout_190::kChannelFlagsOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getChannelSource ()), SquidSalmple::DataLayout_190::kChannelSourceOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getChoke ()), SquidSalmple::DataLayout_190::kChokeOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getBits ()), SquidSalmple::DataLayout_190::kQualityOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getDecay ()), SquidSalmple::DataLayout_190::kDecayOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getETrig ()), SquidSalmple::DataLayout_190::kExternalTriggerOffset);
    setUInt32 (static_cast<uint32_t> (squidChannelProperties.getEndCue ()), SquidSalmple::DataLayout_190::kSampleEndOffset);
    uint16_t frequencyAndType { static_cast<uint16_t> ((squidChannelProperties.getFilterFrequency () << 4) + squidChannelProperties.getFilterType ()) };
    setUInt16 (frequencyAndType, SquidSalmple::DataLayout_190::kCutoffFrequencyOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getFilterResonance ()), SquidSalmple::DataLayout_190::kResonanceOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getLevel ()), SquidSalmple::DataLayout_190::kLevelOffset);
    setUInt32 (static_cast<uint32_t> (squidChannelProperties.getLoopCue ()), SquidSalmple::DataLayout_190::kLoopPositionOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getLoopMode ()), SquidSalmple::DataLayout_190::kLoopOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getQuant ()), SquidSalmple::DataLayout_190::kQuantizeModeOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getPitchShift ()), SquidSalmple::DataLayout_190::kPitchShiftOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getRate ()), SquidSalmple::DataLayout_190::kRateOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getRecDest ()), SquidSalmple::DataLayout_190::kRecDestOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getReverse ()), SquidSalmple::DataLayout_190::kReverseOffset);
    setUInt32 (static_cast<uint32_t> (squidChannelProperties.getEndOfData ()), SquidSalmple::DataLayout_190::kEndOFDataOffset);
    if (squidChannelProperties.getChannelIndex () < 5)
        setUInt16 (static_cast<uint16_t> (squidChannelProperties.getSpeed ()), SquidSalmple::DataLayout_190::kSpeedOffset);
    else
        setUInt16 (static_cast<uint16_t> (32750), SquidSalmple::DataLayout_190::kSpeedOffset);
    setUInt32 (static_cast<uint32_t> (squidChannelProperties.getStartCue ()), SquidSalmple::DataLayout_190::kSampleStartOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getSteps ()), SquidSalmple::DataLayout_190::kStepTrigNumOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getXfade ()), SquidSalmple::DataLayout_190::kXfadeOffset);

    // CV Assigns
    const auto parameterRowSize { (kCvParamsCount_190 + kCvParamsExtra) * 4 };
    for (auto curCvInputIndex { 0 }; curCvInputIndex < kCvInputsCount + kCvInputsExtra; ++curCvInputIndex)
    {
        // we set bits in cvAssignedFlags for each parameter that has cv enabled
        uint32_t cvAssignedFlags { CvAssignedFlag::none };
        squidChannelProperties.forEachCvParameter (curCvInputIndex, [this, curCvInputIndex, parameterRowSize, &cvAssignedFlags, &squidChannelProperties] (juce::ValueTree srcParameterVT)
        {
            CvParameterProperties cvParameterProperties { srcParameterVT, CvParameterProperties::WrapperType::client, CvParameterProperties::EnableCallbacks::no };
            const auto parameterId { cvParameterProperties.getId () };

            const auto cvAssignedFlag { CvParameterIndex::getCvEnabledFlag (parameterId) };
            if (cvParameterProperties.getEnabled ())
                cvAssignedFlags |= cvAssignedFlag;
            const auto cvParamMetadataOffset { SquidSalmple::DataLayout_190::kCvParamsOffset + (curCvInputIndex * parameterRowSize) + (parameterId * 4) };
            const auto offset { static_cast<uint16_t> (cvParameterProperties.getOffset ()) };
            const auto attenuation { static_cast<uint16_t> (cvParameterProperties.getAttenuation ()) };
            setUInt16 (offset, cvParamMetadataOffset + 0);
            setUInt16 (attenuation, cvParamMetadataOffset + 2);
            return true;
        });
        // write cvAssignedFlags (bit flags for each enabled parameter) to metadata
        setUInt32 (cvAssignedFlags, SquidSalmple::DataLayout_190::kCvFlagsOffset + (curCvInputIndex * 4));
    };

    // Cue Sets
    const auto numCues { squidChannelProperties.getNumCueSets () };
    setUInt8 (static_cast<uint8_t> (numCues), SquidSalmple::DataLayout_190::kCuesCountOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getCurCueSet ()), SquidSalmple::DataLayout_190::kCuesSelectedOffset);
    for (auto curCueSet { 0 }; curCueSet < numCues; ++curCueSet)
    {
        setUInt32 (static_cast<uint32_t> (squidChannelProperties.getStartCueSet (curCueSet)), SquidSalmple::DataLayout_190::kCuesOffset + (curCueSet * 12) + 0);
        setUInt32 (static_cast<uint32_t> (squidChannelProperties.getEndCueSet (curCueSet)), SquidSalmple::DataLayout_190::kCuesOffset + (curCueSet * 12) + 4);
        setUInt32 (static_cast<uint32_t> (squidChannelProperties.getLoopCueSet (curCueSet)), SquidSalmple::DataLayout_190::kCuesOffset + (curCueSet * 12) + 8);
    }

    auto writeReserved = [this, &squidChannelProperties] (int reservedDataOffset, int reservedDataSize, std::function<juce::String ()> getter)
    {
        auto reservedData { getter () };
        juce::MemoryBlock tempMemory;
        tempMemory.fromBase64Encoding (reservedData);
        std::memcpy (static_cast<uint8_t*> (busyChunkData.getData ()) + reservedDataOffset, tempMemory.getData (), reservedDataSize);
    };
    // write out the 'reserved' sections
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved1Offset, SquidSalmple::DataLayout_190::k_Reserved1Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved1Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved2Offset, SquidSalmple::DataLayout_190::k_Reserved2Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved2Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved3Offset, SquidSalmple::DataLayout_190::k_Reserved3Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved3Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved4Offset, SquidSalmple::DataLayout_190::k_Reserved4Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved4Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved5Offset, SquidSalmple::DataLayout_190::k_Reserved5Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved5Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved6Offset, SquidSalmple::DataLayout_190::k_Reserved6Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved6Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved7Offset, SquidSalmple::DataLayout_190::k_Reserved7Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved7Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved8Offset, SquidSalmple::DataLayout_190::k_Reserved8Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved8Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved9Offset, SquidSalmple::DataLayout_190::k_Reserved9Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved9Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved10Offset, SquidSalmple::DataLayout_190::k_Reserved10Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved10Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved11Offset, SquidSalmple::DataLayout_190::k_Reserved11Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved11Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved12Offset, SquidSalmple::DataLayout_190::k_Reserved12Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved12Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved13Offset, SquidSalmple::DataLayout_190::k_Reserved13Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved13Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved14Offset, SquidSalmple::DataLayout_190::k_Reserved14Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved14Data (); });
    writeReserved (SquidSalmple::DataLayout_190::k_Reserved15Offset, SquidSalmple::DataLayout_190::k_Reserved15Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved15Data (); });

    BusyChunkWriter busyChunkWriter;
    auto audioBuffer { squidChannelProperties.getSampleDataAudioBuffer () };
    const auto writeSuccess { busyChunkWriter.write (*(squidChannelProperties.getSampleDataAudioBuffer ()->getAudioBuffer ()), outputSampleFile, busyChunkData) };
    jassert (writeSuccess == true);

    return true;
}

void SquidMetaDataWriter::setUInt8 (uint8_t value, int offset)
{
    busyChunkData [offset] = value;
}

void SquidMetaDataWriter::setUInt16 (uint16_t value, int offset)
{
    busyChunkData [offset]     = static_cast<uint8_t> (value);
    busyChunkData [offset + 1] = static_cast<uint8_t> (value >> 8);
}

void SquidMetaDataWriter::setUInt32 (uint32_t value, int offset)
{
    busyChunkData [offset + 0] = static_cast<uint8_t> (value);
    busyChunkData [offset + 1] = static_cast<uint8_t> (value >> 8);
    busyChunkData [offset + 2] = static_cast<uint8_t> (value >> 16);
    busyChunkData [offset + 3] = static_cast<uint8_t> (value >> 24);
}

void SquidMetaDataWriter::setUInt64 (uint64_t value, int offset)
{
    busyChunkData [offset + 0] = static_cast<uint8_t> (value);
    busyChunkData [offset + 1] = static_cast<uint8_t> (value >> 8);
    busyChunkData [offset + 2] = static_cast<uint8_t> (value >> 16);
    busyChunkData [offset + 3] = static_cast<uint8_t> (value >> 24);
    busyChunkData [offset + 4] = static_cast<uint8_t> (value >> 32);
    busyChunkData [offset + 5] = static_cast<uint8_t> (value >> 40);
    busyChunkData [offset + 6] = static_cast<uint8_t> (value >> 48);
    busyChunkData [offset + 7] = static_cast<uint8_t> (value >> 56);
}
