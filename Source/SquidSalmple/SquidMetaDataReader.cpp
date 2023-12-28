#include "SquidMetaDataReader.h"

// TODO - UNTIL IT IS UNDERSTOOD
// TODO - THIS IS ALL PLACEHOLDER WHERE THIS CLASS RELATES TO DATA FIELDS, THIER CONTENTS, LOCATIONS AND BYTE ORDERING

// TODO - THESE ARE JUST PLACEHOLDERS UNTIL I KNOW THE ACTUAL VALUES
// TODO - THERE MAY BE MORE FIELDS, AND ARRAYS OF FIELDS TOO
const auto kAttackDataOffset           { 0 };
const auto kBitsDataOffset             { 2 };
const auto kCueEndDataOffset           { 4 };
const auto kCueLoopDataOffset          { 6 };
const auto kCueStartDataOffset         { 8 };
const auto kCv1DataOffset              { 10 };
const auto kCv2DataOffset              { 12 };
const auto kCv3DataOffset              { 14 };
const auto kDecayDataOffset            { 16 };
const auto kEuclidianTriggerDataOffset { 18 };
const auto kFilterDataOffset           { 20 };
const auto kFilterFrequencyDataOffset  { 22 };
const auto kFilterResonanceDataOffset  { 24 };
const auto kLevelDataOffset            { 26 };
const auto kLoopDataOffset             { 28 };
const auto kQuantDataOffset            { 30 };
const auto kRateDataOffset             { 32 };
const auto kReverseDataOffset          { 34 };
const auto kSpeedDataOffset            { 36 };
const auto kStepsDataOffset            { 38 };
const auto kXfadeDataOffset            { 40 };

juce::ValueTree SquidMetaDataReader::read (juce::File sampleFile)
{
    BusyChunkReader busyChunkReader;
    busyChunkReader.read (sampleFile, busyChunkData);

    SquidMetaDataProperties squidMetaDataProperties { {}, SquidMetaDataProperties::WrapperType::owner, SquidMetaDataProperties::EnableCallbacks::no };
    squidMetaDataProperties.setAttack (getUInt16 (kAttackDataOffset), false);
    squidMetaDataProperties.setBits (getUInt16 (kBitsDataOffset), false);
    squidMetaDataProperties.setCueEnd (getUInt16 (kCueEndDataOffset), false);
    squidMetaDataProperties.setCueLoop (getUInt16 (kCueLoopDataOffset), false);
    squidMetaDataProperties.setCueStart (getUInt16 (kCueStartDataOffset), false);
    squidMetaDataProperties.setCv1 (getUInt16 (kCv1DataOffset), false);
    squidMetaDataProperties.setCv2 (getUInt16 (kCv2DataOffset), false);
    squidMetaDataProperties.setCv3 (getUInt16 (kCv3DataOffset), false);
    squidMetaDataProperties.setDecay (getUInt16 (kDecayDataOffset), false);
    squidMetaDataProperties.setEuclidianTrigger (getUInt16 (kEuclidianTriggerDataOffset), false);
    squidMetaDataProperties.setFilter (getUInt16 (kFilterDataOffset), false);
    squidMetaDataProperties.setFilterFrequency (getUInt16 (kFilterFrequencyDataOffset), false);
    squidMetaDataProperties.setFilterResonance (getUInt16 (kFilterResonanceDataOffset), false);
    squidMetaDataProperties.setLevel (getUInt16 (kLevelDataOffset), false);
    squidMetaDataProperties.setLoop (getUInt16 (kLoopDataOffset), false);
    squidMetaDataProperties.setQuant (getUInt16 (kQuantDataOffset), false);
    squidMetaDataProperties.setRate (getUInt16 (kRateDataOffset), false);
    squidMetaDataProperties.setReverse (getUInt16 (kReverseDataOffset), false);
    squidMetaDataProperties.setSpeed (getUInt16 (kSpeedDataOffset), false);
    squidMetaDataProperties.setSteps (getUInt16 (kStepsDataOffset), false);
    squidMetaDataProperties.setXfade (getUInt16 (kXfadeDataOffset), false);

    return squidMetaDataProperties.getValueTree ();
}

uint8_t SquidMetaDataReader::getUInt8 (int offset)
{
    return static_cast<uint8_t>(busyChunkData [offset]);
}

uint16_t SquidMetaDataReader::getUInt16 (int offset)
{
    return (static_cast<uint16_t>(busyChunkData [offset]) << 8) + static_cast<uint16_t>(busyChunkData [offset + 1]);
}

uint32_t SquidMetaDataReader::getUInt32 (int offset)
{
    return (static_cast<uint32_t>(busyChunkData [offset + 0]) << 24) + (static_cast<uint32_t>(busyChunkData [offset + 1]) << 16) +
           (static_cast<uint32_t>(busyChunkData [offset + 2]) <<  8) + (static_cast<uint32_t>(busyChunkData [offset + 3]) >>  0);
}

uint64_t SquidMetaDataReader::getUInt64 (int offset)
{
    return (static_cast<uint64_t>(busyChunkData [offset + 0]) << 56) + (static_cast<uint64_t>(busyChunkData [offset + 1]) << 48) +
           (static_cast<uint64_t>(busyChunkData [offset + 2]) << 40) + (static_cast<uint64_t>(busyChunkData [offset + 3]) << 32) +
           (static_cast<uint64_t>(busyChunkData [offset + 4]) << 24) + (static_cast<uint64_t>(busyChunkData [offset + 5]) << 16) +
           (static_cast<uint64_t>(busyChunkData [offset + 6]) <<  8) + (static_cast<uint64_t>(busyChunkData [offset + 7]) <<  0);
}
