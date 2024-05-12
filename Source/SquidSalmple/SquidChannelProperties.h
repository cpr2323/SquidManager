#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class SquidChannelProperties : public ValueTreeWrapper<SquidChannelProperties>
{
public:
    SquidChannelProperties () noexcept : ValueTreeWrapper (SquidChannelTypeId)
    {
    }

    SquidChannelProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper (SquidChannelTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setAttack (int attack, bool includeSelfCallback);
    void setBits (int bits, bool includeSelfCallback);
    void setChannelFlags (uint16_t channelFlags, bool includeSelfCallback);
    void setChannelIndex (uint8_t channelIndex, bool includeSelfCallback);
    void setChoke (int chokeChannel, bool includeSelfCallback);
    void setCuePoints (int cueSetIndex, uint32_t start, uint32_t loop, uint32_t end);
    void setCurCueSet (int cueSetIndex, bool includeSelfCallback);
    void setCvAssignAttenuate (int cvIndex, int parameterIndex, int attenuation, bool includeSelfCallback);
    void setCvAssignEnabled (int cvIndex, int parameterIndex, bool isEnabled, bool includeSelfCallback);
    void setCvAssignOffset (int cvIndex, int parameterIndex, int offset, bool includeSelfCallback);
    void setDecay (int decay, bool includeSelfCallback);
    void setETrig (int eTrig, bool includeSelfCallback);
    void setEndCue (uint32_t endCue, bool includeSelfCallback);
    void setEndCueSet (int cueSetIndex, uint32_t endCue, bool includeSelfCallback);
    void setFilterFrequency (int filterFrequency, bool includeSelfCallback);
    void setFilterResonance (int filterResonance, bool includeSelfCallback);
    void setFilterType (int filterType, bool includeSelfCallback);
    void setLevel (int level, bool includeSelfCallback);
    void setLoopCue (uint32_t loopCue, bool includeSelfCallback);
    void setLoopCueSet (int cueSetIndex, uint32_t loopCue, bool includeSelfCallback);
    void setLoopMode (int loopMode, bool includeSelfCallback);
    void setNumCueSets (int numCues, bool includeSelfCallback);
    void setQuant (int quant, bool includeSelfCallback);
    void setRate (int rate, bool includeSelfCallback);
    void setReverse (int reverse, bool includeSelfCallback);
    void setSpeed (int speed, bool includeSelfCallback); // internally 1 - 65500, externally 1-99
    void setStartCue (uint32_t startCue, bool includeSelfCallback);
    void setStartCueSet (int cueSetIndex, uint32_t startCue, bool includeSelfCallback);
    void setSteps (int steps, bool includeSelfCallback);
    void setXfade (int xfade, bool includeSelfCallback);
    void setReserved1Data (juce::String reservedData);
    void setReserved2Data (juce::String reservedData);
    void setReserved3Data (juce::String reservedData);
    void setReserved4Data (juce::String reservedData);
    void setReserved5Data (juce::String reservedData);
    void setReserved6Data (juce::String reservedData);
    void setReserved7Data (juce::String reservedData);
    void setReserved8Data (juce::String reservedData);
    void setReserved9Data (juce::String reservedData);
    void setReserved10Data (juce::String reservedData);
    void setReserved11Data (juce::String reservedData);
    void setReserved12Data (juce::String reservedData);
    void setReserved13Data (juce::String reservedData);

    int getAttack ();
    int getBits ();
    uint16_t getChannelFlags ();
    uint8_t getChannelIndex ();
    int getChoke ();
    int getCurCueSet ();
    int getCvAssignAttenuate (int cvIndex, int parameterIndex);
    bool getCvAssignEnabled (int cvIndex, int parameterIndex);
    int getCvAssignOffset (int cvIndex, int parameterIndex);
    int getDecay ();
    int getETrig ();
    uint32_t getEndCue ();
    uint32_t getEndCueSet (int cueSetIndex);
    int getFilterFrequency ();
    int getFilterResonance ();
    int getFilterType ();
    int getLevel ();
    uint32_t getLoopCue ();
    uint32_t getLoopCueSet (int cueSetIndex);
    int getLoopMode ();
    int getNumCueSets ();
    int getQuant ();
    int getRate ();
    int getReverse ();
    int getSpeed ();
    uint32_t getStartCue ();
    uint32_t getStartCueSet (int cueSetIndex);
    int getSteps ();
    int getXfade ();
    juce::String getReserved1Data ();
    juce::String getReserved2Data ();
    juce::String getReserved3Data ();
    juce::String getReserved4Data ();
    juce::String getReserved5Data ();
    juce::String getReserved6Data ();
    juce::String getReserved7Data ();
    juce::String getReserved8Data ();
    juce::String getReserved9Data ();
    juce::String getReserved10Data ();
    juce::String getReserved11Data ();
    juce::String getReserved12Data ();
    juce::String getReserved13Data ();

    void removeCueSet (int cueSetIndex);

    std::function<void (int attack)> onAttackChange;
    std::function<void (int bits)> onBitsChange;
    std::function<void (uint16_t channelFlags)> onChannelFlagsChange;
    std::function<void (uint8_t channelIndex)> onChannelIndexChange;
    std::function<void (int chokeChannel)> onChokeChange;
    std::function<void (int cueSetIndex)> onCurCueSetChange;
    std::function<void (int cvIndex, int parameterIndex, int attenuation)> onCvAssignAttenuateChange;
    std::function<void (int cvIndex, int parameterIndex, bool isEnabled)> onCvAssignEnabledChange;
    std::function<void (int cvIndex, int parameterIndex, int offset)> onCvAssignOffsetChange;
    std::function<void (int decay)> onDecayChange;
    std::function<void (int eTrig)> onETrigChange;
    std::function<void (uint32_t endCue)> onEndCueChange;
    std::function<void (int cueSetIndex, uint32_t endCue)> onEndCueSetChange;
    std::function<void (int filterFrequency)> onFilterFrequencyChange;
    std::function<void (int filterResonance)> onFilterResonanceChange;
    std::function<void (int filterType)> onFilterTypeChange;
    std::function<void (int level)> onLevelChange;
    std::function<void (uint32_t loopCue)> onLoopCueChange;
    std::function<void (int cueSetIndex, uint32_t loopCue)> onLoopCueSetChange;
    std::function<void (int loopMode)> onLoopModeChange;
    std::function<void (int numCueSets)> onNumCueSetsChange;
    std::function<void (int quant)> onQuantChange;
    std::function<void (int rate)> onRateChange;
    std::function<void (int reverse)> onReverseChange;
    std::function<void (int speed)> onSpeedChange;
    std::function<void (uint32_t startCue)> onStartCueChange;
    std::function<void (int cueSetIndex, uint32_t startCue)> onStartCueSetChange;
    std::function<void (int steps)> onStepsChange;
    std::function<void (int xfade)> onXfadeChange;

    void copyFrom (juce::ValueTree sourceVT);
    static juce::ValueTree create (uint8_t channelIndex);
    juce::ValueTree getCvParameterVT (int cvIndex, int paramterIndex);

    static inline const juce::Identifier SquidChannelTypeId { "SquidMetaData" };
    static inline const juce::Identifier AttackPropertyId           { "attack" };
    static inline const juce::Identifier BitsPropertyId             { "bits" };
    static inline const juce::Identifier ChannelFlagsPropertyId     { "channelFlags" };
    static inline const juce::Identifier ChannelIndexPropertyId     { "_index" };
    static inline const juce::Identifier ChokePropertyId            { "choke" };
    static inline const juce::Identifier CurCueSetPropertyId        { "curCueSet" };
    static inline const juce::Identifier DecayPropertyId            { "decay" };
    static inline const juce::Identifier ETrigPropertyId            { "eTrig" };
    static inline const juce::Identifier EndCuePropertyId           { "endCue" };
    static inline const juce::Identifier FilterFrequencyPropertyId  { "filterFrequency" };
    static inline const juce::Identifier FilterResonancePropertyId  { "filterResonance" };
    static inline const juce::Identifier FilterTypePropertyId       { "filterType" };
    static inline const juce::Identifier LevelPropertyId            { "level" };
    static inline const juce::Identifier LoopCuePropertyId          { "loopCue" };
    static inline const juce::Identifier LoopModePropertyId         { "loopMode" };
    static inline const juce::Identifier NumCueSetsPropertyId       { "numCueSets" };
    static inline const juce::Identifier QuantPropertyId            { "quant" };
    static inline const juce::Identifier RatePropertyId             { "rate" };
    static inline const juce::Identifier ReversePropertyId          { "reverse" };
    static inline const juce::Identifier SpeedPropertyId            { "speed" };
    static inline const juce::Identifier StartCuePropertyId         { "startCue" };
    static inline const juce::Identifier StepsPropertyId            { "steps" };
    static inline const juce::Identifier XfadePropertyId            { "xfade" };

    // CV ASSIGNS
    static inline const juce::Identifier CvAssignsTypeId { "CvAssigns" };
    static inline const juce::Identifier CvAssignInputTypeId { "CvInput" };
    static inline const juce::Identifier CvAssignInputIdPropertyId { "id" };
    static inline const juce::Identifier CvAssignInputParameterTypeId { "Parameter" };
    static inline const juce::Identifier CvAssignInputParameterIdPropertyId        { "id" };
    static inline const juce::Identifier CvAssignInputParameterEnabledPropertyId   { "enabled" };
    static inline const juce::Identifier CvAssignInputParameterAttenuatePropertyId { "attenuation" };
    static inline const juce::Identifier CvAssignInputParameterOffsetPropertyId    { "offset" };

    // CUE SETS
    static inline const juce::Identifier CueSetListTypeId { "CueSetList" };
    static inline const juce::Identifier CueSetTypeId { "CueSet" };
    static inline const juce::Identifier CueSetIdPropertyId    { "id" };
    static inline const juce::Identifier CueSetStartPropertyId { "start" };
    static inline const juce::Identifier CueSetLoopPropertyId  { "loop" };
    static inline const juce::Identifier CueSetEndPropertyId   { "end" };

    // RESERVED DATA
    static inline const juce::Identifier Reserved1DataPropertyId    { "reserved1Data" };
    static inline const juce::Identifier Reserved2DataPropertyId    { "reserved2Data" };
    static inline const juce::Identifier Reserved3DataPropertyId    { "reserved3Data" };
    static inline const juce::Identifier Reserved4DataPropertyId    { "reserved4Data" };
    static inline const juce::Identifier Reserved5DataPropertyId    { "reserved5Data" };
    static inline const juce::Identifier Reserved6DataPropertyId    { "reserved6Data" };
    static inline const juce::Identifier Reserved7DataPropertyId    { "reserved7Data" };
    static inline const juce::Identifier Reserved8DataPropertyId    { "reserved8Data" };
    static inline const juce::Identifier Reserved9DataPropertyId    { "reserved9Data" };
    static inline const juce::Identifier Reserved10DataPropertyId   { "reserved10Data" };
    static inline const juce::Identifier Reserved11DataPropertyId   { "reserved11Data" };
    static inline const juce::Identifier Reserved12DataPropertyId   { "reserved12Data" };
    static inline const juce::Identifier Reserved13DataPropertyId   { "reserved13Data" };

    void initValueTree ();
    void processValueTree () {}

private:
    juce::ValueTree getCueSetVT (int cueSetIndex);

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
