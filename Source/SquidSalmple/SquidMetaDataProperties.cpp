#include "SquidMetaDataProperties.h"
#include "SquidSalmpleDefs.h"
#include "../Utility/ValueTreeHelpers.h"

void SquidMetaDataProperties::initValueTree ()
{
    setAttack (0, false);
    setBits (16, false);
    setChoke (0, false);
    setDecay (0, false);
    setFilterFrequency (0, false);
    setFilterResonance (0, false);
    setFilterType (0, false);
    setLoopMode (0, false);
    setLevel (30, false);
    setQuant (0, false);
    setRate (7, false); // 7 == 44.1k
    setReverse (0, false);
    setSpeed (50, false);
    setSteps (0, false);
    setXfade (0, false);

    setReserved1Data ("");
    setReserved2Data ("");
    setReserved3Data ("");
    setReserved4Data ("");
    setReserved5Data ("");
    setReserved6Data ("");
    setReserved7Data ("");
    setReserved8Data ("");
    setReserved9Data ("");
    setReserved10Data ("");
    setReserved11Data ("");
    setReserved12Data ("");
    setReserved13Data ("");

    // TODO - I can probably remove this, as it is just test code validating that getCvEnabledFlag works
    jassert (CvAssignedFlag::bits == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::Bits));
    jassert (CvAssignedFlag::rate == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::Rate));
    jassert (CvAssignedFlag::level == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::Level));
    jassert (CvAssignedFlag::decay == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::Decay));
    jassert (CvAssignedFlag::speed == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::Speed));
    jassert (CvAssignedFlag::loopMode == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::LoopMode));
    jassert (CvAssignedFlag::reverse == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::Reverse));
    jassert (CvAssignedFlag::startCue == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::StartCue));
    jassert (CvAssignedFlag::endCue == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::EndCue));
    jassert (CvAssignedFlag::loopCue == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::LoopCue));
    jassert (CvAssignedFlag::attack == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::Attack));
    jassert (CvAssignedFlag::cueSet == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::CueSet));
    jassert (CvAssignedFlag::eTrig == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::ETrig));
    jassert (CvAssignedFlag::filtFreq == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::FiltFreq));
    jassert (CvAssignedFlag::filtRes == CvParameterIndex::getCvEnabledFlag (CvParameterIndex::FiltRes));

    // CV ASSIGNS
    juce::ValueTree cvAssignsVT { SquidMetaDataProperties::CvAssignsTypeId };
    for (auto curCvInput { 0 }; curCvInput < kCvInputsCount + kCvInputsExtra; ++curCvInput)
    {
        juce::ValueTree cvInputVT { CvAssignInputTypeId };
        cvInputVT.setProperty (CvAssignInputIdPropertyId, curCvInput + 1, nullptr);
        for (auto curParameterIndex { 0 }; curParameterIndex < 15; ++curParameterIndex)
        {
            juce::ValueTree parameterVT { CvAssignInputParameterTypeId };
            parameterVT.setProperty (CvAssignInputParameterIdPropertyId, curParameterIndex + 1, nullptr);
            parameterVT.setProperty (CvAssignInputParameterEnabledPropertyId, "false", nullptr);
            parameterVT.setProperty (CvAssignInputParameterAttenuatePropertyId, 99, nullptr);
            parameterVT.setProperty (CvAssignInputParameterOffsetPropertyId, 0, nullptr);
            cvInputVT.addChild (parameterVT, -1, nullptr);
        }
        cvAssignsVT.addChild (cvInputVT, -1, nullptr);
    }
    data.addChild (cvAssignsVT, -1, nullptr);

    // CUE SETS
    juce::ValueTree cueSetListVT { CueSetListTypeId };
    juce::ValueTree cueSetVT { CueSetTypeId };
    cueSetVT.setProperty (CueSetIdPropertyId, 1, nullptr);
    cueSetVT.setProperty (CueSetStartPropertyId, 0, nullptr);
    cueSetVT.setProperty (CueSetLoopPropertyId, 0, nullptr);
    cueSetVT.setProperty (CueSetEndPropertyId, 0, nullptr);
    cueSetListVT.addChild (cueSetVT, -1, nullptr);
    data.addChild (cueSetListVT, -1, nullptr);
    setEndCue (0, false);
    setLoopCue (0, false);
    setStartCue (0, false);
    setNumCueSets (1, false);
    setCurCueSet (0, false);
}

void SquidMetaDataProperties::setBits (int bits, bool includeSelfCallback)
{
    setValue (bits, BitsPropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setChoke (int chokeChannel, bool includeSelfCallback)
{
    setValue (chokeChannel, ChokePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setCuePoints (int cueSetIndex, uint32_t start, uint32_t loop, uint32_t end)
{
    const auto numCueSets { getNumCueSets () };
    jassert (cueSetIndex <= numCueSets && cueSetIndex < 64);
    if (cueSetIndex > numCueSets)
        return;

    auto setCueSetProperties = [this] (juce::ValueTree cueSetVT, uint32_t start, uint32_t loop, uint32_t end)
    {
        cueSetVT.setProperty (CueSetStartPropertyId, static_cast<int> (start), nullptr);
        cueSetVT.setProperty (CueSetLoopPropertyId, static_cast<int> (loop), nullptr);
        cueSetVT.setProperty (CueSetEndPropertyId, static_cast<int> (end), nullptr);
    };

    if (cueSetIndex == numCueSets)
    {
        // add new cue set
        auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
        auto newCueSetVT { juce::ValueTree (CueSetTypeId) };
        newCueSetVT.setProperty (CueSetIdPropertyId, numCueSets + 1, nullptr);
        setCueSetProperties (newCueSetVT, start, loop, end);
        cueSetListVT.addChild (newCueSetVT, -1, nullptr);
        setNumCueSets (numCueSets + 1, true);
    }
    else
    {
        auto requestedCueSetVT { getCueSetVT (cueSetIndex) };
        jassert (requestedCueSetVT.isValid ());
        if (! requestedCueSetVT.isValid ())
            return;
        setCueSetProperties (requestedCueSetVT, start, loop, end);
    }
}

void SquidMetaDataProperties::removeCueSet (int cueSetIndex)
{
    const auto numCueSets { getNumCueSets () };
    const auto curCueSet { getCurCueSet () };
    // done allow delete if only 1 Cue Set
    jassert (numCueSets > 1);
    if (numCueSets < 2)
        return;

    // remove cue set
    auto cueSetToRemoveVT { getCueSetVT (cueSetIndex) };
    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    cueSetListVT.removeChild (cueSetToRemoveVT, nullptr);

    // update the IDs of remaining Cue Sets
    auto curCueSetId { 1 };
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, cueSetIndex, &curCueSetId] (juce::ValueTree cueSetVT)
    {
        cueSetVT.setProperty (CueSetIdPropertyId, curCueSetId, nullptr);
        ++curCueSetId;
        return true;
    });

    // update num cue set
    setNumCueSets (numCueSets - 1, true);
    if (curCueSet >= numCueSets - 1)
        setCurCueSet (numCueSets - 2, true);
    else
        setCurCueSet (cueSetIndex, true);
}

void SquidMetaDataProperties::setCurCueSet (int cueSetIndex, bool includeSelfCallback)
{
    setValue (cueSetIndex, CurCueSetPropertyId, includeSelfCallback);
    setStartCue (getStartCueSet (cueSetIndex), true);
    setEndCue (getEndCueSet (cueSetIndex), true);
    setLoopCue (getLoopCueSet (cueSetIndex), true);
}

void SquidMetaDataProperties::setCvAssignAttenuate (int cvIndex, int parameterIndex, int attenuation, bool includeSelfCallback)
{
    jassert (cvIndex < 8);
    jassert (parameterIndex < 15);
    auto cvAssignsVT { data.getChildWithName (CvAssignsTypeId) };
    jassert (cvAssignsVT.isValid ());
    auto cvInputVT { cvAssignsVT.getChild (cvIndex) };
    jassert (cvInputVT.isValid ());
    auto parameterVT { cvInputVT.getChild (parameterIndex) };
    jassert (parameterVT.isValid ());
    parameterVT.setProperty (CvAssignInputParameterAttenuatePropertyId, attenuation, nullptr);
}

void SquidMetaDataProperties::setCvAssignEnabled (int cvIndex, int parameterIndex, bool isEnabled, bool includeSelfCallback)
{
    jassert (cvIndex < 8);
    jassert (parameterIndex < 15);
    auto cvAssignsVT { data.getChildWithName (CvAssignsTypeId) };
    jassert (cvAssignsVT.isValid ());
    auto cvInputVT { cvAssignsVT.getChild (cvIndex) };
    jassert (cvInputVT.isValid ());
    auto parameterVT { cvInputVT.getChild (parameterIndex) };
    jassert (parameterVT.isValid ());
    parameterVT.setProperty (CvAssignInputParameterEnabledPropertyId, isEnabled, nullptr);
}

void SquidMetaDataProperties::setCvAssignOffset (int cvIndex, int parameterIndex, int offset, bool includeSelfCallback)
{
    jassert (cvIndex < 8);
    jassert (parameterIndex < 15);
    auto cvAssignsVT { data.getChildWithName (CvAssignsTypeId) };
    jassert (cvAssignsVT.isValid ());
    auto cvInputVT { cvAssignsVT.getChild (cvIndex) };
    jassert (cvInputVT.isValid ());
    auto parameterVT { cvInputVT.getChild (parameterIndex) };
    jassert (parameterVT.isValid ());
    parameterVT.setProperty (CvAssignInputParameterOffsetPropertyId, offset, nullptr);
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

void SquidMetaDataProperties::setNumCueSets (int numCues, bool includeSelfCallback)
{
    setValue (numCues, NumCueSetsPropertyId, includeSelfCallback);
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
    setValue (static_cast<int> (startCue), StartCuePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setEndCue (uint32_t endCue, bool includeSelfCallback)
{
    setValue (static_cast<int> (endCue), EndCuePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setLoopCue (uint32_t loopCue, bool includeSelfCallback)
{
    setValue (static_cast<int> (loopCue), LoopCuePropertyId, includeSelfCallback);
}

void SquidMetaDataProperties::setStartCueSet (int cueSetIndex, uint32_t startCue, bool includeSelfCallback)
{
    jassert (cueSetIndex < getNumCueSets ());
    if (cueSetIndex >= getNumCueSets ())
        return;

    auto requestedCueSetVT { getCueSetVT (cueSetIndex) };
    jassert (requestedCueSetVT.isValid ());
    if (! requestedCueSetVT.isValid ())
        return;
    requestedCueSetVT.setProperty (CueSetStartPropertyId, static_cast<int>(startCue), nullptr);
}

void SquidMetaDataProperties::setEndCueSet (int cueSetIndex, uint32_t endCue, bool includeSelfCallback)
{
    jassert (cueSetIndex < getNumCueSets ());
    if (cueSetIndex >= getNumCueSets ())
        return;

    auto requestedCueSetVT { getCueSetVT (cueSetIndex) };
    jassert (requestedCueSetVT.isValid ());
    if (! requestedCueSetVT.isValid ())
        return;
    requestedCueSetVT.setProperty (CueSetEndPropertyId, static_cast<int>(endCue), nullptr);
}

void SquidMetaDataProperties::setLoopCueSet (int cueSetIndex, uint32_t loopCue, bool includeSelfCallback)
{
    jassert (cueSetIndex < getNumCueSets ());
    if (cueSetIndex >= getNumCueSets ())
        return;

    auto requestedCueSetVT { getCueSetVT (cueSetIndex) };
    jassert (requestedCueSetVT.isValid ());
    if (! requestedCueSetVT.isValid ())
        return;

    requestedCueSetVT.setProperty (CueSetLoopPropertyId, static_cast<int>(loopCue), nullptr);
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

int SquidMetaDataProperties::getCvAssignAttenuate (int cvIndex, int parameterIndex)
{
    jassert (cvIndex < 8);
    jassert (parameterIndex < 15);
    auto cvAssignsVT { data.getChildWithName (CvAssignsTypeId) };
    jassert (cvAssignsVT.isValid ());
    auto cvInputVT { cvAssignsVT.getChild (cvIndex) };
    jassert (cvInputVT.isValid ());
    auto parameterVT { cvInputVT.getChild (parameterIndex) };
    jassert (parameterVT.isValid ());
    return parameterVT.getProperty (CvAssignInputParameterAttenuatePropertyId);
}

bool SquidMetaDataProperties::getCvAssignEnabled (int cvIndex, int parameterIndex)
{
    jassert (cvIndex < 8);
    jassert (parameterIndex < 15);
    auto cvAssignsVT { data.getChildWithName (CvAssignsTypeId) };
    jassert (cvAssignsVT.isValid ());
    auto cvInputVT { cvAssignsVT.getChild (cvIndex) };
    jassert (cvInputVT.isValid ());
    auto parameterVT { cvInputVT.getChild (parameterIndex) };
    jassert (parameterVT.isValid ());
    return parameterVT.getProperty (CvAssignInputParameterEnabledPropertyId);
}

int SquidMetaDataProperties::getCvAssignOffset (int cvIndex, int parameterIndex)
{
    jassert (cvIndex < 8);
    jassert (parameterIndex < 15);
    auto cvAssignsVT { data.getChildWithName (CvAssignsTypeId) };
    jassert (cvAssignsVT.isValid ());
    auto cvInputVT { cvAssignsVT.getChild (cvIndex) };
    jassert (cvInputVT.isValid ());
    auto parameterVT { cvInputVT.getChild (parameterIndex) };
    jassert (parameterVT.isValid ());
    return parameterVT.getProperty (CvAssignInputParameterOffsetPropertyId);
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

int SquidMetaDataProperties::getNumCueSets ()
{
    return getValue<int> (NumCueSetsPropertyId);
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
    return static_cast<uint32_t> (getValue<int> (StartCuePropertyId));
}

uint32_t SquidMetaDataProperties::getEndCue ()
{
    return static_cast<uint32_t> (getValue<int> (EndCuePropertyId));
}

uint32_t SquidMetaDataProperties::getLoopCue ()
{
    return static_cast<uint32_t> (getValue<int> (LoopCuePropertyId));
}

uint32_t SquidMetaDataProperties::getStartCueSet (int cueSetIndex)
{
    jassert (cueSetIndex < getNumCueSets ());
    if (cueSetIndex >= getNumCueSets ())
        return 0;

    auto requestedCueSetVT { getCueSetVT (cueSetIndex) };
    jassert (requestedCueSetVT.isValid ());
    if (! requestedCueSetVT.isValid ())
        return 0;
    return static_cast<int> (requestedCueSetVT.getProperty (CueSetStartPropertyId));
}

uint32_t SquidMetaDataProperties::getEndCueSet (int cueSetIndex)
{
    jassert (cueSetIndex < getNumCueSets ());
    if (cueSetIndex >= getNumCueSets ())
        return 0;

    auto requestedCueSetVT { getCueSetVT (cueSetIndex) };
    jassert (requestedCueSetVT.isValid ());
    if (! requestedCueSetVT.isValid ())
        return 0;
    return static_cast<int> (requestedCueSetVT.getProperty (CueSetEndPropertyId));
}

uint32_t SquidMetaDataProperties::getLoopCueSet (int cueSetIndex)
{
    jassert (cueSetIndex < getNumCueSets ());
    if (cueSetIndex >= getNumCueSets ())
        return 0;

    auto requestedCueSetVT { getCueSetVT (cueSetIndex) };
    jassert (requestedCueSetVT.isValid ());
    if (! requestedCueSetVT.isValid ())
        return 0;
    return static_cast<int> (requestedCueSetVT.getProperty (CueSetLoopPropertyId));
}

juce::ValueTree SquidMetaDataProperties::getCvParameterVT (int cvIndex, int parameterIndex)
{
    auto cvAssignsVT { data.getChildWithName (SquidMetaDataProperties::CvAssignsTypeId) };
    jassert (cvAssignsVT.isValid ());
    auto cvInputVT { cvAssignsVT.getChild (cvIndex) };
    jassert (cvInputVT.isValid ());
    jassert (cvInputVT.getType () == SquidMetaDataProperties::CvAssignInputTypeId);
    jassert (static_cast<int>(cvInputVT.getProperty (SquidMetaDataProperties::CvAssignInputIdPropertyId)) == cvIndex + 1);
    auto parameterVT { cvInputVT.getChild (parameterIndex) };
    jassert (parameterVT.isValid ());
    jassert (parameterVT.getType () == SquidMetaDataProperties::CvAssignInputParameterTypeId);
    jassert (static_cast<int> (parameterVT.getProperty (SquidMetaDataProperties::CvAssignInputParameterIdPropertyId)) == parameterIndex + 1);
    return parameterVT;
}

juce::ValueTree SquidMetaDataProperties::getCueSetVT (int cueSetIndex)
{
    jassert (cueSetIndex < getNumCueSets ());
    if (cueSetIndex >= getNumCueSets ())
        return {};

    auto cueSetListVT { data.getChildWithName (CueSetListTypeId) };
    jassert (cueSetListVT.isValid ());
    auto cueListCount { 0 };
    juce::ValueTree reqeustedCueSetVT;
    ValueTreeHelpers::forEachChildOfType (cueSetListVT, CueSetTypeId, [this, cueSetIndex, &cueListCount, &reqeustedCueSetVT] (juce::ValueTree cueSetVT)
    {
        if (cueListCount == cueSetIndex)
        {
            reqeustedCueSetVT = cueSetVT;
            return false;
        }
        ++cueListCount;
        return true;
    });
    jassert (reqeustedCueSetVT.isValid ());
    return reqeustedCueSetVT;
}

void SquidMetaDataProperties::copyFrom (juce::ValueTree sourceVT)
{
    SquidMetaDataProperties sourceMetaDataProperties (sourceVT, SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::no);
    // Copy CV Assigns
    for (auto curCvInputIndex { 0 }; curCvInputIndex < kCvInputsCount + kCvInputsExtra; ++curCvInputIndex)
    {
        for (auto curParameterIndex { 0 }; curParameterIndex < 15; ++curParameterIndex)
        {
            auto srcParameterVT { sourceMetaDataProperties.getCvParameterVT (curCvInputIndex, curParameterIndex) };
            auto dstParameterVT { getCvParameterVT (curCvInputIndex, curParameterIndex) };
            const auto enabled { static_cast<bool> (srcParameterVT.getProperty (SquidMetaDataProperties::CvAssignInputParameterEnabledPropertyId)) };
            const auto offset { static_cast<int> (srcParameterVT.getProperty (SquidMetaDataProperties::CvAssignInputParameterOffsetPropertyId)) };
            const auto attenuation { static_cast<int>(srcParameterVT.getProperty (SquidMetaDataProperties::CvAssignInputParameterAttenuatePropertyId)) };
            dstParameterVT.setProperty (SquidMetaDataProperties::CvAssignInputParameterEnabledPropertyId, enabled, nullptr);
            dstParameterVT.setProperty (SquidMetaDataProperties::CvAssignInputParameterAttenuatePropertyId, attenuation, nullptr);
            dstParameterVT.setProperty (SquidMetaDataProperties::CvAssignInputParameterOffsetPropertyId, offset, nullptr);
        }
    }

    // Clear old Cue Sets
    auto dstCueSetListVT { data.getChildWithName (SquidMetaDataProperties::CueSetListTypeId) };
    jassert (dstCueSetListVT.isValid ());
    dstCueSetListVT.removeAllChildren (nullptr);

    // Copy new Cue Sets
    auto srcCueSetListVT { sourceVT.getChildWithName (SquidMetaDataProperties::CueSetListTypeId) };
    jassert (srcCueSetListVT.isValid ());
    ValueTreeHelpers::forEachChildOfType (srcCueSetListVT, SquidMetaDataProperties::CueSetTypeId, [this, &dstCueSetListVT] (juce::ValueTree cueSetVT)
    {
        dstCueSetListVT.addChild (cueSetVT.createCopy (), -1, nullptr);
        return true;
    });

    // Copy properties
    setAttack (sourceMetaDataProperties.getAttack (), false);
    setBits (sourceMetaDataProperties.getBits (), false);
    setChoke (sourceMetaDataProperties.getChoke (), false);
    setNumCueSets (sourceMetaDataProperties.getNumCueSets (), false);
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

    // Copy reserved data
    setReserved1Data (sourceMetaDataProperties.getReserved1Data ());
    setReserved2Data (sourceMetaDataProperties.getReserved2Data ());
    setReserved3Data (sourceMetaDataProperties.getReserved3Data ());
    setReserved4Data (sourceMetaDataProperties.getReserved4Data ());
    setReserved5Data (sourceMetaDataProperties.getReserved5Data ());
    setReserved6Data (sourceMetaDataProperties.getReserved6Data ());
    setReserved7Data (sourceMetaDataProperties.getReserved7Data ());
    setReserved8Data (sourceMetaDataProperties.getReserved8Data ());
    setReserved9Data (sourceMetaDataProperties.getReserved9Data ());
    setReserved10Data (sourceMetaDataProperties.getReserved10Data ());
    setReserved11Data (sourceMetaDataProperties.getReserved11Data ());
    setReserved12Data (sourceMetaDataProperties.getReserved12Data ());
    setReserved13Data (sourceMetaDataProperties.getReserved13Data ());
}

juce::ValueTree SquidMetaDataProperties::create ()
{
    SquidMetaDataProperties metaDataProperties;
    return metaDataProperties.getValueTree ();
}

void SquidMetaDataProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt.getType () == CvAssignInputParameterTypeId)
    {
        // figure out cvInputIndex and cvParameterIndex
        auto parentCvInputVT { vt.getParent () };
        jassert (parentCvInputVT.isValid ());
        jassert (parentCvInputVT.getType () == CvAssignInputTypeId);
        const auto cvInputIndex { static_cast<int> (parentCvInputVT.getProperty (CvAssignInputIdPropertyId)) - 1 };
        const auto parameterIndex { parentCvInputVT.indexOf (vt) };
        // make appropriate callback
        if (property == CvAssignInputParameterEnabledPropertyId)
        {
            if (onCvAssignEnabledChange != nullptr)
                onCvAssignEnabledChange (cvInputIndex, parameterIndex, getCvAssignEnabled (cvInputIndex, parameterIndex));
        }
        else if (property == CvAssignInputParameterAttenuatePropertyId)
        {
            if (onCvAssignAttenuateChange != nullptr)
                onCvAssignAttenuateChange (cvInputIndex, parameterIndex, getCvAssignAttenuate (cvInputIndex, parameterIndex));
        }
        else if (property == CvAssignInputParameterOffsetPropertyId)
        {
            if (onCvAssignOffsetChange != nullptr)
                onCvAssignOffsetChange (cvInputIndex, parameterIndex, getCvAssignOffset (cvInputIndex, parameterIndex));
        }
    }

    if (vt != data)
    {
        if (property == AttackPropertyId)
        {
            if (onAttackChange != nullptr)
                onAttackChange (getAttack ());
        }
        else if (property == BitsPropertyId)
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
        else if (property == DecayPropertyId)
        {
            if (onDecayChange != nullptr)
                onDecayChange (getDecay ());
        }
        else if (property == EndCuePropertyId)
        {
            if (onEndCueChange != nullptr)
                onEndCueChange (getEndCue ());
        }
        else if (property == ETrigPropertyId)
        {
            if (onETrigChange != nullptr)
                onETrigChange (getETrig ());
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
        else if (property == LevelPropertyId)
        {
            if (onLevelChange != nullptr)
                onLevelChange (getLevel ());
        }
        else if (property == LoopCuePropertyId)
        {
            if (onLoopCueChange != nullptr)
                onLoopCueChange (getLoopCue ());
        }
        else if (property == LoopModePropertyId)
        {
            if (onLoopModeChange != nullptr)
                onLoopModeChange (getLoopMode ());
        }
        else if (property == NumCueSetsPropertyId)
        {
            if (onNumCueSetsChange != nullptr)
                onNumCueSetsChange (getNumCueSets ());
        }
        else if (property == QuantPropertyId)
        {
            if (onQuantChange != nullptr)
                onQuantChange (getQuant ());
        }
        else if (property == RatePropertyId)
        {
            if (onRateChange != nullptr)
                onRateChange (getRate ());
        }
        else if (property == ReversePropertyId)
        {
            if (onReverseChange != nullptr)
                onReverseChange (getReverse ());
        }
        else if (property == SpeedPropertyId)
        {
            if (onSpeedChange != nullptr)
                onSpeedChange (getSpeed ());
        }
        else if (property == StartCuePropertyId)
        {
            if (onStartCueChange != nullptr)
                onStartCueChange (getStartCue ());
        }
        else if (property == StepsPropertyId)
        {
            if (onStepsChange != nullptr)
                onStepsChange (getSteps ());
        }
        else if (property == XfadePropertyId)
        {
            if (onXfadeChange != nullptr)
                onXfadeChange (getXfade ());
        }
    }
}
