#include "SquidMetaDataReader.h"
#include "BusyChunkReader.h"
#include "SquidSalmpleDefs.h"
#include "../CvParameterProperties.h"
#include "../SquidChannelProperties.h"
#include "../../Utility/DebugLog.h"
#include "../../Utility/DumpStack.h"

#define LOG_READER 1
#if LOG_READER
#define LogReader(text) DebugLog ("SquidMetaDataReader", text);
#else
#define LogReader(text) ;
#endif

enum class MetaDataStatus
{
    invalid,
    fw186,
    latest
};

void SquidMetaDataReader::read (juce::ValueTree channelPropertiesVT, juce::File sampleFile, uint8_t channelIndex)
{
    LogReader ("read - reading: " + juce::String (sampleFile.getFullPathName ()));
    SquidChannelProperties squidChannelProperties { channelPropertiesVT, SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no };
    BusyChunkReader busyChunkReader;
    busyChunkData.reset ();
    auto metaDataStatus { MetaDataStatus::invalid };
    if (busyChunkReader.readMetaData (sampleFile, busyChunkData))
    {
        LogReader (sampleFile.getFileName () + " contains meta-data");
        const auto busyChunkVersion { getValue <SquidSalmple::DataLayout_186::kBusyChunkSignatureAndVersionSize> (SquidSalmple::DataLayout_186::kBusyChunkSignatureAndVersionOffset) };
        if ((busyChunkVersion & 0xFFFFFF00) != (kSignatureAndVersionCurrent & 0xFFFFFF00))
        {
            juce::Logger::outputDebugString ("'busy' metadata chunk has wrong signature");
        }
        else
        {
            const auto metaDataVersion { busyChunkVersion & 0x000000FF };
            if (metaDataVersion != (kSignatureAndVersionCurrent & 0x000000FF))
                juce::Logger::outputDebugString ("Version mismatch. version read in: " + juce::String (metaDataVersion) + ". expected version: " + juce::String (kSignatureAndVersionCurrent & 0x000000FF));

            if (metaDataVersion < 115) // I know I can't read in 114, so I am assuming I can read in anything after that
                juce::Logger::outputDebugString ("Unsupported version. Reverting to default meta-data");
            else if (metaDataVersion < 119)
                metaDataStatus = MetaDataStatus::fw186;
            else
                metaDataStatus = MetaDataStatus::latest;
        }
    }

    // META-DATA IS THE LATEST
    if (metaDataStatus == MetaDataStatus::latest)
    {
        squidChannelProperties.setAttack (getValue <SquidSalmple::DataLayout_190::kAttackSize> (SquidSalmple::DataLayout_190::kAttackOffset), false);
        squidChannelProperties.setBits (getValue <SquidSalmple::DataLayout_190::kQualitySize> (SquidSalmple::DataLayout_190::kQualityOffset), false);
        squidChannelProperties.setChannelFlags (getValue <SquidSalmple::DataLayout_190::kChannelFlagsSize> (SquidSalmple::DataLayout_190::kChannelFlagsOffset), false);
        jassert (! ((squidChannelProperties.getChannelFlags () & ChannelFlags::kCueRandom) && (squidChannelProperties.getChannelFlags () & ChannelFlags::kCueStepped)));
#if JUCE_DEBUG
        const auto channelFlags { squidChannelProperties.getChannelFlags () };
        juce::String channelFlagsString;
        if (channelFlags == 0)
        {
            channelFlagsString += "NONE";
        }
        else
        {
            auto addFlagString = [&channelFlagsString] (juce::String flagString)
            {
                channelFlagsString += (channelFlagsString.length () == 0 ? "" : ", ") + flagString;
            };
            if (channelFlags & ChannelFlags::kMute)
                addFlagString ("mute");
            if (channelFlags & ChannelFlags::kSolo)
                addFlagString ("solo");
            if (channelFlags & ChannelFlags::kNoGate)
                addFlagString ("noGate");
            if (channelFlags & ChannelFlags::kCueRandom)
                addFlagString ("cueRandom");
            if (channelFlags & ChannelFlags::kCueStepped)
                addFlagString ("cueStepped");
            if (channelFlags & ChannelFlags::kNeighborOutput)
                addFlagString ("neighborOutput");
        }

        LogReader ("Channel Flags: " + channelFlagsString);
#endif
        squidChannelProperties.setChannelSource (getValue <SquidSalmple::DataLayout_190::kChannelSourceSize> (SquidSalmple::DataLayout_190::kChannelSourceOffset), false);
        squidChannelProperties.setChoke (getValue <SquidSalmple::DataLayout_190::kChokeSize> (SquidSalmple::DataLayout_190::kChokeOffset), false);
        squidChannelProperties.setDecay (getValue <SquidSalmple::DataLayout_190::kDecaySize> (SquidSalmple::DataLayout_190::kDecayOffset), false);
        squidChannelProperties.setEndCue (getValue <SquidSalmple::DataLayout_190::kSampleEndSize> (SquidSalmple::DataLayout_190::kSampleEndOffset), false);
        squidChannelProperties.setEndOfData (getValue <SquidSalmple::DataLayout_190::kEndOfDataSize> (SquidSalmple::DataLayout_190::kEndOFDataOffset), false);
        squidChannelProperties.setETrig (getValue <SquidSalmple::DataLayout_190::kExternalTriggerSize> (SquidSalmple::DataLayout_190::kExternalTriggerOffset), false);
        uint16_t frequencyAndType { getValue <SquidSalmple::DataLayout_190::kCutoffFrequencySize> (SquidSalmple::DataLayout_190::kCutoffFrequencyOffset) };
        squidChannelProperties.setFilterType (frequencyAndType & 0x000F, false);
        squidChannelProperties.setFilterFrequency (frequencyAndType >> 4, false);
        squidChannelProperties.setFilterResonance (getValue <SquidSalmple::DataLayout_190::kResonanceSize> (SquidSalmple::DataLayout_190::kResonanceOffset), false);
        squidChannelProperties.setLevel (getValue <SquidSalmple::DataLayout_190::kLevelSize> (SquidSalmple::DataLayout_190::kLevelOffset), false);
        squidChannelProperties.setLoopCue (getValue <SquidSalmple::DataLayout_190::kLoopPositionSize> (SquidSalmple::DataLayout_190::kLoopPositionOffset), false);
        squidChannelProperties.setLoopMode (getValue <SquidSalmple::DataLayout_190::kLoopSize> (SquidSalmple::DataLayout_190::kLoopOffset), false);
        squidChannelProperties.setQuant (getValue <SquidSalmple::DataLayout_190::kQuantizeModeSize> (SquidSalmple::DataLayout_190::kQuantizeModeOffset), false);
        squidChannelProperties.setPitchShift (getValue<SquidSalmple::DataLayout_190::kPitchShiftSize> (SquidSalmple::DataLayout_190::kPitchShiftOffset), false);
        squidChannelProperties.setRate (getValue <SquidSalmple::DataLayout_190::kRateSize> (SquidSalmple::DataLayout_190::kRateOffset), false);
        squidChannelProperties.setRecDest (getValue <SquidSalmple::DataLayout_190::kRecDestSize> (SquidSalmple::DataLayout_190::kRecDestOffset), false);
        squidChannelProperties.setReverse (getValue <SquidSalmple::DataLayout_190::kReverseSize> (SquidSalmple::DataLayout_190::kReverseOffset), false);
        squidChannelProperties.setSpeed (getValue <SquidSalmple::DataLayout_190::kSpeedSize> (SquidSalmple::DataLayout_190::kSpeedOffset), false);
        squidChannelProperties.setStartCue (getValue <SquidSalmple::DataLayout_190::kSampleStartSize> (SquidSalmple::DataLayout_190::kSampleStartOffset), false);
        squidChannelProperties.setSteps (getValue<SquidSalmple::DataLayout_190::kStepTrigNumSize> (SquidSalmple::DataLayout_190::kStepTrigNumOffset), false);
        squidChannelProperties.setXfade (getValue <SquidSalmple::DataLayout_190::kXfadeSize> (SquidSalmple::DataLayout_190::kXfadeOffset), false);

        ////////////////////////////////////
        // cv assign
        constexpr auto rowSize { (kCvParamsCount_190 + kCvParamsExtra) * 4 };
        for (auto curCvInputIndex { 0 }; curCvInputIndex < kCvInputsCount + kCvInputsExtra; ++curCvInputIndex)
        {
            squidChannelProperties.forEachCvParameter (curCvInputIndex, [this, curCvInputIndex, rowSize] (juce::ValueTree cvParameterVT)
            {
                CvParameterProperties cvParameterProperties { cvParameterVT, CvParameterProperties::WrapperType::client, CvParameterProperties::EnableCallbacks::no };
                const auto parameterId { cvParameterProperties.getId () };

                const auto cvAssignedFlagMask { CvParameterIndex::getCvEnabledFlag (parameterId) };
                const auto cvAssignFlags { getValue <4> (SquidSalmple::DataLayout_190::kCvFlagsOffset + (4 * curCvInputIndex)) };
                const auto isEnabled { cvAssignFlags & cvAssignedFlagMask };

                const auto cvParamOffset { SquidSalmple::DataLayout_190::kCvParamsOffset + (curCvInputIndex * rowSize) + (parameterId * 4) };
                const auto offset { getValue <2> (cvParamOffset + 0) };
                const auto attenuation { static_cast<int32_t> (getValue <2> (cvParamOffset + 2)) };

                LogReader (juce::String ("cv: ") + juce::String (curCvInputIndex) + ", param: (" + CvParameterIndex::getParameterName (cvAssignedFlagMask) + ")" + juce::String (parameterId) +
                           ", raw enabled data: " + juce::String (cvAssignFlags) +
                           ", enabled: " + (isEnabled ? "true" : "false") +
                           ", atten: " + juce::String (attenuation) + ", offset: " + juce::String (offset));
                cvParameterProperties.setEnabled (isEnabled, false);
                cvParameterProperties.setAttenuation (attenuation, false);
                cvParameterProperties.setOffset (offset, false);
                return true;
            });
        }

        ////////////////////////////////////
        // cue sets
        auto logCueSet = [this, numSamples = squidChannelProperties.getEndOfData ()] ([[maybe_unused]] int8_t cueSetIndex, uint32_t startCue, uint32_t loopCue, uint32_t endCue)
        {
            LogReader ("read - cue set " + (cueSetIndex == -1 ? "current" : juce::String (cueSetIndex)) +
                       ": start = " + juce::String (startCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (startCue).paddedLeft ('0', 6) +
                       "], loop = " + juce::String (loopCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (loopCue).paddedLeft ('0', 6) +
                       "], end = " + juce::String (endCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (endCue).paddedLeft ('0', 6) + "]");
            jassert ((startCue <= loopCue && loopCue < endCue) || (numSamples == 0 && startCue == 0 && loopCue == 0 && endCue == 0));
        };

        const auto numCues { getValue <SquidSalmple::DataLayout_190::kCuesCountSize> (SquidSalmple::DataLayout_190::kCuesCountOffset) };
        const auto curCue { getValue <SquidSalmple::DataLayout_190::kCuesSelectedSize> (SquidSalmple::DataLayout_190::kCuesSelectedOffset) };
        //squidChannelProperties.setNumCueSets (numCues, false); // don't do this, because the count is updated below by squidChannelProperties.addCueSet
        LogReader ("read - cur cue meta data (cue set " + juce::String (curCue) + "):");
        logCueSet (-1, squidChannelProperties.getStartCue (), squidChannelProperties.getLoopCue (), squidChannelProperties.getEndCue ());

        LogReader ("read - Cue List: " + juce::String (numCues));
        for (uint8_t curCueSetIndex { 0 }; curCueSetIndex < numCues; ++curCueSetIndex)
        {
            const auto cueSetOffset { SquidSalmple::DataLayout_190::kCuesOffset + (curCueSetIndex * 12) };
            const auto startCue { getValue <4> (cueSetOffset + 0) };
            const auto endCue { getValue <4> (cueSetOffset + 4) };
            const auto loopCue { getValue <4> (cueSetOffset + 8) };
            logCueSet (curCueSetIndex, startCue, loopCue, endCue);
            squidChannelProperties.setCueSetPoints (curCueSetIndex, startCue, loopCue, endCue);
        }
        squidChannelProperties.setCurCueSet (curCue, false);

        auto readReserved = [this, &squidChannelProperties] (int reservedDataOffset, int reservedDataSize, std::function<void (juce::String)> setter)
        {
            juce::MemoryBlock tempMemory;
            tempMemory.replaceAll (static_cast<uint8_t*> (busyChunkData.getData ()) + reservedDataOffset, reservedDataSize);
            auto textVersion { tempMemory.toBase64Encoding () };
            setter (textVersion);
        };
        // read and store the 'reserved' sections
        readReserved (SquidSalmple::DataLayout_190::k_Reserved1Offset, SquidSalmple::DataLayout_190::k_Reserved1Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved1Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved2Offset, SquidSalmple::DataLayout_190::k_Reserved2Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved2Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved3Offset, SquidSalmple::DataLayout_190::k_Reserved3Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved3Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved4Offset, SquidSalmple::DataLayout_190::k_Reserved4Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved4Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved5Offset, SquidSalmple::DataLayout_190::k_Reserved5Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved5Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved6Offset, SquidSalmple::DataLayout_190::k_Reserved6Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved6Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved7Offset, SquidSalmple::DataLayout_190::k_Reserved7Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved7Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved8Offset, SquidSalmple::DataLayout_190::k_Reserved8Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved8Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved9Offset, SquidSalmple::DataLayout_190::k_Reserved9Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved9Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved10Offset, SquidSalmple::DataLayout_190::k_Reserved10Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved10Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved11Offset, SquidSalmple::DataLayout_190::k_Reserved11Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved11Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved12Offset, SquidSalmple::DataLayout_190::k_Reserved12Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved12Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved13Offset, SquidSalmple::DataLayout_190::k_Reserved13Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved13Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved14Offset, SquidSalmple::DataLayout_190::k_Reserved14Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved14Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_190::k_Reserved15Offset, SquidSalmple::DataLayout_190::k_Reserved15Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved15Data (reservedData); });
    }
    // META-DATA IS BEFORE PITCH SHIFT WAS ADDED
    else if (metaDataStatus == MetaDataStatus::fw186)
    {
        // should we alert the user with something like
        // You are opening a bank created for a earlier firmware version. If you save this bank it will be written with the new firmware version format, and will no be usable with
        // the earlier firmware
        squidChannelProperties.setAttack (getValue <SquidSalmple::DataLayout_186::kAttackSize> (SquidSalmple::DataLayout_186::kAttackOffset), false);
        squidChannelProperties.setBits (getValue <SquidSalmple::DataLayout_186::kQualitySize> (SquidSalmple::DataLayout_186::kQualityOffset), false);
        squidChannelProperties.setChannelFlags (getValue <SquidSalmple::DataLayout_186::kChannelFlagsSize> (SquidSalmple::DataLayout_186::kChannelFlagsOffset), false);
        jassert (!((squidChannelProperties.getChannelFlags ()& ChannelFlags::kCueRandom) && (squidChannelProperties.getChannelFlags ()& ChannelFlags::kCueStepped)));
#if JUCE_DEBUG
        const auto channelFlags { squidChannelProperties.getChannelFlags () };
        juce::String channelFlagsString;
        if (channelFlags == 0)
        {
            channelFlagsString += "NONE";
        }
        else
        {
            auto addFlagString = [&channelFlagsString] (juce::String flagString)
                {
                    channelFlagsString += (channelFlagsString.length () == 0 ? "" : ", ") + flagString;
                };
            if (channelFlags & ChannelFlags::kMute)
                addFlagString ("mute");
            if (channelFlags & ChannelFlags::kSolo)
                addFlagString ("solo");
            if (channelFlags & ChannelFlags::kNoGate)
                addFlagString ("noGate");
            if (channelFlags & ChannelFlags::kCueRandom)
                addFlagString ("cueRandom");
            if (channelFlags & ChannelFlags::kCueStepped)
                addFlagString ("cueStepped");
            if (channelFlags & ChannelFlags::kNeighborOutput)
                addFlagString ("neighborOutput");
        }

        LogReader ("Channel Flags: " + channelFlagsString);
#endif
        squidChannelProperties.setChannelSource (getValue <SquidSalmple::DataLayout_186::kChannelSourceSize> (SquidSalmple::DataLayout_186::kChannelSourceOffset), false);
        squidChannelProperties.setChoke (getValue <SquidSalmple::DataLayout_186::kChokeSize> (SquidSalmple::DataLayout_186::kChokeOffset), false);
        squidChannelProperties.setDecay (getValue <SquidSalmple::DataLayout_186::kDecaySize> (SquidSalmple::DataLayout_186::kDecayOffset), false);
        squidChannelProperties.setEndCue (getValue <SquidSalmple::DataLayout_186::kSampleEndSize> (SquidSalmple::DataLayout_186::kSampleEndOffset), false);
        squidChannelProperties.setEndOfData (getValue <SquidSalmple::DataLayout_186::kEndOfDataSize> (SquidSalmple::DataLayout_186::kEndOFDataOffset), false);
        squidChannelProperties.setETrig (getValue <SquidSalmple::DataLayout_186::kExternalTriggerSize> (SquidSalmple::DataLayout_186::kExternalTriggerOffset), false);
        uint16_t frequencyAndType { getValue <SquidSalmple::DataLayout_186::kCutoffFrequencySize> (SquidSalmple::DataLayout_186::kCutoffFrequencyOffset) };
        squidChannelProperties.setFilterType (frequencyAndType & 0x000F, false);
        squidChannelProperties.setFilterFrequency (frequencyAndType >> 4, false);
        squidChannelProperties.setFilterResonance (getValue <SquidSalmple::DataLayout_186::kResonanceSize> (SquidSalmple::DataLayout_186::kResonanceOffset), false);
        squidChannelProperties.setLevel (getValue <SquidSalmple::DataLayout_186::kLevelSize> (SquidSalmple::DataLayout_186::kLevelOffset), false);
        squidChannelProperties.setLoopCue (getValue <SquidSalmple::DataLayout_186::kLoopPositionSize> (SquidSalmple::DataLayout_186::kLoopPositionOffset), false);
        squidChannelProperties.setLoopMode (getValue <SquidSalmple::DataLayout_186::kLoopSize> (SquidSalmple::DataLayout_186::kLoopOffset), false);
        squidChannelProperties.setQuant (getValue <SquidSalmple::DataLayout_186::kQuantizeModeSize> (SquidSalmple::DataLayout_186::kQuantizeModeOffset), false);
        squidChannelProperties.setRate (getValue <SquidSalmple::DataLayout_186::kRateSize> (SquidSalmple::DataLayout_186::kRateOffset), false);
        squidChannelProperties.setRecDest (getValue <SquidSalmple::DataLayout_186::kRecDestSize> (SquidSalmple::DataLayout_186::kRecDestOffset), false);
        squidChannelProperties.setReverse (getValue <SquidSalmple::DataLayout_186::kReverseSize> (SquidSalmple::DataLayout_186::kReverseOffset), false);
        squidChannelProperties.setSpeed (getValue <SquidSalmple::DataLayout_186::kSpeedSize> (SquidSalmple::DataLayout_186::kSpeedOffset), false);
        squidChannelProperties.setStartCue (getValue <SquidSalmple::DataLayout_186::kSampleStartSize> (SquidSalmple::DataLayout_186::kSampleStartOffset), false);
        squidChannelProperties.setSteps (getValue<SquidSalmple::DataLayout_186::kStepTrigNumSize> (SquidSalmple::DataLayout_186::kStepTrigNumOffset), false);
        squidChannelProperties.setXfade (getValue <SquidSalmple::DataLayout_186::kXfadeSize> (SquidSalmple::DataLayout_186::kXfadeOffset), false);

        ////////////////////////////////////
        // cv assign
        const auto rowSize { (kCvParamsCount_186 + kCvParamsExtra) * 4 };
        for (auto curCvInputIndex { 0 }; curCvInputIndex < kCvInputsCount + kCvInputsExtra; ++curCvInputIndex)
        {
            for (auto curParameterIndex { 0 }; curParameterIndex < 15; ++curParameterIndex)
            {
                CvParameterProperties cvParameterProperties { squidChannelProperties.getCvParameterVT (curCvInputIndex, curParameterIndex), CvParameterProperties::WrapperType::client, CvParameterProperties::EnableCallbacks::no };

                const auto cvParamOffset { SquidSalmple::DataLayout_186::kCvParamsOffset + (curCvInputIndex * rowSize) + (curParameterIndex * 4) };
                const auto cvAssignedFlag { CvParameterIndex::getCvEnabledFlag (curParameterIndex) };
                const auto cvAssignFlags { getValue <2> (SquidSalmple::DataLayout_186::kCvFlagsOffset + (2 * curCvInputIndex)) };
                const auto offset { getValue <2> (cvParamOffset + 0) };
                const auto attenuation { static_cast<int16_t> (getValue <2> (cvParamOffset + 2)) };
                cvParameterProperties.setEnabled (cvAssignFlags& cvAssignedFlag, false);
                cvParameterProperties.setAttenuation (attenuation, false);
                cvParameterProperties.setOffset (offset, false);
            }
        }

        ////////////////////////////////////
        // cue sets
        auto logCueSet = [this, numSamples = squidChannelProperties.getEndOfData ()] ([[maybe_unused]] int8_t cueSetIndex, uint32_t startCue, uint32_t loopCue, uint32_t endCue)
            {
                LogReader ("read - cue set " + (cueSetIndex == -1 ? "current" : juce::String (cueSetIndex)) +
                           ": start = " + juce::String (startCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (startCue).paddedLeft ('0', 6) +
                           "], loop = " + juce::String (loopCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (loopCue).paddedLeft ('0', 6) +
                           "], end = " + juce::String (endCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (endCue).paddedLeft ('0', 6) + "]");
                jassert ((startCue <= loopCue && loopCue < endCue) || (numSamples == 0 && startCue == 0 && loopCue == 0 && endCue == 0));
            };

        const auto numCues { getValue <SquidSalmple::DataLayout_186::kCuesCountSize> (SquidSalmple::DataLayout_186::kCuesCountOffset) };
        const auto curCue { getValue <SquidSalmple::DataLayout_186::kCuesSelectedSize> (SquidSalmple::DataLayout_186::kCuesSelectedOffset) };
        //squidChannelProperties.setNumCueSets (numCues, false); // don't do this, because the count is updated below by squidChannelProperties.addCueSet
        LogReader ("read - cur cue meta data (cue set " + juce::String (curCue) + "):");
        logCueSet (-1, squidChannelProperties.getStartCue (), squidChannelProperties.getLoopCue (), squidChannelProperties.getEndCue ());

        LogReader ("read - Cue List: " + juce::String (numCues));
        for (uint8_t curCueSetIndex { 0 }; curCueSetIndex < numCues; ++curCueSetIndex)
        {
            const auto cueSetOffset { SquidSalmple::DataLayout_186::kCuesOffset + (curCueSetIndex * 12) };
            const auto startCue { getValue <4> (cueSetOffset + 0) };
            const auto endCue { getValue <4> (cueSetOffset + 4) };
            const auto loopCue { getValue <4> (cueSetOffset + 8) };
            logCueSet (curCueSetIndex, startCue, loopCue, endCue);
            squidChannelProperties.setCueSetPoints (curCueSetIndex, startCue, loopCue, endCue);
        }
        squidChannelProperties.setCurCueSet (curCue, false);

        auto readReserved = [this, &squidChannelProperties] (int reservedDataOffset, int reservedDataSize, std::function<void (juce::String)> setter)
            {
                juce::MemoryBlock tempMemory;
                tempMemory.replaceAll (static_cast<uint8_t*> (busyChunkData.getData ()) + reservedDataOffset, reservedDataSize);
                auto textVersion { tempMemory.toBase64Encoding () };
                setter (textVersion);
            };
        // read and store the 'reserved' sections
        readReserved (SquidSalmple::DataLayout_186::k_Reserved1Offset, SquidSalmple::DataLayout_186::k_Reserved1Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved1Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved2Offset, SquidSalmple::DataLayout_186::k_Reserved2Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved2Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved3Offset, SquidSalmple::DataLayout_186::k_Reserved3Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved3Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved4Offset, SquidSalmple::DataLayout_186::k_Reserved4Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved4Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved5Offset, SquidSalmple::DataLayout_186::k_Reserved5Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved5Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved6Offset, SquidSalmple::DataLayout_186::k_Reserved6Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved6Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved7Offset, SquidSalmple::DataLayout_186::k_Reserved7Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved7Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved8Offset, SquidSalmple::DataLayout_186::k_Reserved8Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved8Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved9Offset, SquidSalmple::DataLayout_186::k_Reserved9Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved9Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved10Offset, SquidSalmple::DataLayout_186::k_Reserved10Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved10Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved11Offset, SquidSalmple::DataLayout_186::k_Reserved11Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved11Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved12Offset, SquidSalmple::DataLayout_186::k_Reserved12Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved12Data (reservedData); });
        readReserved (SquidSalmple::DataLayout_186::k_Reserved13Offset, SquidSalmple::DataLayout_186::k_Reserved13Size, [&squidChannelProperties] (juce::String reservedData) { squidChannelProperties.setReserved13Data (reservedData); });

    }
    // NO METADATA
    else // metaDataStatus == MetaDataStatus::invalid
    {
        LogReader (sampleFile.getFileName () + " does not contain meta-data");
        auto numSamples = squidChannelProperties.getSampleDataNumSamples ();
        uint32_t endOffset = numSamples * 2;
        // initialize parameters that have defaults related to specific channel or sample
        squidChannelProperties.setChannelIndex (channelIndex, false);
        squidChannelProperties.setChannelSource (channelIndex, false);
        squidChannelProperties.setChoke (channelIndex, false);
        squidChannelProperties.setEndOfData (endOffset, false);
        squidChannelProperties.setRecDest (channelIndex, false);
        if (auto markerList { busyChunkReader.getMarkerList (sampleFile) }; markerList.size () != 0)
        {
            auto addCueSet = [&squidChannelProperties] (int cueSetIndex, int startCue, int endCue)
            {
                LogReader ("import - cue set " + juce::String (cueSetIndex) +
                    ": start = " + juce::String (startCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (startCue).paddedLeft ('0', 6) +
                    "], loop = " + juce::String (startCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (startCue).paddedLeft ('0', 6) +
                    "], end = " + juce::String (endCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (endCue).paddedLeft ('0', 6) + "]");
                squidChannelProperties.setCueSetPoints (cueSetIndex, startCue, startCue, endCue);
            };
            LogReader ("importing markers");
            // import markers
            if (markerList [0] > 0)
            {
                // create first cue set from start of sample to first marker
                addCueSet (0, 0, SquidChannelProperties::sampleOffsetToByteOffset (markerList [0]));
            }
            for (auto markerListIndex { 0 }; markerListIndex < markerList.size () - 1; ++markerListIndex)
            {
                const auto startCue { SquidChannelProperties::sampleOffsetToByteOffset (markerList [markerListIndex]) };
                const auto endCue { SquidChannelProperties::sampleOffsetToByteOffset (markerList [markerListIndex + 1]) };
                const auto cueSetIndex { markerListIndex + 1 };
                addCueSet (cueSetIndex, startCue, endCue);
                jassert (startCue <= endCue);
            }
            if (const auto lastMarker { markerList [markerList.size () - 1] }; lastMarker < numSamples)
            {
                // create last cue set from last marker to end of sample
                addCueSet (static_cast<int> (markerList.size ()), SquidChannelProperties::sampleOffsetToByteOffset (lastMarker), endOffset);
            }
            // set initial cue points to first cue set
            squidChannelProperties.setStartCue (squidChannelProperties.getStartCueSet (0), false);
            squidChannelProperties.setLoopCue (squidChannelProperties.getLoopCueSet (0), false);
            squidChannelProperties.setEndCue (squidChannelProperties.getEndCueSet (0), false);
        }
        else
        {
            // set end cue to end of sample
            squidChannelProperties.setEndCue (endOffset, false);
            // set first cue set
            squidChannelProperties.setCueSetPoints (0, 0, 0, endOffset);
        }
    }

    squidChannelProperties.setSampleFileName (sampleFile.getFullPathName (), false);
}