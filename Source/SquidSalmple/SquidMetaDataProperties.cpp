#include "SquidMetaDataProperties.h"

void SquidMetaDataProperties::setBits (int bits, bool includeSelfCallback)
{
    setValue (bits, BitsPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setRate (int rate, bool includeSelfCallback)
{
    setValue (rate, RatePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setSpeed (int speed, bool includeSelfCallback)
{
    setValue (speed, SpeedPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setFilter (int filter, bool includeSelfCallback)
{
    setValue (filter, FilterPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setFilterFrequency (int filterFrequency, bool includeSelfCallback)
{
    setValue (filterFrequency, FilterFrequencyPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setFilterResonance (int filterResonance, bool includeSelfCallback)
{
    setValue (filterResonance, FilterResonancePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setQuant (int quant, bool includeSelfCallback)
{
    setValue (quant, QuantPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setLoop (int loop, bool includeSelfCallback)
{
    setValue (loop, LoopPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setXfade (int xfade, bool includeSelfCallback)
{
    setValue (xfade, XfadePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setReverse (int reverse, bool includeSelfCallback)
{
    setValue (reverse, ReversePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setLevel (int level, bool includeSelfCallback)
{
    setValue (level, LevelPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setAttack (int attack, bool includeSelfCallback)
{
    setValue (attack, AttackPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setDecay (int decay, bool includeSelfCallback)
{
    setValue (decay, DecayPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setEuclidianTrigger (int euclidianTrigger, bool includeSelfCallback)
{
    setValue (euclidianTrigger, EuclidianTriggerPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setSteps (int steps, bool includeSelfCallback)
{
    setValue (steps, StepsPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setCueStart (int cueStart, bool includeSelfCallback)
{
    setValue (cueStart, CueStartPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setCueEnd (int cueEnd, bool includeSelfCallback)
{
    setValue (cueEnd, CueEndPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setCueLoop (int cueLoop, bool includeSelfCallback)
{
    setValue (cueLoop, CueLoopPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setCv1 (int cv1, bool includeSelfCallback)
{
    setValue (cv1, Cv1PropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setCv2 (int cv2, bool includeSelfCallback)
{
    setValue (cv2, Cv2PropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setCv3 (int cv3, bool includeSelfCallback)
{
    setValue (cv3, Cv3PropertyId, includeSelfCallback);
}

int SquidMetaDataProperties::getBits ()
{
    return getValue<int> (BitsPropertyId);
}

int SquidMetaDataProperties::getRate ()
{
    return getValue<int> (RatePropertyId);
}

int SquidMetaDataProperties::getSpeed ()
{
    return getValue<int> (SpeedPropertyId);
}

int SquidMetaDataProperties::getFilter ()
{
    return getValue<int> (FilterPropertyId);
}

int SquidMetaDataProperties::getFilterFrequency ()
{
    return getValue<int> (FilterFrequencyPropertyId);
}

int SquidMetaDataProperties::getFilterResonance ()
{
    return getValue<int> (FilterResonancePropertyId);
}

int SquidMetaDataProperties::getQuant ()
{
    return getValue<int> (QuantPropertyId);
}

int SquidMetaDataProperties::getLoop ()
{
    return getValue<int> (LoopPropertyId);
}

int SquidMetaDataProperties::getXfade ()
{
    return getValue<int> (XfadePropertyId);
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

int SquidMetaDataProperties::getEuclidianTrigger ()
{
    return getValue<int> (EuclidianTriggerPropertyId);
}

int SquidMetaDataProperties::getSteps ()
{
    return getValue<int> (StepsPropertyId);
}

int SquidMetaDataProperties::getCueStart ()
{
    return getValue<int> (CueStartPropertyId);
}

int SquidMetaDataProperties::getCueEnd ()
{
    return getValue<int> (CueEndPropertyId);
}

int SquidMetaDataProperties::getCueLoop ()
{
    return getValue<int> (CueLoopPropertyId);
}

int SquidMetaDataProperties::getCv1 ()
{
    return getValue<int> (Cv1PropertyId);
}

int SquidMetaDataProperties::getCv2 ()
{
    return getValue<int> (Cv2PropertyId);
}

int SquidMetaDataProperties::getCv3 ()
{
    return getValue<int> (Cv3PropertyId);
}

void SquidMetaDataProperties::copyFrom (juce::ValueTree sourceVT)
{
    SquidMetaDataProperties sourceMetaDataProperties (sourceVT, SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::no);
    setBits (sourceMetaDataProperties.getBits (), false);
    setRate (sourceMetaDataProperties.getRate (), false);
    setSpeed (sourceMetaDataProperties.getSpeed (), false);
    setFilter (sourceMetaDataProperties.getFilter (), false);
    setQuant (sourceMetaDataProperties.getQuant (), false);
    setLoop (sourceMetaDataProperties.getLoop (), false);
    setXfade (sourceMetaDataProperties.getXfade (), false);
    setReverse (sourceMetaDataProperties.getReverse (), false);
    setLevel (sourceMetaDataProperties.getLevel (), false);
    setAttack (sourceMetaDataProperties.getAttack (), false);
    setDecay (sourceMetaDataProperties.getDecay (), false);
    setEuclidianTrigger (sourceMetaDataProperties.getEuclidianTrigger (), false);
    setSteps (sourceMetaDataProperties.getSteps (), false);
    setCueStart (sourceMetaDataProperties.getCueStart (), false);
    setCueEnd (sourceMetaDataProperties.getCueEnd (), false);
    setCueLoop (sourceMetaDataProperties.getCueLoop (), false);
    setCv1 (sourceMetaDataProperties.getCv1 (), false);
    setCv2 (sourceMetaDataProperties.getCv2 (), false);
    setCv3 (sourceMetaDataProperties.getCv3 (), false);
}

juce::ValueTree SquidMetaDataProperties::create ()
{
    SquidMetaDataProperties metaDataProperties;
    return metaDataProperties.getValueTree ();
}

void SquidMetaDataProperties::initValueTree ()
{
    setBits (0, false);
    setRate (0, false);
    setSpeed (0, false);
    setFilter (0, false);
    setFilterFrequency (0, false);
    setFilterResonance (0, false);
    setQuant (0, false);
    setLoop (0, false);
    setXfade (0, false);
    setReverse (0, false);
    setLevel (0, false);
    setAttack (0, false);
    setDecay (0, false);
    setEuclidianTrigger (0, false);
    setSteps (0, false);
    setCueStart (0, false);
    setCueEnd (0, false);
    setCueLoop (0, false);
    setCv1 (0, false);
    setCv2 (0, false);
    setCv3 (0, false);
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
    else if (property == FilterPropertyId)
    {
        if (onFilterChange != nullptr)
            onFilterChange (getFilter ());
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
    else if (property == LoopPropertyId)
    {
        if (onLoopChange != nullptr)
            onLoopChange (getLoop ());
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
    else if (property == EuclidianTriggerPropertyId)
    {
        if (onEuclidianTriggerChange != nullptr)
            onEuclidianTriggerChange (getEuclidianTrigger ());
    }
    else if (property == StepsPropertyId)
    {
        if (onStepsChange != nullptr)
            onStepsChange (getSteps ());
    }
    else if (property == CueStartPropertyId)
    {
        if (onCueStartChange != nullptr)
            onCueStartChange (getCueStart ());
    }
    else if (property == CueEndPropertyId)
    {
        if (onCueEndChange != nullptr)
            onCueEndChange (getCueEnd ());
    }
    else if (property == CueLoopPropertyId)
    {
        if (onCueLoopChange != nullptr)
            onCueLoopChange (getCueLoop ());
    }
    else if (property == Cv1PropertyId)
    {
        if (onCv1Change != nullptr)
            onCv1Change (getCv1 ());
    }
    else if (property == Cv2PropertyId)
    {
        if (onCv2Change != nullptr)
            onCv2Change (getCv2 ());
    }
    else if (property == Cv3PropertyId)
    {
        if (onCv3Change != nullptr)
            onCv3Change (getCv3 ());
    }
}
