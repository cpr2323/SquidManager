#include "SquidMetaDataWriter.h"
#include "SquidSalmpleDefs.h"

bool SquidMetaDataWriter::write (juce::ValueTree squidMetaDataPropertiesVT, juce::File inputSampleFile, juce::File outputSampleFile)
{
    jassert (inputSampleFile != outputSampleFile);

    busyChunkData.setSize (SquidSalmple::DataLayout::kEndOfData, true);

    SquidMetaDataProperties squidMetaDataProperties { squidMetaDataPropertiesVT, SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::no };
    setUInt32 (static_cast<uint32_t> (kSignatureAndVersionCurrent), SquidSalmple::DataLayout::kBusyChunkSignatureAndVersionOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getAttack ()), SquidSalmple::DataLayout::kAttackOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getChoke ()), SquidSalmple::DataLayout::kChokeOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getBits ()), SquidSalmple::DataLayout::kQualityOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getDecay ()), SquidSalmple::DataLayout::kDecayOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getETrig ()), SquidSalmple::DataLayout::kExternalTriggerOffset);
    setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getEndCue ()), SquidSalmple::DataLayout::kSampleEndOffset);
    uint16_t frequencyAndType { static_cast<uint16_t> ((squidMetaDataProperties.getFilterFrequency () << 4) + squidMetaDataProperties.getFilterType ()) };
    setUInt16 (frequencyAndType, SquidSalmple::DataLayout::kCutoffFrequencyOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getFilterResonance ()), SquidSalmple::DataLayout::kResonanceOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getLevel ()), SquidSalmple::DataLayout::kLevelOffset);
    setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getLoopCue ()), SquidSalmple::DataLayout::kLoopPositionOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getLoopMode ()), SquidSalmple::DataLayout::kLoopOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getQuant ()), SquidSalmple::DataLayout::kQuantizeModeOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getRate ()), SquidSalmple::DataLayout::kRateOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getReverse ()), SquidSalmple::DataLayout::kReverseOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getSpeed ()), SquidSalmple::DataLayout::kSpeedOffset);
    setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getStartCue ()), SquidSalmple::DataLayout::kSampleStartOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getSteps ()), SquidSalmple::DataLayout::kStepTrigNumOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getXfade ()), SquidSalmple::DataLayout::kXfadeOffset);

    // CV Assigns
    auto cvAssignsVT { squidMetaDataProperties.getValueTree ().getChildWithName ("CvAssigns") };
    jassert (cvAssignsVT.isValid ());
    ValueTreeHelpers::forEachChildOfType (cvAssignsVT, "CvInput", [this] (juce::ValueTree cvInputVT)
    {
        const auto cvId { static_cast<int> (cvInputVT.getProperty ("id")) };
        juce::Logger::outputDebugString ("processing cvInput #" + juce::String (cvId));
        uint16_t cvAssignedFlags { CvAssignedFlag::none };
        const auto parameterRowSize { (kCvParamsCount + kCvParamsExtra) * 2 };
        ValueTreeHelpers::forEachChildOfType (cvInputVT, "Parameter", [this, cvId, parameterRowSize, &cvAssignedFlags] (juce::ValueTree parameterVT)
        {
            const auto parameterName { parameterVT.getProperty ("name").toString () };
            const auto enabled { static_cast<bool> (parameterVT.getProperty ("enabled")) };
            const auto offset { static_cast<int> (parameterVT.getProperty ("offset")) };
            auto attenuation { static_cast<int> (parameterVT.getProperty ("attenuate")) };
            // NOTE - the value stored internally is 0 to 199, externally we have -99 to 99
            //        so, 0 to 99 external maps 0-99 internal, but -0 to -99 externally maps to 100 - 199 internally, ie. external value = value < 100 ? value : 100 - value
            // currently I am storing the -99 to 99 range in the data model, which means we loose the 0 that is 100
            // I think I should change this so the data model also stores 0 to 199, to keep the operation of the software the same as the firmware
            if (attenuation < 0)
                attenuation = 100 + std::abs (attenuation);
            juce::Logger::outputDebugString (" processing parameter: " + parameterName);
            auto getFlagBitParamIndexFromParameterName = [] (juce::String parameterName) -> std::tuple<uint16_t, uint8_t>
            {
                if (parameterName == "bits")
                    return { CvAssignedFlag::bits, 0 };
                if (parameterName == "rate")
                    return { CvAssignedFlag::rate, 1 };
                if (parameterName == "level")
                    return { CvAssignedFlag::level, 2 };
                if (parameterName == "decay")
                    return { CvAssignedFlag::decay, 3 };
                if (parameterName == "speed")
                    return { CvAssignedFlag::speed, 4 };
                if (parameterName == "loopMode")
                    return { CvAssignedFlag::loopMode, 5 };
                if (parameterName == "reverse")
                    return { CvAssignedFlag::reverse, 6 };
                if (parameterName == "startCue")
                    return { CvAssignedFlag::startCue, 7 };
                if (parameterName == "endCue")
                    return { CvAssignedFlag::endCue, 8 };
                if (parameterName == "loopCue")
                    return { CvAssignedFlag::loopCue, 9 };
                if (parameterName == "attack")
                    return { CvAssignedFlag::attack, 10 };
                if (parameterName == "cue Set")
                    return { CvAssignedFlag::cueSet, 11 };
                if (parameterName == "eTrig")
                    return { CvAssignedFlag::eTrig, 12 };
                if (parameterName == "filterFrequency")
                    return { CvAssignedFlag::filtFreq, 13 };
                if (parameterName == "filterResonance")
                    return { CvAssignedFlag::filtRes, 14 };
                // unknown parameter name
                jassertfalse;
                return { CvAssignedFlag::none, 255 };
            };
            const auto [cvAssignedFlag, cvParamIndex] { getFlagBitParamIndexFromParameterName (parameterName) };
    
            if (enabled)
                cvAssignedFlags |= cvAssignedFlag;
            
            const auto cvParamOffset { SquidSalmple::DataLayout::kCvParamsOffset + ((cvId - 1) * parameterRowSize) + (cvParamIndex* 4) };
            setUInt16 (offset, cvParamOffset + 0);
            setUInt16 (attenuation, cvParamOffset + 2);
            return true;
        });
        // write CvFlags to MemoryBlock
        setUInt16 (cvAssignedFlags, SquidSalmple::DataLayout::kCvFlagsOffset + (cvId - 1) * 2);
        return true;
    });

    // Cue Sets
    const auto numCues { squidMetaDataProperties.getNumCues () };
    setUInt8 (static_cast<uint8_t> (numCues), SquidSalmple::DataLayout::kCuesCountOffset);
    for (auto curCueSet { 0 }; curCueSet < numCues; ++curCueSet)
    {
        setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getStartCueSet (curCueSet)), SquidSalmple::DataLayout::kCuesOffset + (curCueSet * 12) + 0);
        setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getEndCueSet (curCueSet)), SquidSalmple::DataLayout::kCuesOffset + (curCueSet * 12) + 4);
        setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getLoopCueSet (curCueSet)), SquidSalmple::DataLayout::kCuesOffset + (curCueSet * 12) + 8);
    }

    auto writeReserved = [this, &squidMetaDataProperties] (int reservedDataOffset, int reservedDataSize, std::function<juce::String ()> getter)
    {
        auto reservedData { getter () };
        juce::MemoryBlock tempMemory;
        tempMemory.fromBase64Encoding (reservedData);
        std::memcpy (static_cast<uint8_t*>(busyChunkData.getData ()) + reservedDataOffset, tempMemory.getData (), reservedDataSize);
    };
    // write out the 'reserved' sections
    writeReserved (SquidSalmple::DataLayout::k_Reserved1Offset, SquidSalmple::DataLayout::k_Reserved1Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved1Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved2Offset, SquidSalmple::DataLayout::k_Reserved2Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved2Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved3Offset, SquidSalmple::DataLayout::k_Reserved3Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved3Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved4Offset, SquidSalmple::DataLayout::k_Reserved4Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved4Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved5Offset, SquidSalmple::DataLayout::k_Reserved5Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved5Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved6Offset, SquidSalmple::DataLayout::k_Reserved6Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved6Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved7Offset, SquidSalmple::DataLayout::k_Reserved7Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved7Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved8Offset, SquidSalmple::DataLayout::k_Reserved8Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved8Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved9Offset, SquidSalmple::DataLayout::k_Reserved9Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved9Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved10Offset, SquidSalmple::DataLayout::k_Reserved10Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved10Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved11Offset, SquidSalmple::DataLayout::k_Reserved11Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved11Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved12Offset, SquidSalmple::DataLayout::k_Reserved12Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved12Data (); });
    writeReserved (SquidSalmple::DataLayout::k_Reserved13Offset, SquidSalmple::DataLayout::k_Reserved13Size, [&squidMetaDataProperties] () { return squidMetaDataProperties.getReserved13Data (); });

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
