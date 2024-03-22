#include "SquidMetaDataWriter.h"
#include "SquidSalmpleDefs.h"

bool SquidMetaDataWriter::write (juce::ValueTree squidMetaDataPropertiesVT, juce::File inputSampleFile, juce::File outputSampleFile)
{
    jassert (inputSampleFile != outputSampleFile);

    busyChunkData.setSize (SquidSalmple::DataLayout::kEndOfData);

    SquidMetaDataProperties squidMetaDataProperties { squidMetaDataPropertiesVT, SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::no };
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getAttack ()), SquidSalmple::DataLayout::kAttackOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getBits ()), SquidSalmple::DataLayout::kQualityOffset);
    setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getCueEnd ()), SquidSalmple::DataLayout::kSampleEndOffset);
    setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getCueLoop ()), SquidSalmple::DataLayout::kLoopPositionOffset);
    setUInt32 (static_cast<uint32_t> (squidMetaDataProperties.getCueStart ()), SquidSalmple::DataLayout::kSampleStartOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getDecay ()), SquidSalmple::DataLayout::kDecayOffset);
    // TODO: verify ordering bits freq/type or type/freq
    // 12bits of filter frequency and 4 bits of filter type
    uint16_t frequencyAndType { static_cast<uint16_t> ((squidMetaDataProperties.getFilterFrequency () << 4) + squidMetaDataProperties.getFilter ()) };
    setUInt16 (frequencyAndType, SquidSalmple::DataLayout::kCutoffFrequencyOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getFilterResonance ()), SquidSalmple::DataLayout::kResonanceOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getLevel ()), SquidSalmple::DataLayout::kLevelOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getLoop ()), SquidSalmple::DataLayout::kLoopOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getQuant ()), SquidSalmple::DataLayout::kQuantizeModeOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getRate ()), SquidSalmple::DataLayout::kRateOffset);
    setUInt8 (static_cast<uint8_t> (squidMetaDataProperties.getReverse ()), SquidSalmple::DataLayout::kReverseOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getSpeed ()), SquidSalmple::DataLayout::kSpeedOffset);
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
