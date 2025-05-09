#pragma once

#include <JuceHeader.h>

const auto kCueStart { 0 };
const auto kCueEnd   { 1 };
const auto kCueLoop  { 2 };
const auto kCueNumSets  { 64 };

const auto kCvInputsCount { 3 };
const auto kCvInputsExtra { 5 };

const auto kCvParamsCount_186 { 16 };
const auto kCvParamsCount_190 { 18 };
const auto kCvParamsExtra { 4 };

const uint32_t kSignatureAndVersionCurrent { 0x12345677 }; // 119

enum class FilterType
{
    off,
    lp,
    bp,
    notch,
    hp
};

enum class LoopType
{
    none,
    normal,
    zigZag,
    gate,
    zigZagGate
};

enum class Quantization
{
    none,
    chromatic,
    octave,
    major,
    minor,
    harmonicMinor,
    pentatonicMajor,
    pentatonicMinor,
    lydian,
    phrygian,
    japanese,
    rootAndFifth,
    oneChord,
    fourChord,
    fiveChord,
};

enum class ExternalTrigger {
    off,
    v1,
    v2,
    v3,
    v4,
    v5,
    v6,
    v7,
    v8,
    on
};

// THIS IS JUST FOR TEST PURPOSES
namespace CvAssignedFlag
{
    constexpr uint32_t none       { 0x00000000 };
    constexpr uint32_t unused     { 0x00000001 }; // unused
    constexpr uint32_t bits       { 0x00000002 };
    constexpr uint32_t rate       { 0x00000004 };
    constexpr uint32_t level      { 0x00000008 };
    constexpr uint32_t decay      { 0x00000010 };
    constexpr uint32_t speed      { 0x00000020 };
    constexpr uint32_t loopMode   { 0x00000040 };
    constexpr uint32_t reverse    { 0x00000080 };
    constexpr uint32_t startCue   { 0x00000100 };
    constexpr uint32_t endCue     { 0x00000200 };
    constexpr uint32_t loopCue    { 0x00000400 };
    constexpr uint32_t attack     { 0x00000800 };
    constexpr uint32_t cueSet     { 0x00001000 };
    constexpr uint32_t eTrig      { 0x00002000 };
    constexpr uint32_t filtFreq   { 0x00004000 };
    constexpr uint32_t filtRes    { 0x00008000 };
    constexpr uint32_t unused2    { 0x00010000 };
    constexpr uint32_t pitchShift { 0x00020000 };
}

namespace CvParameterIndex
{
    constexpr int Bits       { 0 };
    constexpr int Rate       { 1 };
    constexpr int Level      { 2 };
    constexpr int Decay      { 3 };
    constexpr int Speed      { 4 };
    constexpr int LoopMode   { 5 };
    constexpr int Reverse    { 6 };
    constexpr int StartCue   { 7 };
    constexpr int EndCue     { 8 };
    constexpr int LoopCue    { 9 };
    constexpr int Attack     { 10 };
    constexpr int CueSet     { 11 };
    constexpr int ETrig      { 12 };
    constexpr int FiltFreq   { 13 };
    constexpr int FiltRes    { 14 };
    constexpr int none2      { 15 };
    constexpr int PitchShift { 16 };

    constexpr uint32_t getCvEnabledFlag (int parameterIndex)
    {
        return 1 << (parameterIndex + 1);
    }

    // TODO - this should be the parameter index
    juce::String getParameterName (uint32_t cvAssignFlag);
};

namespace ChannelFlags
{
    constexpr uint16_t kMute           { 0x0001 };
    constexpr uint16_t kSolo           { 0x0002 };
    constexpr uint16_t kNoGate         { 0x0004 };
    constexpr uint16_t kCueRandom      { 0x0008 };
    constexpr uint16_t kCueStepped     { 0x0010 };
    constexpr uint16_t kNeighborOutput { 0x0100 };
};

constexpr auto k8BitSize  { static_cast<int> (sizeof (uint8_t)) };
constexpr auto k16BitSize { static_cast<int> (sizeof (uint16_t)) };
constexpr auto k32BitSize { static_cast<int> (sizeof (uint32_t)) };
constexpr auto k64BitSize { static_cast<int> (sizeof (uint64_t)) };

// Field offsets and size
namespace SquidSalmple
{
    namespace DataLayout_186
    {
        const auto kBusyChunkSignatureAndVersionOffset { 0 };
        const auto kBusyChunkSignatureAndVersionSize { k32BitSize };
        const auto k_Reserved1Offset { kBusyChunkSignatureAndVersionOffset + kBusyChunkSignatureAndVersionSize };
        const auto k_Reserved1Size { 4 * k8BitSize };
        const auto kSampleStartOffset { k_Reserved1Offset + k_Reserved1Size };
        const auto kSampleStartSize { k32BitSize };
        const auto k_Reserved2Offset { kSampleStartOffset + kSampleStartSize };
        const auto k_Reserved2Size { 4 * k8BitSize };
        const auto kSampleEndOffset { k_Reserved2Offset + k_Reserved2Size };
        const auto kSampleEndSize { k32BitSize };
        const auto kEndOFDataOffset { kSampleEndOffset + kSampleEndSize };
        const auto kEndOfDataSize { k32BitSize };
        const auto kQualityOffset { kEndOFDataOffset + kEndOfDataSize };
        const auto kQualitySize { k8BitSize };
        const auto kLoopOffset { kQualityOffset + kQualitySize };   // NONE, NORMAL, ZIGZAG, GATE, ZIGZAG_GATE,
        const auto kLoopSize { k8BitSize };
        const auto k_Reserved3Offset { kLoopOffset + kLoopSize };
        const auto k_Reserved3Size { 2 * k8BitSize };
        const auto kLoopPositionOffset { k_Reserved3Offset + k_Reserved3Size };
        const auto kLoopPositionSize { k32BitSize };
        const auto k_Reserved4Offset { kLoopPositionOffset + kLoopPositionSize };
        const auto k_Reserved4Size { 5 * k8BitSize };
        const auto kXfadeOffset { k_Reserved4Offset + k_Reserved4Size };
        const auto kXfadeSize { k8BitSize };
        const auto kReverseOffset { kXfadeOffset + kXfadeSize };
        const auto kReverseSize { k8BitSize };
        const auto k_Reserved5Offset { kReverseOffset + kReverseSize };
        const auto k_Reserved5Size { 5 * k8BitSize };
        const auto kDecayOffset { k_Reserved5Offset + k_Reserved5Size };
        const auto kDecaySize { k16BitSize };
        const auto kAttackOffset { kDecayOffset + kDecaySize };
        const auto kAttackSize { k16BitSize };
        const auto k_Reserved6Offset { kAttackOffset + kAttackSize };
        const auto k_Reserved6Size { 2 * k8BitSize };
        const auto kCutoffFrequencyOffset { k_Reserved6Offset+ k_Reserved6Size };
        const auto kCutoffFrequencySize { k16BitSize };
        const auto kResonanceOffset { kCutoffFrequencyOffset + kCutoffFrequencySize };
        const auto kResonanceSize { k16BitSize };
        const auto kRateOffset { kResonanceOffset + kResonanceSize };
        const auto kRateSize { k8BitSize };
        const auto k_Reserved7Offset { kRateOffset + kRateSize };
        const auto k_Reserved7Size { 3 * k8BitSize };
        const auto kSpeedOffset { k_Reserved7Offset + k_Reserved7Size };
        const auto kSpeedSize { k16BitSize };
        const auto kLevelOffset { kSpeedOffset + kSpeedSize };
        const auto kLevelSize { k16BitSize };
        const auto kChannelSourceOffset { kLevelOffset + kLevelSize };
        const auto kChannelSourceSize { k8BitSize };
        const auto kRecDestOffset { kChannelSourceOffset + kChannelSourceSize };
        const auto kRecDestSize { k8BitSize };
        const auto k_Reserved8Offset { kRecDestOffset + kRecDestSize };
        const auto k_Reserved8Size { 12 * k8BitSize };
        const auto kCvFlagsOffset  { k_Reserved8Offset + k_Reserved8Size }; // cvflags
        const auto kCvFlagsSize { (kCvInputsCount + kCvInputsExtra) * k16BitSize };
        const auto kCvParamsOffset { kCvFlagsOffset + kCvFlagsSize }; // cvparams
        const auto kCvParamsSize { (kCvInputsCount + kCvInputsExtra) * (kCvParamsCount_186 + kCvParamsExtra) * 2 * k16BitSize };  // numCvs * numDestinations * dataSize
        const auto k_Reserved9Offset { kCvParamsOffset + kCvParamsSize }; // 4 times
        const auto k_Reserved9Size { (kCvParamsCount_186 + kCvParamsExtra) * k32BitSize };
        const auto kChannelFlagsOffset { k_Reserved9Offset + k_Reserved9Size };
        const auto kChannelFlagsSize { k16BitSize };
        const auto kStepTrigCurOffset { kChannelFlagsOffset + kChannelFlagsSize };
        const auto kStepTrigCurSize { k8BitSize };
        const auto kStepTrigNumOffset { kStepTrigCurOffset + kStepTrigCurSize }; // Off, 2, 3, 4, 5, 6, 7 ,8
        const auto kStepTrigNumSize { k8BitSize };
        const auto k_Reserved10Offset { kStepTrigNumOffset + kStepTrigNumSize };
        const auto k_Reserved10Size { 2 * k8BitSize };
        const auto kExternalTriggerOffset { k_Reserved10Offset + k_Reserved10Size }; // Off, > 1, > 2, > 3, > 4, > 5, > 6, > 7, > 8, On
        const auto kExternalTriggerSize { k8BitSize };
        const auto kQuantizeModeOffset { kExternalTriggerOffset + kExternalTriggerSize }; // none, chromatic, octave, major, minor, harmonicMinor, pentatonicMajor, pentatonicMinor, lydian, phrygian, japanese, rootAndFifth, oneChord, fourChord, fiveChord
        const auto kQuantizeModeSize { k8BitSize };
        const auto k_Reserved11Offset { kQuantizeModeOffset + kQuantizeModeSize };
        const auto k_Reserved11Size { k8BitSize };
        const auto kChokeOffset { k_Reserved11Offset + k_Reserved11Size }; // C1, C2, C3, C4, C5, C6, C7, C8
        const auto kChokeSize { k8BitSize };
        const auto k_Reserved12Offset { kChokeOffset + kChokeSize };
        const auto k_Reserved12Size { 2 * k8BitSize };
        const auto kCuesOffset { k_Reserved12Offset + k_Reserved12Size };
        const auto kCuesSize { (kCueNumSets * 3) * k32BitSize };
        const auto kCuesCountOffset { kCuesOffset + kCuesSize };
        const auto kCuesCountSize { k8BitSize };
        const auto kCuesQueuedOffset { kCuesCountOffset + kCuesCountSize };
        const auto kCuesQueuedSize { k8BitSize };
        const auto kCuesSelectedOffset { kCuesQueuedOffset + kCuesQueuedSize };
        const auto kCuesSelectedSize { k8BitSize };
        const auto k_Reserved13Offset { kCuesSelectedOffset + kCuesSelectedSize };
        const auto k_Reserved13Size { 253 * k8BitSize };
        const auto kEndOfData { k_Reserved13Offset + k_Reserved13Size };
    };

    namespace DataLayout_190
    {
        const auto kBusyChunkSignatureAndVersionOffset { 0 };
        const auto kBusyChunkSignatureAndVersionSize { k32BitSize };
        const auto k_Reserved1Offset { kBusyChunkSignatureAndVersionOffset + kBusyChunkSignatureAndVersionSize };
        const auto k_Reserved1Size { 4 * k8BitSize };
        const auto kSampleStartOffset { k_Reserved1Offset + k_Reserved1Size };
        const auto kSampleStartSize { k32BitSize };
        const auto k_Reserved2Offset { kSampleStartOffset + kSampleStartSize };
        const auto k_Reserved2Size { 4 * k8BitSize };
        const auto kSampleEndOffset { k_Reserved2Offset + k_Reserved2Size };
        const auto kSampleEndSize { k32BitSize };
        const auto kEndOFDataOffset { kSampleEndOffset + kSampleEndSize };
        const auto kEndOfDataSize { k32BitSize };
        const auto kQualityOffset { kEndOFDataOffset + kEndOfDataSize };
        const auto kQualitySize { k8BitSize };
        const auto kLoopOffset { kQualityOffset + kQualitySize };   // NONE, NORMAL, ZIGZAG, GATE, ZIGZAG_GATE,
        const auto kLoopSize { k8BitSize };
        const auto k_Reserved3Offset { kLoopOffset + kLoopSize };
        const auto k_Reserved3Size { 2 * k8BitSize };
        const auto kLoopPositionOffset { k_Reserved3Offset + k_Reserved3Size };
        const auto kLoopPositionSize { k32BitSize };
        const auto k_Reserved4Offset { kLoopPositionOffset + kLoopPositionSize };
        const auto k_Reserved4Size { 5 * k8BitSize };
        const auto kXfadeOffset { k_Reserved4Offset + k_Reserved4Size };
        const auto kXfadeSize { k8BitSize };
        const auto kReverseOffset { kXfadeOffset + kXfadeSize };
        const auto kReverseSize { k8BitSize };
        const auto k_Reserved5Offset { kReverseOffset + kReverseSize };
        const auto k_Reserved5Size { 5 * k8BitSize };
        const auto kDecayOffset { k_Reserved5Offset + k_Reserved5Size };
        const auto kDecaySize { k16BitSize };
        const auto kAttackOffset { kDecayOffset + kDecaySize };
        const auto kAttackSize { k16BitSize };
        const auto k_Reserved6Offset { kAttackOffset + kAttackSize };
        const auto k_Reserved6Size { 2 * k8BitSize };
        const auto kCutoffFrequencyOffset { k_Reserved6Offset + k_Reserved6Size };
        const auto kCutoffFrequencySize { k16BitSize };
        const auto kResonanceOffset { kCutoffFrequencyOffset + kCutoffFrequencySize };
        const auto kResonanceSize { k16BitSize };
        const auto kRateOffset { kResonanceOffset + kResonanceSize };
        const auto kRateSize { k8BitSize };
        const auto k_Reserved7Offset { kRateOffset + kRateSize };
        const auto k_Reserved7Size { 3 * k8BitSize };
        const auto kSpeedOffset { k_Reserved7Offset + k_Reserved7Size };
        const auto kSpeedSize { k16BitSize };
        const auto kLevelOffset { kSpeedOffset + kSpeedSize };
        const auto kLevelSize { k16BitSize };
        const auto kChannelSourceOffset { kLevelOffset + kLevelSize };
        const auto kChannelSourceSize { k8BitSize };
        const auto kRecDestOffset { kChannelSourceOffset + kChannelSourceSize };
        const auto kRecDestSize { k8BitSize };
        const auto k_Reserved8Offset { kRecDestOffset + kRecDestSize };
        const auto k_Reserved8Size { 12 * k8BitSize };
        const auto kCvFlagsOffset { k_Reserved8Offset + k_Reserved8Size }; // cvflags
        const auto kCvFlagsSize { (kCvInputsCount + kCvInputsExtra) * k32BitSize };
        const auto kCvParamsOffset { kCvFlagsOffset + kCvFlagsSize }; // cvparams
        const auto kCvParamsSize { (kCvInputsCount + kCvInputsExtra) * (kCvParamsCount_190 + kCvParamsExtra) * 2 * k16BitSize };  // numCvs * numDestinations * dataSize
        const auto k_Reserved9Offset { kCvParamsOffset + kCvParamsSize }; // 4 times
        const auto k_Reserved9Size { (kCvParamsCount_190 + kCvParamsExtra) * k32BitSize };
        const auto kChannelFlagsOffset { k_Reserved9Offset + k_Reserved9Size };
        const auto kChannelFlagsSize { k16BitSize };
        const auto kStepTrigCurOffset { kChannelFlagsOffset + kChannelFlagsSize };
        const auto kStepTrigCurSize { k8BitSize };
        const auto kStepTrigNumOffset { kStepTrigCurOffset + kStepTrigCurSize }; // Off, 2, 3, 4, 5, 6, 7 ,8
        const auto kStepTrigNumSize { k8BitSize };
        const auto k_Reserved10Offset { kStepTrigNumOffset + kStepTrigNumSize };
        const auto k_Reserved10Size { 2 * k8BitSize };
        const auto kExternalTriggerOffset { k_Reserved10Offset + k_Reserved10Size }; // Off, > 1, > 2, > 3, > 4, > 5, > 6, > 7, > 8, On
        const auto kExternalTriggerSize { k8BitSize };
        const auto kQuantizeModeOffset { kExternalTriggerOffset + kExternalTriggerSize }; // none, chromatic, octave, major, minor, harmonicMinor, pentatonicMajor, pentatonicMinor, lydian, phrygian, japanese, rootAndFifth, oneChord, fourChord, fiveChord
        const auto kQuantizeModeSize { k8BitSize };
        const auto k_Reserved11Offset { kQuantizeModeOffset + kQuantizeModeSize };
        const auto k_Reserved11Size { k8BitSize };
        const auto kChokeOffset { k_Reserved11Offset + k_Reserved11Size }; // C1, C2, C3, C4, C5, C6, C7, C8
        const auto kChokeSize { k8BitSize };
        const auto k_Reserved12Offset { kChokeOffset + kChokeSize };
        const auto k_Reserved12Size { 2 * k8BitSize };
        const auto kCuesOffset { k_Reserved12Offset + k_Reserved12Size };
        const auto kCuesSize { (kCueNumSets * 3) * k32BitSize };
        const auto kCuesCountOffset { kCuesOffset + kCuesSize };
        const auto kCuesCountSize { k8BitSize };
        const auto kCuesQueuedOffset { kCuesCountOffset + kCuesCountSize };
        const auto kCuesQueuedSize { k8BitSize };
        const auto kCuesSelectedOffset { kCuesQueuedOffset + kCuesQueuedSize };
        const auto kCuesSelectedSize { k8BitSize };
        const auto k_Reserved13Offset { kCuesSelectedOffset + kCuesSelectedSize }; // alignment padding
        const auto k_Reserved13Size { k8BitSize };
        const auto kPitchShiftOffset { k_Reserved13Offset + k_Reserved13Size };
        const auto kPitchShiftSize { k16BitSize };
        const auto k_Reserved14Offset { kPitchShiftOffset + kPitchShiftSize }; // padding
        const auto k_Reserved14Size { k16BitSize };
        const auto k_Reserved15Offset { k_Reserved14Offset + k_Reserved14Size };
        const auto k_Reserved15Size { 62 * k32BitSize };
        const auto kEndOfData { k_Reserved15Offset + k_Reserved15Size };
    };
};