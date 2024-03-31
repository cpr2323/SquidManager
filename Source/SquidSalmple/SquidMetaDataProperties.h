#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class SquidMetaDataProperties : public ValueTreeWrapper<SquidMetaDataProperties>
{
public:
    SquidMetaDataProperties () noexcept : ValueTreeWrapper (SquidMetaDataTypeId)
    {
    }

    SquidMetaDataProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper (SquidMetaDataTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void addCueSet (juce::int64 startCue, juce::int64 loopCue, juce::int64 endCue);
    void setAttack (int attack, bool includeSelfCallback);
    void setBits (int bits, bool includeSelfCallback);
    void setCuePoints (int cueSet, juce::int64 start, juce::int64 loop, juce::int64 end);
    void setDecay (int decay, bool includeSelfCallback);
    void setEndCue (uint32_t endCue, bool includeSelfCallback);
    void setEndCueSet (int cueIndex, uint32_t endCue, bool includeSelfCallback);
    void setEuclidianTrigger (int euclidianTrigger, bool includeSelfCallback);
    void setFilterFrequency (int filterFrequency, bool includeSelfCallback);
    void setFilterResonance (int filterResonance, bool includeSelfCallback);
    void setFilterType (int filterType, bool includeSelfCallback);
    void setLevel (int level, bool includeSelfCallback);
    void setLoopCue (uint32_t loopCue, bool includeSelfCallback);
    void setLoopCueSet (int cueIndex, uint32_t loopCue, bool includeSelfCallback);
    void setLoopMode (int loopMode, bool includeSelfCallback);
    void setQuant (int quant, bool includeSelfCallback);
    void setRate (int rate, bool includeSelfCallback);
    void setReverse (int reverse, bool includeSelfCallback);
    void setSpeed (int speed, bool includeSelfCallback);
    void setStartCue (uint32_t startCue, bool includeSelfCallback);
    void setStartCueSet (int cueIndex, uint32_t startCue, bool includeSelfCallback);
    void setSteps (int steps, bool includeSelfCallback);
    void setXfade (int xfade, bool includeSelfCallback);

    int getAttack ();
    int getBits ();
    int getDecay ();
    uint32_t getEndCue ();
    uint32_t getEndCueSet (int cueIndex);
    int getEuclidianTrigger ();
    int getFilterFrequency ();
    int getFilterResonance ();
    int getFilterType ();
    int getLevel ();
    uint32_t getLoopCue ();
    uint32_t getLoopCueSet (int cueIndex);
    int getLoopMode ();
    int getNumCues ();
    int getQuant ();
    int getRate ();
    int getReverse ();
    int getSpeed ();
    uint32_t getStartCue ();
    uint32_t getStartCueSet (int cueIndex);
    int getSteps ();
    int getXfade ();

    std::function<void (int attack)> onAttackChange;
    std::function<void (int bits)> onBitsChange;
    std::function<void (int decay)> onDecayChange;
    std::function<void (uint32_t endCue)> onEndCueChange;
    std::function<void (int cueIndex, uint32_t endCue)> onEndCueSetChange;
    std::function<void (int euclidianTrigger)> onEuclidianTriggerChange;
    std::function<void (int filterFrequency)> onFilterFrequencyChange;
    std::function<void (int filterResonance)> onFilterResonanceChange;
    std::function<void (int filterType)> onFilterTypeChange;
    std::function<void (int level)> onLevelChange;
    std::function<void (uint32_t loopCue)> onLoopCueChange;
    std::function<void (int cueIndex, uint32_t loopCue)> onLoopCueSetChange;
    std::function<void (int loopMode)> onLoopModeChange;
    std::function<void (int quant)> onQuantChange;
    std::function<void (int rate)> onRateChange;
    std::function<void (int reverse)> onReverseChange;
    std::function<void (int speed)> onSpeedChange;
    std::function<void (uint32_t startCue)> onStartCueChange;
    std::function<void (int cueIndex, uint32_t startCue)> onStartCueSetChange;
    std::function<void (int steps)> onStepsChange;
    std::function<void (int xfade)> onXfadeChange;

    void copyFrom (juce::ValueTree sourceVT);
    static juce::ValueTree create ();

    static inline const juce::Identifier SquidMetaDataTypeId { "SquidMetaData" };
    static inline const juce::Identifier AttackPropertyId           { "attack" };
    static inline const juce::Identifier BitsPropertyId             { "bits" };
    static inline const juce::Identifier Cv1PropertyId              { "cv1" };
    static inline const juce::Identifier Cv2PropertyId              { "cv2" };
    static inline const juce::Identifier Cv3PropertyId              { "cv3" };
    static inline const juce::Identifier DecayPropertyId            { "decay" };
    static inline const juce::Identifier EndCuePropertyId           { "endCue" };
    static inline const juce::Identifier EuclidianTriggerPropertyId { "euclidianTrig" };
    static inline const juce::Identifier FilterFrequencyPropertyId  { "filterFrequency" };
    static inline const juce::Identifier FilterResonancePropertyId  { "filterResonance" };
    static inline const juce::Identifier FilterTypePropertyId       { "filterType" };
    static inline const juce::Identifier LevelPropertyId            { "level" };
    static inline const juce::Identifier LoopCuePropertyId          { "loopCue" };
    static inline const juce::Identifier LoopModePropertyId         { "loopMode" };
    static inline const juce::Identifier QuantPropertyId            { "quant" };
    static inline const juce::Identifier RatePropertyId             { "rate" };
    static inline const juce::Identifier ReversePropertyId          { "reverse" };
    static inline const juce::Identifier SpeedPropertyId            { "speed" };
    static inline const juce::Identifier StartCuePropertyId         { "startCue" };
    static inline const juce::Identifier StepsPropertyId            { "steps" };
    static inline const juce::Identifier XfadePropertyId            { "xfade" };

    static inline const juce::Identifier CueSetListTypeId { "CueSetList" };
    static inline const juce::Identifier CueSetTypeId { "CueSet" };
    static inline const juce::Identifier CueSetIdPropertyId    { "id" };
    static inline const juce::Identifier CueSetStartPropertyId { "start" };
    static inline const juce::Identifier CueSetLoopPropertyId  { "loop" };
    static inline const juce::Identifier CueSetEndPropertyId   { "end" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
