#include "SquidMetaDataReader.h"
#include "SquidSalmpleDefs.h"
#include "../Utility/DebugLog.h"

#define LOG_READER 1
#if LOG_READER
#define LogReader(text) DebugLog ("SquidMetaDataReader", text);
#else
#define LogReader(text) ;
#endif

juce::ValueTree SquidMetaDataReader::read (juce::File sampleFile)
{
    LogReader ("read - reading: " + juce::String (sampleFile.getFullPathName ()));
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

    SquidMetaDataProperties squidMetaDataProperties { {}, SquidMetaDataProperties::WrapperType::owner, SquidMetaDataProperties::EnableCallbacks::no };

    squidMetaDataProperties.setAttack (getValue <SquidSalmple::DataLayout::kAttackSize> (SquidSalmple::DataLayout::kAttackOffset), false);
    squidMetaDataProperties.setBits (getValue <SquidSalmple::DataLayout::kQualitySize> (SquidSalmple::DataLayout::kQualityOffset), false);
    squidMetaDataProperties.setDecay (getValue <SquidSalmple::DataLayout::kDecaySize> (SquidSalmple::DataLayout::kDecayOffset), false);
    squidMetaDataProperties.setEndCue (getValue <SquidSalmple::DataLayout::kSampleEndSize> (SquidSalmple::DataLayout::kSampleEndOffset), false);
    // TODO: verify ordering bits freq/type or type/freq
    // 12bits of filter frequency and 4 bits of filter type
    uint16_t frequencyAndType { getValue <SquidSalmple::DataLayout::kCutoffFrequencySize> (SquidSalmple::DataLayout::kCutoffFrequencyOffset) };
    squidMetaDataProperties.setFilterType (frequencyAndType & 0x000F, false);
    squidMetaDataProperties.setFilterFrequency (frequencyAndType >> 4, false);
    squidMetaDataProperties.setFilterResonance (getValue <SquidSalmple::DataLayout::kResonanceSize> (SquidSalmple::DataLayout::kResonanceOffset), false);
    squidMetaDataProperties.setLevel (getValue <SquidSalmple::DataLayout::kLevelSize> (SquidSalmple::DataLayout::kLevelOffset), false);
    squidMetaDataProperties.setLoopCue (getValue <SquidSalmple::DataLayout::kLoopPositionSize> (SquidSalmple::DataLayout::kLoopPositionOffset), false);
    squidMetaDataProperties.setLoopMode (getValue <SquidSalmple::DataLayout::kLoopSize> (SquidSalmple::DataLayout::kLoopOffset), false);
    squidMetaDataProperties.setQuant (getValue <SquidSalmple::DataLayout::kQuantizeModeSize> (SquidSalmple::DataLayout::kQuantizeModeOffset), false);
    squidMetaDataProperties.setRate (getValue <SquidSalmple::DataLayout::kRateSize> (SquidSalmple::DataLayout::kRateOffset), false);
    squidMetaDataProperties.setReverse (getValue <SquidSalmple::DataLayout::kReverseSize> (SquidSalmple::DataLayout::kReverseOffset), false);
    squidMetaDataProperties.setSpeed (getValue <SquidSalmple::DataLayout::kSpeedSize> (SquidSalmple::DataLayout::kSpeedOffset), false);
    squidMetaDataProperties.setStartCue (getValue <SquidSalmple::DataLayout::kSampleStartSize> (SquidSalmple::DataLayout::kSampleStartOffset), false);
    squidMetaDataProperties.setXfade (getValue <SquidSalmple::DataLayout::kXfadeSize> (SquidSalmple::DataLayout::kXfadeOffset), false);

    const auto playPosition { getValue <SquidSalmple::DataLayout::k_Reserved2Size> (SquidSalmple::DataLayout::k_Reserved2Offset) };
    LogReader ("read - play position meta data = " + juce::String (playPosition).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (playPosition).paddedLeft ('0', 6) + "]");
    const auto endOfSample { getValue <SquidSalmple::DataLayout::kEndOfSampleSize> (SquidSalmple::DataLayout::kEndOfSampleOffset) };
    LogReader ("read - end of sample meta data = " + juce::String (endOfSample).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (endOfSample).paddedLeft ('0', 6) + "]");
    auto logCueSet = [this] (uint8_t cueSetIndex, uint32_t startCue, uint32_t loopCue, uint32_t endCue)
    {
        LogReader ("read - cue set " + juce::String (cueSetIndex) + ": start = " + juce::String (startCue).paddedLeft('0', 6) + " [0x" + juce::String::toHexString (startCue).paddedLeft ('0', 6) + "], loop = " +
                   juce::String (loopCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (loopCue).paddedLeft ('0', 6) + "], end = " + juce::String (endCue).paddedLeft ('0', 6) + " [0x" + juce::String::toHexString (endCue).paddedLeft ('0', 6) + "]");
        jassert (startCue <= loopCue && loopCue < endCue);
    };

    const auto numCues { getValue <SquidSalmple::DataLayout::kCuesCountSize> (SquidSalmple::DataLayout::kCuesCountOffset) };
    const auto curCue { getValue <SquidSalmple::DataLayout::kCuesSelectedSize> (SquidSalmple::DataLayout::kCuesSelectedOffset) };
    LogReader ("read - cue meta data (cue set " + juce::String(curCue) + "):");
    logCueSet (0, squidMetaDataProperties.getStartCue (), squidMetaDataProperties.getLoopCue (), squidMetaDataProperties.getEndCue ());

    LogReader ("read - Cue List: " + juce::String (numCues));
    for (auto curCueSet { 0 }; curCueSet < numCues; ++curCueSet)
    {
        const auto cueSetOffset { SquidSalmple::DataLayout::kCuesOffset + (curCueSet * 12) };
        const auto startCue { getValue <4> (cueSetOffset + 0) };
        const auto endCue { getValue <4> (cueSetOffset + 4) };
        const auto loopCue { getValue <4> (cueSetOffset + 8) };
        logCueSet (curCueSet, startCue, loopCue, endCue);
        squidMetaDataProperties.addCueSet (startCue, loopCue, endCue);
    }

    return squidMetaDataProperties.getValueTree ();
}
