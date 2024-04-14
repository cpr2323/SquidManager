#include "SquidMetaDataProperties.h"
#include "../Utility/ValueTreeHelpers.h"

void SquidMetaDataProperties::setBits (int bits, bool includeSelfCallback)
{
    setValue (bits, BitsPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setChoke (int chokeChannel, bool includeSelfCallback)
{
    setValue (chokeChannel, ChokePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setCuePoints (int cueSet, juce::int64 start, juce::int64 loop, juce::int64 end)
{
    if (getNumCues () <= cueSet)
        return;

    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    jassert (CueSetListTypeId.isValid ());
    auto cueListCount { 0 };
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, cueSet, start, loop, end, &cueListCount] (juce::ValueTree cueSetVT)
    {
        if (cueListCount == cueSet)
        {
            cueSetVT.setProperty (CueSetStartPropertyId, start, nullptr);
            cueSetVT.setProperty (CueSetLoopPropertyId, loop, nullptr);
            cueSetVT.setProperty (CueSetEndPropertyId, end, nullptr);
            return false;
        }
        ++cueListCount;
        return true;
    });
}

void SquidMetaDataProperties::setCurCueSet (int cueSetIndex, bool includeSelfCallback)
{
    setValue (cueSetIndex, CurCueSetPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setRate (int rate, bool includeSelfCallback)
{
    setValue (rate, RatePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setSpeed (int speed, bool includeSelfCallback)
{
    setValue (speed, SpeedPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setFilterFrequency (int filterFrequency, bool includeSelfCallback)
{
    setValue (filterFrequency, FilterFrequencyPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setFilterResonance (int filterResonance, bool includeSelfCallback)
{
    setValue (filterResonance, FilterResonancePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setFilterType (int filter, bool includeSelfCallback)
{
    setValue (filter, FilterTypePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setQuant (int quant, bool includeSelfCallback)
{
    setValue (quant, QuantPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setLoopMode (int loopMode, bool includeSelfCallback)
{
    setValue (loopMode, LoopModePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setXfade (int xfade, bool includeSelfCallback)
{
    setValue (xfade, XfadePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setReserved1Data (juce::String reservedData)
{
    setValue (reservedData, Reserved1DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved2Data (juce::String reservedData)
{
    setValue (reservedData, Reserved2DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved3Data (juce::String reservedData)
{
    setValue (reservedData, Reserved3DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved4Data (juce::String reservedData)
{
    setValue (reservedData, Reserved4DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved5Data (juce::String reservedData)
{
    setValue (reservedData, Reserved5DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved6Data (juce::String reservedData)
{
    setValue (reservedData, Reserved6DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved7Data (juce::String reservedData)
{
    setValue (reservedData, Reserved7DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved8Data (juce::String reservedData)
{
    setValue (reservedData, Reserved8DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved9Data (juce::String reservedData)
{
    setValue (reservedData, Reserved9DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved10Data (juce::String reservedData)
{
    setValue (reservedData, Reserved10DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved11Data (juce::String reservedData)
{
    setValue (reservedData, Reserved11DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved12Data (juce::String reservedData)
{
    setValue (reservedData, Reserved12DataPropertyId, false);
}
void SquidMetaDataProperties::setReserved13Data (juce::String reservedData)
{
    setValue (reservedData, Reserved13DataPropertyId, false);
}

void SquidMetaDataProperties::setReverse (int reverse, bool includeSelfCallback)
{
    setValue (reverse, ReversePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setLevel (int level, bool includeSelfCallback)
{
    setValue (level, LevelPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::addCueSet (juce::int64 startCue, juce::int64 loopCue, juce::int64 endCue)
{
    const auto numCues { getNumCues () };
    if (getNumCues () >= 64)
        return;

    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    juce::ValueTree cueSet { CueSetTypeId };
    cueSet.setProperty (CueSetIdPropertyId, numCues + 1, nullptr);
    cueSet.setProperty (CueSetStartPropertyId, startCue, nullptr);
    cueSet.setProperty (CueSetLoopPropertyId, loopCue, nullptr);
    cueSet.setProperty (CueSetEndPropertyId, endCue, nullptr);
    cueSetListVT.addChild (cueSet, -1, nullptr);
}

void SquidMetaDataProperties::setAttack (int attack, bool includeSelfCallback)
{
    setValue (attack, AttackPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setDecay (int decay, bool includeSelfCallback)
{
    setValue (decay, DecayPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setETrig (int eTrig, bool includeSelfCallback)
{
    setValue (eTrig, ETrigPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setSteps (int steps, bool includeSelfCallback)
{
    setValue (steps, StepsPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setStartCue (uint32_t startCue, bool includeSelfCallback)
{
    setValue (static_cast<juce::int64> (startCue), StartCuePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setEndCue (uint32_t endCue, bool includeSelfCallback)
{
    setValue (static_cast<juce::int64> (endCue), EndCuePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setLoopCue (uint32_t loopCue, bool includeSelfCallback)
{
    setValue (static_cast<juce::int64> (loopCue), LoopCuePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setStartCueSet (int cueIndex, uint32_t startCue, bool includeSelfCallback)
{
    if (getNumCues () <= cueIndex)
        return;

    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    jassert (CueSetListTypeId.isValid ());
    auto cueListCount { 0 };
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, cueIndex, startCue, &cueListCount] (juce::ValueTree cueSetVT)
    {
        if (cueListCount == cueIndex)
        {
            cueSetVT.setProperty (CueSetStartPropertyId, static_cast<juce::int64>(startCue), nullptr);
            return false;
        }
        ++cueListCount;
        return true;
    });
}

void SquidMetaDataProperties::setEndCueSet (int cueIndex, uint32_t endCue, bool includeSelfCallback)
{
    if (getNumCues () <= cueIndex)
        return;

    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    jassert (CueSetListTypeId.isValid ());
    auto cueListCount { 0 };
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, cueIndex, endCue, &cueListCount] (juce::ValueTree cueSetVT)
    {
        if (cueListCount == cueIndex)
        {
            cueSetVT.setProperty (CueSetEndPropertyId, static_cast<juce::int64>(endCue), nullptr);
            return false;
        }
        ++cueListCount;
        return true;
    });
}

void SquidMetaDataProperties::setLoopCueSet (int cueIndex, uint32_t loopCue, bool includeSelfCallback)
{
    if (getNumCues () <= cueIndex)
        return;

    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    jassert (CueSetListTypeId.isValid ());
    auto cueListCount { 0 };
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, cueIndex, loopCue, &cueListCount] (juce::ValueTree cueSetVT)
    {
        if (cueListCount == cueIndex)
        {
            cueSetVT.setProperty (CueSetLoopPropertyId, static_cast<juce::int64>(loopCue), nullptr);
            return false;
        }
        ++cueListCount;
        return true;
    });
}

int SquidMetaDataProperties::getBits ()
{
    return getValue<int> (BitsPropertyId);
}

int SquidMetaDataProperties::getChoke ()
{
    return getValue<int> (ChokePropertyId);
}

int SquidMetaDataProperties::getCurCueSet ()
{
    return getValue<int> (CurCueSetPropertyId);
}

int SquidMetaDataProperties::getRate ()
{
    return getValue<int> (RatePropertyId);
}

int SquidMetaDataProperties::getSpeed ()
{
    return getValue<int> (SpeedPropertyId);
}

int SquidMetaDataProperties::getFilterFrequency ()
{
    return getValue<int> (FilterFrequencyPropertyId);
}

int SquidMetaDataProperties::getFilterResonance ()
{
    return getValue<int> (FilterResonancePropertyId);
}

int SquidMetaDataProperties::getFilterType ()
{
    return getValue<int> (FilterTypePropertyId);
}

int SquidMetaDataProperties::getQuant ()
{
    return getValue<int> (QuantPropertyId);
}

int SquidMetaDataProperties::getLoopMode ()
{
    return getValue<int> (LoopModePropertyId);
}

int SquidMetaDataProperties::getNumCues ()
{
    auto cueSetListVT {data.getChildWithName(CueSetListTypeId)};
    jassert (CueSetListTypeId.isValid ());
    auto cueListCount { 0 };
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, &cueListCount] (juce::ValueTree cueSetVT)
    {
        ++cueListCount;
        return true;
    });
    return cueListCount;
}

int SquidMetaDataProperties::getXfade ()
{
    return getValue<int> (XfadePropertyId);
}

juce::String SquidMetaDataProperties::getReserved1Data ()
{
    return getValue<juce::String> (Reserved1DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved2Data ()
{
    return getValue<juce::String> (Reserved2DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved3Data ()
{
    return getValue<juce::String> (Reserved3DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved4Data ()
{
    return getValue<juce::String> (Reserved4DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved5Data ()
{
    return getValue<juce::String> (Reserved5DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved6Data ()
{
    return getValue<juce::String> (Reserved6DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved7Data ()
{
    return getValue<juce::String> (Reserved7DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved8Data ()
{
    return getValue<juce::String> (Reserved8DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved9Data ()
{
    return getValue<juce::String> (Reserved9DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved10Data ()
{
    return getValue<juce::String> (Reserved10DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved11Data ()
{
    return getValue<juce::String> (Reserved11DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved12Data ()
{
    return getValue<juce::String> (Reserved12DataPropertyId);
}
juce::String SquidMetaDataProperties::getReserved13Data ()
{
    return getValue<juce::String> (Reserved13DataPropertyId);
}

int SquidMetaDataProperties::getReverse ()
{
    return getValue<int> (ReversePropertyId);
}

int SquidMetaDataProperties::getLevel ()
{
    return getValue<int> (LevelPropertyId);
}

int SquidMetaDataProperties::getAttack ()
{
    return getValue<int> (AttackPropertyId);
}

int SquidMetaDataProperties::getDecay ()
{
    return getValue<int> (DecayPropertyId);
}

int SquidMetaDataProperties::getETrig ()
{
    return getValue<int> (ETrigPropertyId);
}

int SquidMetaDataProperties::getSteps ()
{
    return getValue<int> (StepsPropertyId);
}

uint32_t SquidMetaDataProperties::getStartCue ()
{
    return static_cast<uint32_t> (getValue<juce::int64> (StartCuePropertyId));
}

uint32_t SquidMetaDataProperties::getEndCue ()
{
    return static_cast<uint32_t> (getValue<juce::int64> (EndCuePropertyId));
}

uint32_t SquidMetaDataProperties::getLoopCue ()
{
    return static_cast<uint32_t> (getValue<juce::int64> (LoopCuePropertyId));
}

uint32_t SquidMetaDataProperties::getStartCueSet (int cueIndex)
{
    if (getNumCues () <= cueIndex)
        return 0;

    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    jassert (CueSetListTypeId.isValid ());
    auto cueListCount { 0 };
    auto startCue { -1 };
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, cueIndex, &cueListCount, &startCue] (juce::ValueTree cueSetVT)
    {
        if (cueListCount == cueIndex)
        {
            startCue = cueSetVT.getProperty (CueSetStartPropertyId);
            return false;
        }
        ++cueListCount;
        return true;
    });

    return startCue;
}

uint32_t SquidMetaDataProperties::getEndCueSet (int cueIndex)
{
    if (getNumCues () <= cueIndex)
        return 0;

    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    jassert (CueSetListTypeId.isValid ());
    auto cueListCount { 0 };
    auto endCue { -1 };
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, cueIndex, &cueListCount, &endCue] (juce::ValueTree cueSetVT)
    {
        if (cueListCount == cueIndex)
        {
            endCue = cueSetVT.getProperty (CueSetEndPropertyId);
            return false;
        }
        ++cueListCount;
        return true;
    });

    return endCue;
}

uint32_t SquidMetaDataProperties::getLoopCueSet (int cueIndex)
{
    if (getNumCues () <= cueIndex)
        return 0;

    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    jassert (CueSetListTypeId.isValid ());
    auto cueListCount { 0 };
    auto loopCue { -1 };
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, cueIndex, &cueListCount, &loopCue] (juce::ValueTree cueSetVT)
    {
        if (cueListCount == cueIndex)
        {
            loopCue = cueSetVT.getProperty (CueSetLoopPropertyId);
            return false;
        }
        ++cueListCount;
        return true;
    });

    return loopCue;
}

void SquidMetaDataProperties::copyFrom (juce::ValueTree sourceVT)
{
    SquidMetaDataProperties sourceMetaDataProperties (sourceVT, SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::no);
    setAttack (sourceMetaDataProperties.getAttack (), false);
    setBits (sourceMetaDataProperties.getBits (), false);
    setChoke (sourceMetaDataProperties.getChoke (), false);
    setCurCueSet (sourceMetaDataProperties.getCurCueSet (), false);
    setDecay (sourceMetaDataProperties.getDecay (), false);
    setEndCue (sourceMetaDataProperties.getEndCue (), false);
    setFilterFrequency (sourceMetaDataProperties.getFilterFrequency (), false);
    setFilterResonance (sourceMetaDataProperties.getFilterResonance (), false);
    setFilterType (sourceMetaDataProperties.getFilterType (), false);
    setLoopCue (sourceMetaDataProperties.getLoopCue (), false);
    setLoopMode (sourceMetaDataProperties.getLoopMode (), false);
    setLevel (sourceMetaDataProperties.getLevel (), false);
    setQuant (sourceMetaDataProperties.getQuant (), false);
    setRate (sourceMetaDataProperties.getRate (), false);
    setReverse (sourceMetaDataProperties.getReverse (), false);
    setSpeed (sourceMetaDataProperties.getSpeed (), false);
    setStartCue (sourceMetaDataProperties.getStartCue (), false);
    setSteps (sourceMetaDataProperties.getSteps (), false);
    setXfade (sourceMetaDataProperties.getXfade (), false);
}

juce::ValueTree SquidMetaDataProperties::create ()
{
    SquidMetaDataProperties metaDataProperties;
    return metaDataProperties.getValueTree ();
}

void SquidMetaDataProperties::initValueTree ()
{
    setAttack (0, false);
    setBits (16, false);
    setChoke (0, false);
    setCurCueSet (0, false);
    setDecay (0, false);
    setEndCue (0, false);
    setFilterFrequency (0, false);
    setFilterResonance (0, false);
    setFilterType (0, false);
    setLoopCue (0, false);
    setLoopMode (0, false);
    setLevel (30, false);
    setQuant (0, false);
    setRate (7, false); // 7 == 44.1k
    setReverse (0, false);
    setSpeed (50, false); 
    setStartCue (0, false);
    setSteps (0, false);
    setXfade (0, false);

    juce::ValueTree cueSetList { CueSetListTypeId };
    data.addChild (cueSetList, -1, nullptr);
}

void SquidMetaDataProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt != data)
        return;

    if (property == BitsPropertyId)
    {
        if (onBitsChange != nullptr)
            onBitsChange (getBits ());
    }
    else if (property == ChokePropertyId)
    {
        if (onChokeChange != nullptr)
            onChokeChange (getChoke ());
    }
    else if (property == CurCueSetPropertyId)
    {
        if (onCurCueSetChange != nullptr)
            onCurCueSetChange (getCurCueSet ());
    }
    else if (property == ETrigPropertyId)
    {
        if (onETrigChange!= nullptr)
            onETrigChange (getETrig ());
    }
    else if (property == RatePropertyId)
    {
        if (onRateChange != nullptr)
            onRateChange (getRate ());
    }
    else if (property == SpeedPropertyId)
    {
        if (onSpeedChange != nullptr)
            onSpeedChange (getSpeed ());
    }
    else if (property == FilterTypePropertyId)
    {
        if (onFilterTypeChange != nullptr)
            onFilterTypeChange (getFilterType ());
    }
    else if (property == FilterFrequencyPropertyId)
    {
        if (onFilterFrequencyChange != nullptr)
            onFilterFrequencyChange (getFilterFrequency ());
    }
    else if (property == FilterResonancePropertyId)
    {
        if (onFilterResonanceChange != nullptr)
            onFilterResonanceChange (getFilterResonance ());
    }
    else if (property == QuantPropertyId)
    {
        if (onQuantChange != nullptr)
            onQuantChange (getQuant ());
    }
    else if (property == LoopModePropertyId)
    {
        if (onLoopModeChange != nullptr)
            onLoopModeChange (getLoopMode ());
    }
    else if (property == XfadePropertyId)
    {
        if (onXfadeChange != nullptr)
            onXfadeChange (getXfade ());
    }
    else if (property == ReversePropertyId)
    {
        if (onReverseChange != nullptr)
            onReverseChange (getReverse ());
    }
    else if (property == LevelPropertyId)
    {
        if (onLevelChange != nullptr)
            onLevelChange (getLevel ());
    }
    else if (property == AttackPropertyId)
    {
        if (onAttackChange != nullptr)
            onAttackChange (getAttack ());
    }
    else if (property == DecayPropertyId)
    {
        if (onDecayChange != nullptr)
            onDecayChange (getDecay ());
    }
    else if (property == StepsPropertyId)
    {
        if (onStepsChange != nullptr)
            onStepsChange (getSteps ());
    }
    else if (property == StartCuePropertyId)
    {
        if (onStartCueChange != nullptr)
            onStartCueChange (getStartCue ());
    }
    else if (property == EndCuePropertyId)
    {
        if (onEndCueChange != nullptr)
            onEndCueChange (getEndCue ());
    }
    else if (property == LoopCuePropertyId)
    {
        if (onLoopCueChange != nullptr)
            onLoopCueChange (getLoopCue ());
    }
}
