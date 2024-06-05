#include "SquidMetaDataWriter.h"
#include "BusyChunkWriter.h"
#include "SquidSalmpleDefs.h"
#include "../SquidChannelProperties.h"

bool SquidMetaDataWriter::write (juce::ValueTree squidChannelPropertiesVT, juce::File inputSampleFile, juce::File outputSampleFile)
{
    jassert (inputSampleFile != outputSampleFile);

    busyChunkData.setSize (SquidSalmple::DataLayout::kEndOfData, true);

    SquidChannelProperties squidChannelProperties { squidChannelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no };
    setUInt32 (static_cast<uint32_t> (kSignatureAndVersionCurrent), SquidSalmple::DataLayout::kBusyChunkSignatureAndVersionOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getAttack ()), SquidSalmple::DataLayout::kAttackOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getChannelFlags ()), SquidSalmple::DataLayout::kChannelFlagsOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getChannelSource ()), SquidSalmple::DataLayout::kChannelSourceOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getChoke ()), SquidSalmple::DataLayout::kChokeOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getBits ()), SquidSalmple::DataLayout::kQualityOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getDecay ()), SquidSalmple::DataLayout::kDecayOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getETrig ()), SquidSalmple::DataLayout::kExternalTriggerOffset);
    setUInt32 (static_cast<uint32_t> (squidChannelProperties.getEndCue ()), SquidSalmple::DataLayout::kSampleEndOffset);
    uint16_t frequencyAndType { static_cast<uint16_t> ((squidChannelProperties.getFilterFrequency () << 4) + squidChannelProperties.getFilterType ()) };
    setUInt16 (frequencyAndType, SquidSalmple::DataLayout::kCutoffFrequencyOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getFilterResonance ()), SquidSalmple::DataLayout::kResonanceOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getLevel ()), SquidSalmple::DataLayout::kLevelOffset);
    setUInt32 (static_cast<uint32_t> (squidChannelProperties.getLoopCue ()), SquidSalmple::DataLayout::kLoopPositionOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getLoopMode ()), SquidSalmple::DataLayout::kLoopOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getQuant ()), SquidSalmple::DataLayout::kQuantizeModeOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getRate ()), SquidSalmple::DataLayout::kRateOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getReverse ()), SquidSalmple::DataLayout::kReverseOffset);
    setUInt32 (static_cast<uint32_t> (squidChannelProperties.getSampleLength ()), SquidSalmple::DataLayout::kSampleLengthOffset);
    setUInt16 (static_cast<uint16_t> (squidChannelProperties.getSpeed ()), SquidSalmple::DataLayout::kSpeedOffset);
    setUInt32 (static_cast<uint32_t> (squidChannelProperties.getStartCue ()), SquidSalmple::DataLayout::kSampleStartOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getSteps ()), SquidSalmple::DataLayout::kStepTrigNumOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getXfade ()), SquidSalmple::DataLayout::kXfadeOffset);

    // CV Assigns
    const auto parameterRowSize { (kCvParamsCount + kCvParamsExtra) * 4 };
    for (auto curCvInputIndex { 0 }; curCvInputIndex < kCvInputsCount + kCvInputsExtra; ++curCvInputIndex)
    {
        // we set bits in cvAssignedFlags for each parameter that has cv enabled
        uint16_t cvAssignedFlags { CvAssignedFlag::none };
        for (auto curParameterIndex { 0 }; curParameterIndex < 15; ++curParameterIndex)
        {
            juce::ValueTree parameterVT { squidChannelProperties.getCvParameterVT (curCvInputIndex, curParameterIndex) };
            const auto cvParamOffset { SquidSalmple::DataLayout::kCvParamsOffset + (curCvInputIndex * parameterRowSize) + (curParameterIndex * 4) };
            const auto cvAssignedFlag { CvParameterIndex::getCvEnabledFlag (curParameterIndex) };
            const auto enabled { static_cast<bool> (parameterVT.getProperty (SquidChannelProperties::CvAssignInputParameterEnabledPropertyId)) };
            const auto offset { static_cast<int> (parameterVT.getProperty (SquidChannelProperties::CvAssignInputParameterOffsetPropertyId)) };
            const auto attenuation { static_cast<int> (parameterVT.getProperty (SquidChannelProperties::CvAssignInputParameterAttenuatePropertyId)) };
            if (enabled)
                cvAssignedFlags |= cvAssignedFlag;
            setUInt16 (offset, cvParamOffset + 0);
            setUInt16 (attenuation, cvParamOffset + 2);
//            return true;
        };
        // write cvAssignedFlags (bit flags for each enabled parameter) to metadata
        setUInt16 (cvAssignedFlags, SquidSalmple::DataLayout::kCvFlagsOffset + (curCvInputIndex * 2));
//        return true;
    };

    // Cue Sets
    const auto numCues { squidChannelProperties.getNumCueSets () };
    setUInt8 (static_cast<uint8_t> (numCues), SquidSalmple::DataLayout::kCuesCountOffset);
    setUInt8 (static_cast<uint8_t> (squidChannelProperties.getCurCueSet()), SquidSalmple::DataLayout::kCuesSelectedOffset);
    for (auto curCueSet { 0 }; curCueSet < numCues; ++curCueSet)
    {
        setUInt32 (static_cast<uint32_t> (squidChannelProperties.getStartCueSet (curCueSet)), SquidSalmple::DataLayout::kCuesOffset + (curCueSet * 12) + 0);
        setUInt32 (static_cast<uint32_t> (squidChannelProperties.getEndCueSet (curCueSet)), SquidSalmple::DataLayout::kCuesOffset + (curCueSet * 12) + 4);
        setUInt32 (static_cast<uint32_t> (squidChannelProperties.getLoopCueSet (curCueSet)), SquidSalmple::DataLayout::kCuesOffset + (curCueSet * 12) + 8);
    }

    auto writeReserved = [this, &squidChannelProperties] (int reservedDataOffset, int reservedDataSize, std::function<juce::String ()> getter)
    {
        auto reservedData { getter () };
        juce::MemoryBlock tempMemory;
        tempMemory.fromBase64Encoding (reservedData);
        std::memcpy (static_cast<uint8_t*>(busyChunkData.getData ()) + reservedDataOffset, tempMemory.getData (), reservedDataSize);
    };
    // write out the 'reserved' sections
    writeReserved (SquidSalmple::DataLayout::k_Reserved1Offset, SquidSalmple::DataLayout::k_Reserved1Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved1Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved2Offset, SquidSalmple::DataLayout::k_Reserved2Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved2Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved3Offset, SquidSalmple::DataLayout::k_Reserved3Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved3Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved4Offset, SquidSalmple::DataLayout::k_Reserved4Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved4Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved5Offset, SquidSalmple::DataLayout::k_Reserved5Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved5Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved6Offset, SquidSalmple::DataLayout::k_Reserved6Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved6Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved7Offset, SquidSalmple::DataLayout::k_Reserved7Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved7Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved8Offset, SquidSalmple::DataLayout::k_Reserved8Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved8Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved9Offset, SquidSalmple::DataLayout::k_Reserved9Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved9Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved10Offset, SquidSalmple::DataLayout::k_Reserved10Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved10Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved11Offset, SquidSalmple::DataLayout::k_Reserved11Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved11Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved12Offset, SquidSalmple::DataLayout::k_Reserved12Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved12Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved13Offset, SquidSalmple::DataLayout::k_Reserved13Size, [&squidChannelProperties] () { return squidChannelProperties.getReserved13Data (); });

    BusyChunkWriter busyChunkWriter;
    const auto writeSuccess { busyChunkWriter.write (inputSampleFile, outputSampleFile, busyChunkData) };
    jassert (writeSuccess == true);

    return true;
}

void SquidMetaDataWriter::setUInt8 (uint8_t value, int offset)
{
    busyChunkData [offset] = value;
}

void SquidMetaDataWriter::setUInt16 (uint16_t value, int offset)
{
    busyChunkData [offset]     = static_cast<uint8_t>(value); 
    busyChunkData [offset + 1] = static_cast<uint8_t>(value >> 8);
}

void SquidMetaDataWriter::setUInt32 (uint32_t value, int offset)
{
    busyChunkData [offset + 0] = static_cast<uint8_t>(value); 
    busyChunkData [offset + 1] = static_cast<uint8_t>(value >> 8);
    busyChunkData [offset + 2] = static_cast<uint8_t>(value >> 16); 
    busyChunkData [offset + 3] = static_cast<uint8_t>(value >> 24);
}

void SquidMetaDataWriter::setUInt64 (uint64_t value, int offset)
{
    busyChunkData [offset + 0] = static_cast<uint8_t>(value); 
    busyChunkData [offset + 1] = static_cast<uint8_t>(value >> 8); 
    busyChunkData [offset + 2] = static_cast<uint8_t>(value >> 16); 
    busyChunkData [offset + 3] = static_cast<uint8_t>(value >> 24); 
    busyChunkData [offset + 4] = static_cast<uint8_t>(value >> 32);
    busyChunkData [offset + 5] = static_cast<uint8_t>(value >> 40);
    busyChunkData [offset + 6] = static_cast<uint8_t>(value >> 48);
    busyChunkData [offset + 7] = static_cast<uint8_t>(value >> 56);
}
