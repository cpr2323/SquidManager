#include "SquidMetaDataReader.h"
#include "BusyChunkReader.h"
#include "SquidSalmpleDefs.h"
#include "../SquidChannelProperties.h"
#include "../../Utility/DebugLog.h"

#define LOG_READER 1
#if LOG_READER
#define LogReader(text) DebugLog ("SquidMetaDataReader", text);
#else
#define LogReader(text) ;
#endif

juce::ValueTree SquidMetaDataReader::read (juce::File sampleFile)
{
    LogReader ("read - reading: " + juce::String (sampleFile.getFullPathName ()));

    auto numSamples { 0 };
    {
        juce::AudioFormatManager audioFormatManager;
        audioFormatManager.registerBasicFormats ();
        if (std::unique_ptr<juce::AudioFormatReader> sampleFileReader { audioFormatManager.createReaderFor (sampleFile) }; sampleFileReader != nullptr)
        {
            numSamples = sampleFileReader->lengthInSamples;
        }
    }

    BusyChunkReader busyChunkReader;
    busyChunkData.reset ();
    busyChunkReader.read (sampleFile, busyChunkData);
    //const auto rawChunkData { static_cast<uint8_t*>(busyChunkData.getData ()) };
    jassert (busyChunkData.getSize () == SquidSalmple::DataLayout::kEndOfData);
    const auto busyChunkVersion { getValue <SquidSalmple::DataLayout::kBusyChunkSignatureAndVersionSize> (SquidSalmple::DataLayout::kBusyChunkSignatureAndVersionOffset) };
    //jassert (busyChunkVersion == kSignatureAndVersionCurrent);
    if ((busyChunkVersion & 0xFFFFFF00) != (kSignatureAndVersionCurrent & 0xFFFFFF00))
    {
        juce::Logger::outputDebugString ("'busy' metadata chunk has wrong signature");
        jassertfalse;
        return {};
    }
    if ((busyChunkVersion & 0x000000FF) != (kSignatureAndVersionCurrent & 0x000000FF))
        juce::Logger::outputDebugString ("Version mismatch. version read in: " + juce::String (busyChunkVersion & 0x000000FF) + ". expected version: " + juce::String (kSignatureAndVersionCurrent & 0x000000FF));

    SquidChannelProperties squidChannelProperties { {}, SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no };

    squidChannelProperties.setAttack (getValue <SquidSalmple::DataLayout::kAttackSize> (SquidSalmple::DataLayout::kAttackOffset), false);
    squidChannelProperties.setBits (getValue <SquidSalmple::DataLayout::kQualitySize> (SquidSalmple::DataLayout::kQualityOffset), false);
    squidChannelProperties.setChannelFlags (getValue <SquidSalmple::DataLayout::kChannelFlagsSize> (SquidSalmple::DataLayout::kChannelFlagsOffset), false);
    squidChannelProperties.setChoke (getValue <SquidSalmple::DataLayout::kChokeSize> (SquidSalmple::DataLayout::kChokeOffset), false);
    squidChannelProperties.setDecay (getValue <SquidSalmple::DataLayout::kDecaySize> (SquidSalmple::DataLayout::kDecayOffset), false);
    squidChannelProperties.setEndCue (getValue <SquidSalmple::DataLayout::kSampleEndSize> (SquidSalmple::DataLayout::kSampleEndOffset), false);
    squidChannelProperties.setETrig (getValue <SquidSalmple::DataLayout::kExternalTriggerSize> (SquidSalmple::DataLayout::kExternalTriggerOffset), false);
    uint16_t frequencyAndType { getValue <SquidSalmple::DataLayout::kCutoffFrequencySize> (SquidSalmple::DataLayout::kCutoffFrequencyOffset) };
    squidChannelProperties.setFilterType (frequencyAndType & 0x000F, false);
    squidChannelProperties.setFilterFrequency (frequencyAndType >> 4, false);
    squidChannelProperties.setFilterResonance (getValue <SquidSalmple::DataLayout::kResonanceSize> (SquidSalmple::DataLayout::kResonanceOffset), false);
    squidChannelProperties.setLevel (getValue <SquidSalmple::DataLayout::kLevelSize> (SquidSalmple::DataLayout::kLevelOffset), false);
    squidChannelProperties.setLoopCue (getValue <SquidSalmple::DataLayout::kLoopPositionSize> (SquidSalmple::DataLayout::kLoopPositionOffset), false);
    squidChannelProperties.setLoopMode (getValue <SquidSalmple::DataLayout::kLoopSize> (SquidSalmple::DataLayout::kLoopOffset), false);
    squidChannelProperties.setQuant (getValue <SquidSalmple::DataLayout::kQuantizeModeSize> (SquidSalmple::DataLayout::kQuantizeModeOffset), false);
    squidChannelProperties.setRate (getValue <SquidSalmple::DataLayout::kRateSize> (SquidSalmple::DataLayout::kRateOffset), false);
    squidChannelProperties.setReverse (getValue <SquidSalmple::DataLayout::kReverseSize> (SquidSalmple::DataLayout::kReverseOffset), false);
    squidChannelProperties.setSpeed (getValue <SquidSalmple::DataLayout::kSpeedSize> (SquidSalmple::DataLayout::kSpeedOffset), false);
    squidChannelProperties.setStartCue (getValue <SquidSalmple::DataLayout::kSampleStartSize> (SquidSalmple::DataLayout::kSampleStartOffset), false);
    squidChannelProperties.setSteps (getValue<SquidSalmple::DataLayout::kStepTrigNumSize> (SquidSalmple::DataLayout::kStepTrigNumOffset), false);
    squidChannelProperties.setXfade (getValue <SquidSalmple::DataLayout::kXfadeSize> (SquidSalmple::DataLayout::kXfadeOffset), false);

    ////////////////////////////////////
    // cv assign
    const auto rowSize { (kCvParamsCount + kCvParamsExtra) * 4 };
    for (auto curCvInputIndex { 0 }; curCvInputIndex < kCvInputsCount + kCvInputsExtra; ++curCvInputIndex)
    {
        for (auto curParameterIndex { 0 }; curParameterIndex < 15; ++curParameterIndex)
        {
            juce::ValueTree parameterVT { squidChannelProperties.getCvParameterVT (curCvInputIndex, curParameterIndex) };
            const auto cvParamOffset { SquidSalmple::DataLayout::kCvParamsOffset + (curCvInputIndex * rowSize) + (curParameterIndex * 4) };
            const auto cvAssignedFlag { CvParameterIndex::getCvEnabledFlag (curParameterIndex) };
            const auto cvAssignFlags { getValue <2> (SquidSalmple::DataLayout::kCvFlagsOffset + (2 * curCvInputIndex)) };
            const auto offset { getValue <2> (cvParamOffset + 0) };
            const auto attenuation { static_cast<int16_t>(getValue <2> (cvParamOffset + 2)) };
            parameterVT.setProperty (SquidChannelProperties::CvAssignInputParameterEnabledPropertyId, cvAssignFlags & cvAssignedFlag ? "true" : "false", nullptr);
            parameterVT.setProperty (SquidChannelProperties::CvAssignInputParameterAttenuatePropertyId, attenuation, nullptr);
            parameterVT.setProperty (SquidChannelProperties::CvAssignInputParameterOffsetPropertyId, offset, nullptr);
        }
    }

    ////////////////////////////////////
    // cue sets
    const auto playPosition { getValue <SquidSalmple::DataLayout::k_Reserved2Size> (SquidSalmple::DataLayout::k_Reserved2Offset) };
    LogReader ("read - play position meta data = " + juce::String (playPosition).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (playPosition).paddedLeft ('0', 6) + "]");
    const auto endOfSample { getValue <SquidSalmple::DataLayout::kEndOfSampleSize> (SquidSalmple::DataLayout::kEndOfSampleOffset) };
    LogReader ("read - end of sample meta data = " + juce::String (endOfSample).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (endOfSample).paddedLeft ('0', 6) + "], ratio from numSamples = " + juce::String(endOfSample / numSamples));
    auto logCueSet = [this, numSamples] (uint8_t cueSetIndex, uint32_t startCue, uint32_t loopCue, uint32_t endCue)
    {
        LogReader ("read - cue set " + juce::String (cueSetIndex) + ": start = " + juce::String (startCue).paddedLeft('0', 6) + " [0x" + juce::String::toHexString (startCue).paddedLeft ('0', 6) + "], loop = " +
                   juce::String (loopCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (loopCue).paddedLeft ('0', 6) + "], end = " + juce::String (endCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (endCue).paddedLeft ('0', 6) + "]" +
                   ", ratio from numSamples = " + juce::String (endCue / numSamples));
        jassert (startCue <= loopCue && loopCue < endCue);
    };

    const auto numCues { getValue <SquidSalmple::DataLayout::kCuesCountSize> (SquidSalmple::DataLayout::kCuesCountOffset) };
    const auto curCue { getValue <SquidSalmple::DataLayout::kCuesSelectedSize> (SquidSalmple::DataLayout::kCuesSelectedOffset) };
    //squidChannelProperties.setNumCueSets (numCues, false); // don't do this, because the count is updated below by squidChannelProperties.addCueSet
    LogReader ("read - cur cue meta data (cue set " + juce::String(curCue) + "):");
    logCueSet (0, squidChannelProperties.getStartCue (), squidChannelProperties.getLoopCue (), squidChannelProperties.getEndCue ());

    LogReader ("read - Cue List: " + juce::String (numCues));
    for (auto curCueSetIndex { 0 }; curCueSetIndex < numCues; ++curCueSetIndex)
    {
        const auto cueSetOffset { SquidSalmple::DataLayout::kCuesOffset + (curCueSetIndex * 12) };
        const auto startCue { getValue <4> (cueSetOffset + 0) };
        const auto endCue { getValue <4> (cueSetOffset + 4) };
        const auto loopCue { getValue <4> (cueSetOffset + 8) };
        logCueSet (curCueSetIndex, startCue, loopCue, endCue);
        squidChannelProperties.setCuePoints (curCueSetIndex, startCue, loopCue, endCue);
    }
    squidChannelProperties.setCurCueSet (curCue, false);

    auto readReserved = [this, &squidChannelProperties] (int reservedDataOffset, int reservedDataSize, std::function<void(juce::String)> setter)
    {
        juce::MemoryBlock tempMemory;
        tempMemory.replaceAll (static_cast<uint8_t*>(busyChunkData.getData ()) + reservedDataOffset, reservedDataSize);
        auto textVersion { tempMemory.toBase64Encoding () };
        setter (textVersion);
    };
    // read and store the 'reserved' sections
    readReserved (SquidSalmple::DataLayout::k_Reserved1Offset, SquidSalmple::DataLayout::k_Reserved1Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved1Data (reservedData); } );
    readReserved (SquidSalmple::DataLayout::k_Reserved2Offset, SquidSalmple::DataLayout::k_Reserved2Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved2Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved3Offset, SquidSalmple::DataLayout::k_Reserved3Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved3Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved4Offset, SquidSalmple::DataLayout::k_Reserved4Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved4Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved5Offset, SquidSalmple::DataLayout::k_Reserved5Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved5Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved6Offset, SquidSalmple::DataLayout::k_Reserved6Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved6Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved7Offset, SquidSalmple::DataLayout::k_Reserved7Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved7Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved8Offset, SquidSalmple::DataLayout::k_Reserved8Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved8Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved9Offset, SquidSalmple::DataLayout::k_Reserved9Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved9Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved10Offset, SquidSalmple::DataLayout::k_Reserved10Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved10Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved11Offset, SquidSalmple::DataLayout::k_Reserved11Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved11Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved12Offset, SquidSalmple::DataLayout::k_Reserved12Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved12Data (reservedData); });
    readReserved (SquidSalmple::DataLayout::k_Reserved13Offset, SquidSalmple::DataLayout::k_Reserved13Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved13Data (reservedData); });

    return squidChannelProperties.getValueTree ();
}