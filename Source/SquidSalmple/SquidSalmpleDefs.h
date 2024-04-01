#pragma once

#include <JuceHeader.h>

const auto kCueStart { 0 };
const auto kCueEnd   { 1 };
const auto kCueLoop  { 2 };
const auto kCueNumSets  { 64 };

const auto kCvInputsCount { 3 };
const auto kCvInputsExtra { 5 };

const auto kCvParamsCount { 16 };
const auto kCvParamsExtra { 4 };

const uint64_t kSignatureAndVersionCurrent { 0x12345676 };

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

constexpr auto k8BitSize  { static_cast<int> (sizeof (uint8_t)) };
constexpr auto k16BitSize { static_cast<int> (sizeof (uint16_t)) };
constexpr auto k32BitSize { static_cast<int> (sizeof (uint32_t)) };
constexpr auto k64BitSize { static_cast<int> (sizeof (uint64_t)) };

// Field offsets and size
namespace SquidSalmple
{
    namespace DataLayout
    {
        const auto kBusyChunkSignatureAndVersionOffset { 0 };
        const auto kBusyChunkSignatureAndVersionSize { k32BitSize };
        const auto k_Reserved1Offset { kBusyChunkSignatureAndVersionOffset + kBusyChunkSignatureAndVersionSize }; // play state
        const auto k_Reserved1Size { k8BitSize };
        const auto k_Pad1Offset { k_Reserved1Offset + k_Reserved1Size };
        const auto k_Pad1Size { 3 * k8BitSize };
        const auto kSampleStartOffset { k_Pad1Offset+ k_Pad1Size };
        const auto kSampleStartSize { k32BitSize };
        const auto k_Reserved2Offset { kSampleStartOffset + kSampleStartSize }; // sample position
        const auto k_Reserved2Size { k32BitSize };
        const auto kSampleEndOffset { k_Reserved2Offset + k_Reserved2Size };
        const auto kSampleEndSize { k32BitSize };
        const auto k_Reserved3Offset { kSampleEndOffset + kSampleEndSize }; // data end
        const auto k_Reserved3Size { 4 * k8BitSize };
        const auto kQualityOffset { k_Reserved3Offset + k_Reserved3Size };
        const auto kQualitySize { k8BitSize };
        const auto kLoopOffset { kQualityOffset + kQualitySize };   // NONE, NORMAL, ZIGZAG, GATE, ZIGZAG_GATE,
        const auto kLoopSize { k8BitSize };
        const auto k_Pad3Offset { kLoopOffset + kLoopSize };
        const auto k_Pad3Size { 2 * k8BitSize };
        const auto kLoopPositionOffset { k_Pad3Offset + k_Pad3Size };
        const auto kLoopPositionSize { k32BitSize };
        const auto k_Reserved4Offset { kLoopPositionOffset + kLoopPositionSize }; // loop env position (4) & loopzigzagreverse (1)
        const auto k_Reserved4Size { 5 * k8BitSize };
        const auto kXfadeOffset { k_Reserved4Offset + k_Reserved4Size };
        const auto kXfadeSize { k8BitSize };
        const auto kReverseOffset { kXfadeOffset + kXfadeSize };
        const auto kReverseSize { k8BitSize };
        const auto k_Pad6Offset { kReverseOffset + kReverseSize };
        const auto k_Pad6Size { k8BitSize };
        const auto k_Reserved5Offset { k_Pad6Offset + k_Pad6Size }; // acum (2) & accum inc (2)
        const auto k_Reserved5Size { 4 * k8BitSize };
        const auto kDecayOffset { k_Reserved5Offset + k_Reserved5Size };
        const auto kDecaySize { k16BitSize };
        const auto kAttackOffset { kDecayOffset + kDecaySize };
        const auto kAttackSize { k16BitSize };
        const auto k_Reserved6Offset { kAttackOffset + kAttackSize }; // env state
        const auto k_Reserved6Size { 2 * k8BitSize };
        const auto kCutoffFrequencyOffset { k_Reserved6Offset+ k_Reserved6Size };
        const auto kCutoffFrequencySize { k16BitSize };
        const auto kResonanceOffset { kCutoffFrequencyOffset + kCutoffFrequencySize };
        const auto kResonanceSize { k16BitSize };
        const auto kRateOffset { kResonanceOffset + kResonanceSize };
        const auto kRateSize { k8BitSize };
        const auto k_Reserved7Offset { kRateOffset + kRateSize }; // rate count (1), rate inc (2)
        const auto k_Reserved7Size { 3 * k8BitSize };
        const auto kSpeedOffset { k_Reserved7Offset + k_Reserved7Size };
        const auto kSpeedSize { k16BitSize };
        const auto kLevelOffset { kSpeedOffset + kSpeedSize };
        const auto kLevelSize { k16BitSize };
        const auto k_Reserved8Offset { kLevelOffset + kLevelSize }; // bank src, bank dest, banksavedstart, banksavedend, banksavedloop
        const auto k_Reserved8Size { 14 * k8BitSize };
        const auto k_Reserved8aOffset { k_Reserved8Offset + k_Reserved8Size }; // cvflags
        const auto k_Reserved8aSize { 8 * k16BitSize };
        const auto k_Reserved8bOffset { k_Reserved8aOffset + k_Reserved8aSize }; // cvparams
        const auto k_Reserved8bSize { 8 * 40 * k16BitSize };
        const auto k_Reserved8cOffset { k_Reserved8bOffset + k_Reserved8bSize }; // CvParamsOriginalValue
        const auto k_Reserved8cSize { 20 * k32BitSize };
        const auto k_Reserved8dOffset { k_Reserved8cOffset + k_Reserved8cSize }; // flags
        const auto k_Reserved8dSize { k16BitSize };
        const auto kStepTrigCurOffset { k_Reserved8dOffset + k_Reserved8dSize }; 
        const auto kStepTrigCurSize { k8BitSize };
        const auto kStepTrigNumOffset { kStepTrigCurOffset + kStepTrigCurSize };
        const auto kStepTrigNumSize { k8BitSize };
        const auto k_Reserved9Offset { kStepTrigNumOffset + kStepTrigNumSize }; // misc3
        const auto k_Reserved9Size { k16BitSize };
        const auto kExternalTriggerOffset { k_Reserved9Offset + k_Reserved9Size };
        const auto kExternalTriggerSize { k8BitSize };
        const auto kQuantizeModeOffset { kExternalTriggerOffset + kExternalTriggerSize }; //     none, chromatic, octave, major, minor, harmonicMinor, pentatonicMajor, pentatonicMinor, lydian, phrygian, japanese, rootAndFifth, oneChord, fourChord, fiveChord
        const auto kQuantizeModeSize { k8BitSize };
        const auto k_Reserved10Offset { kQuantizeModeOffset + kQuantizeModeSize }; // zoom amount
        const auto k_Reserved10Size { k8BitSize };
        const auto kChokeOffset { k_Reserved10Offset + k_Reserved10Size };
        const auto kChokeSize { k8BitSize };
        const auto k_PadXOffset { kChokeOffset + kChokeSize };
        const auto k_PadXSize { 2 * k8BitSize };
        const auto kCuesOffset { k_PadXOffset + k_PadXSize };
        const auto kCuesSize { (kCueNumSets * 3) * k32BitSize };
        const auto kCuesCountOffset { kCuesOffset + kCuesSize };
        const auto kCuesCountSize { k8BitSize };
        const auto kCuesQueuedOffset { kCuesCountOffset + kCuesCountSize};
        const auto kCuesQueuedSize { k8BitSize };
        const auto kCuesSelectedOffset { kCuesQueuedOffset + kCuesQueuedSize };
        const auto kCuesSelectedSize { k8BitSize };
        const auto k_PadYOffset { kCuesSelectedOffset + kCuesSelectedSize };
        const auto k_PadYSize { k8BitSize };
        const auto k_Reserved11Offset { k_PadYOffset + k_PadYSize };
        const auto k_Reserved11Size { 63 * k32BitSize };
        const auto kEndOfData { k_Reserved11Offset + k_Reserved11Size };
    };
};
