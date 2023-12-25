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

    void setBits (int bits, bool includeSelfCallback);
    void setRate (int rate, bool includeSelfCallback);
    void setSpeed (int speed, bool includeSelfCallback);
    void setFilter (int filter, bool includeSelfCallback);
    void setFilterFrequency (int filterFrequency, bool includeSelfCallback);
    void setFilterResonance (int filterResonance, bool includeSelfCallback);
    void setQuant (int quant, bool includeSelfCallback);
    void setLoop (int loop, bool includeSelfCallback);
    void setXfade (int xfade, bool includeSelfCallback);
    void setReverse (int reverse, bool includeSelfCallback);
    void setLevel (int level, bool includeSelfCallback);
    void setAttack (int attack, bool includeSelfCallback);
    void setDecay (int decay, bool includeSelfCallback);
    void setEuclidianTrigger (int euclidianTrigger, bool includeSelfCallback);
    void setSteps (int steps, bool includeSelfCallback);
    void setCueStart (int cueStart, bool includeSelfCallback);
    void setCueEnd (int cueEnd, bool includeSelfCallback);
    void setCueLoop (int cueLoop, bool includeSelfCallback);
    void setCv1 (int cv1, bool includeSelfCallback);
    void setCv2 (int cv2, bool includeSelfCallback);
    void setCv3 (int cv3, bool includeSelfCallback);

    int getBits ();
    int getRate ();
    int getSpeed ();
    int getFilter ();
    int getFilterFrequency ();
    int getFilterResonance ();
    int getQuant ();
    int getLoop ();
    int getXfade ();
    int getReverse ();
    int getLevel ();
    int getAttack ();
    int getDecay ();
    int getEuclidianTrigger ();
    int getSteps ();
    int getCueStart ();
    int getCueEnd ();
    int getCueLoop ();
    int getCv1 ();
    int getCv2 ();
    int getCv3 ();

    std::function<void (int bits)> onBitsChange;
    std::function<void (int rate)> onRateChange;
    std::function<void (int speed)> onSpeedChange;
    std::function<void (int filter)> onFilterChange;
    std::function<void (int filterFrequency)> onFilterFrequencyChange;
    std::function<void (int filterResonance)> onFilterResonanceChange;
    std::function<void (int quant)> onQuantChange;
    std::function<void (int loop)> onLoopChange;
    std::function<void (int xfade)> onXfadeChange;
    std::function<void (int reverse)> onReverseChange;
    std::function<void (int level)> onLevelChange;
    std::function<void (int attack)> onAttackChange;
    std::function<void (int decay)> onDecayChange;
    std::function<void (int euclidianTrigger)> onEuclidianTriggerChange;
    std::function<void (int steps)> onStepsChange;
    std::function<void (int cueStart)> onCueStartChange;
    std::function<void (int cueEnd)> onCueEndChange;
    std::function<void (int cueLoop)> onCueLoopChange;
    std::function<void (int cv1)> onCv1Change;
    std::function<void (int cv2)> onCv2Change;
    std::function<void (int cv3)> onCv3Change;

    void copyFrom (juce::ValueTree sourceVT);
    static juce::ValueTree create ();

    static inline const juce::Identifier SquidMetaDataTypeId { "SquidMetaData" };
    static inline const juce::Identifier BitsPropertyId             { "bits" };
    static inline const juce::Identifier RatePropertyId             { "rate" };
    static inline const juce::Identifier SpeedPropertyId            { "speed" };
    static inline const juce::Identifier FilterPropertyId           { "filter" };
    static inline const juce::Identifier FilterFrequencyPropertyId  { "filterFrequency" };
    static inline const juce::Identifier FilterResonancePropertyId  { "filterResonance" };
    static inline const juce::Identifier QuantPropertyId            { "quant" };
    static inline const juce::Identifier LoopPropertyId             { "loop" };
    static inline const juce::Identifier XfadePropertyId            { "xfade" };
    static inline const juce::Identifier ReversePropertyId          { "reverse" };
    static inline const juce::Identifier LevelPropertyId            { "level" };
    static inline const juce::Identifier AttackPropertyId           { "attack" };
    static inline const juce::Identifier DecayPropertyId            { "decay" };
    static inline const juce::Identifier EuclidianTriggerPropertyId { "euclidianTrig" };
    static inline const juce::Identifier StepsPropertyId            { "steps" };
    static inline const juce::Identifier CueStartPropertyId         { "cueStart" };
    static inline const juce::Identifier CueEndPropertyId           { "cueEnd" };
    static inline const juce::Identifier CueLoopPropertyId          { "cueLoop" };
    static inline const juce::Identifier Cv1PropertyId              { "cv1" };
    static inline const juce::Identifier Cv2PropertyId              { "cv2" };
    static inline const juce::Identifier Cv3PropertyId              { "cv3" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
