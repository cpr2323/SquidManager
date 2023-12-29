#include "SquidMetaDataWriter.h"

#include "SquidMetaDataReader.h"

// TODO - UNTIL IT IS UNDERSTOOD
// TODO - THIS IS ALL PLACEHOLDER WHERE THIS CLASS RELATES TO DATA FIELDS, THIER CONTENTS, LOCATIONS AND BYTE ORDERING

// TODO - THESE ARE JUST PLACEHOLDERS UNTIL I KNOW THE ACTUAL VALUES
// TODO - THERE MAY BE MORE FIELDS, AND ARRAYS OF FIELDS TOO
const auto kAttackDataOffset { 0 };
const auto kBitsDataOffset { 2 };
const auto kCueEndDataOffset { 4 };
const auto kCueLoopDataOffset { 6 };
const auto kCueStartDataOffset { 8 };
const auto kCv1DataOffset { 10 };
const auto kCv2DataOffset { 12 };
const auto kCv3DataOffset { 14 };
const auto kDecayDataOffset { 16 };
const auto kEuclidianTriggerDataOffset { 18 };
const auto kFilterDataOffset { 20 };
const auto kFilterFrequencyDataOffset { 22 };
const auto kFilterResonanceDataOffset { 24 };
const auto kLevelDataOffset { 26 };
const auto kLoopDataOffset { 28 };
const auto kQuantDataOffset { 30 };
const auto kRateDataOffset { 32 };
const auto kReverseDataOffset { 34 };
const auto kSpeedDataOffset { 36 };
const auto kStepsDataOffset { 38 };
const auto kXfadeDataOffset { 40 };

bool SquidMetaDataWriter::write (juce::ValueTree squidMetaDataPropertiesVT, juce::File inputSampleFile, juce::File outputSampleFile)
{
    jassert (inputSampleFile != outputSampleFile);

    busyChunkData.setSize (21 * sizeof (uint16_t));

    SquidMetaDataProperties squidMetaDataProperties { squidMetaDataPropertiesVT, SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::no };
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getAttack ()), kAttackDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getBits ()), kBitsDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getCueEnd ()), kCueEndDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getCueLoop ()), kCueLoopDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getCueStart ()), kCueStartDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getCv1 ()), kCv1DataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getCv2 ()), kCv2DataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getCv3 ()), kCv3DataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getDecay ()), kDecayDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getEuclidianTrigger ()), kEuclidianTriggerDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getFilter ()), kFilterDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getFilterFrequency ()), kFilterFrequencyDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getFilterResonance ()), kFilterResonanceDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getLevel ()), kLevelDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getLoop ()), kLoopDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getQuant ()), kQuantDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getRate ()), kRateDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getReverse ()), kReverseDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getSpeed ()), kSpeedDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getSteps ()), kStepsDataOffset);
    setUInt16 (static_cast<uint16_t> (squidMetaDataProperties.getXfade ()), kXfadeDataOffset);

    BusyChunkWriter busyChunkWriter;
    jassert (busyChunkWriter.write (inputSampleFile, outputSampleFile, busyChunkData) == true);

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
