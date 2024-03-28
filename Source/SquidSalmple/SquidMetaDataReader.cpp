#include "SquidMetaDataReader.h"
#include "SquidSalmpleDefs.h"

juce::ValueTree SquidMetaDataReader::read (juce::File sampleFile)
{
    BusyChunkReader busyChunkReader;
    busyChunkReader.read (sampleFile, busyChunkData);
    //const auto rawChunkData { static_cast<uint8_t*>(busyChunkData.getData ()) };
    const auto busyChunkVersion { getValue <SquidSalmple::DataLayout::kBusyChunkSignatureAndVersionSize> (SquidSalmple::DataLayout::kBusyChunkSignatureAndVersionOffset) };
    //jassert (busyChunkVersion == kSignatureAndVersionCurrent);
    if (busyChunkVersion != kSignatureAndVersionCurrent)
        juce::Logger::outputDebugString ("Earlier version of fw: " + juce::String(busyChunkVersion & 0xFF) + ". expected " + juce::String (kSignatureAndVersionCurrent & 0xFF));

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

    return squidMetaDataProperties.getValueTree ();
}
