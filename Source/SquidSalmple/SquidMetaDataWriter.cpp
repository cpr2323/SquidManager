#include "SquidMetaDataWriter.h"
#include "SquidSalmpleDefs.h"

bool SquidMetaDataWriter::write (juce::ValueTree squidMetaDataPropertiesVT, juce::File inputSampleFile, juce::File outputSampleFile)
{
    jassert (inputSampleFile != outputSampleFile);

    busyChunkData.setSize (SquidSalmple::DataLayout::kEndOfData);

    SquidMetaDataProperties squidMetaDataProperties { squidMetaDataPropertiesVT, SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::no };
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getAttack ()), SquidSalmple::DataLayout::kAttackOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getBits ()), SquidSalmple::DataLayout::kQualityOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getDecay ()), SquidSalmple::DataLayout::kDecayOffset);
    setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getEndCue ()), SquidSalmple::DataLayout::kSampleEndOffset);
    // TODO: verify ordering bits freq/type or type/freq
    // 12bits of filter frequency and 4 bits of filter type
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
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getXfade ()), SquidSalmple::DataLayout::kXfadeOffset);

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
    busyChunkData [offset]     = static_cast<uint8_t>(value >> 8);
    busyChunkData [offset + 1] = static_cast<uint8_t>(value);
}

void SquidMetaDataWriter::setUInt32 (uint32_t value, int offset)
{
    busyChunkData [offset + 0] = static_cast<uint8_t>(value >> 24);
    busyChunkData [offset + 1] = static_cast<uint8_t>(value >> 16);
    busyChunkData [offset + 2] = static_cast<uint8_t>(value >> 8);
    busyChunkData [offset + 3] = static_cast<uint8_t>(value);
}

void SquidMetaDataWriter::setUInt64 (uint64_t value, int offset)
{
    busyChunkData [offset + 0] = static_cast<uint8_t>(value >> 56);
    busyChunkData [offset + 1] = static_cast<uint8_t>(value >> 48);
    busyChunkData [offset + 2] = static_cast<uint8_t>(value >> 40);
    busyChunkData [offset + 3] = static_cast<uint8_t>(value >> 32);
    busyChunkData [offset + 4] = static_cast<uint8_t>(value >> 24);
    busyChunkData [offset + 5] = static_cast<uint8_t>(value >> 16);
    busyChunkData [offset + 6] = static_cast<uint8_t>(value >> 8);
    busyChunkData [offset + 7] = static_cast<uint8_t>(value);
}