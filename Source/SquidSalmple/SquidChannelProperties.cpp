#include "SquidChannelProperties.h"
#include "Metadata/SquidSalmpleDefs.h"
#include "../Utility/ValueTreeHelpers.h"

static const auto kScaleMax { 65535. };
static const auto kScaleStep { kScaleMax / 100 };

void SquidChannelProperties::initValueTree ()
{
    setAttack (0, false);
    setBits (16, false);
    setChannelFlags (0, false);
    setChannelIndex (0, false);
    setChoke (0, false);
    setDecay (0, false);
    setETrig (0, false);
    setFileName ("", false);
    setFilterFrequency (0, false);
    setFilterResonance (0, false);
    setFilterType (0, false);
    setLoopMode (0, false);
    setLevel (30 * kScaleStep, false);
    setNumCueSets (0, false);
    setQuant (0, false);
    setRate (0, false);
    setReverse (0, false);
    setSpeed ((50 * kScaleStep), false);
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
    juce::ValueTree cvAssignsVT { SquidChannelProperties::CvAssignsTypeId };
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

void SquidChannelProperties::setBits (int bits, bool includeSelfCallback)
{
    setValue (bits, BitsPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setChannelFlags (uint16_t channelFlags, bool includeSelfCallback)
{
    setValue (static_cast<int> (channelFlags), ChannelFlagsPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setChannelIndex (uint8_t channelIndex, bool includeSelfCallback)
{
    setValue (static_cast<int>(channelIndex), ChannelIndexPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setChannelSource (uint8_t channelIndex, bool includeSelfCallback)
{
    setValue (static_cast<int>(channelIndex), ChannelSourcePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setChoke (int chokeChannel, bool includeSelfCallback)
{
    setValue (chokeChannel, ChokePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setCuePoints (int cueSetIndex, uint32_t start, uint32_t loop, uint32_t end)
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

void SquidChannelProperties::removeCueSet (int cueSetIndex)
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

void SquidChannelProperties::setCurCueSet (int cueSetIndex, bool includeSelfCallback)
{
    setValue (cueSetIndex, CurCueSetPropertyId, includeSelfCallback);
    setStartCue (getStartCueSet (cueSetIndex), true);
    setEndCue (getEndCueSet (cueSetIndex), true);
    setLoopCue (getLoopCueSet (cueSetIndex), true);
}

void SquidChannelProperties::setCvAssignAttenuate (int cvIndex, int parameterIndex, int attenuation, bool includeSelfCallback)
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

void SquidChannelProperties::setCvAssignEnabled (int cvIndex, int parameterIndex, bool isEnabled, bool includeSelfCallback)
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

void SquidChannelProperties::setCvAssignOffset (int cvIndex, int parameterIndex, int offset, bool includeSelfCallback)
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

void SquidChannelProperties::setRate (int rate, bool includeSelfCallback)
{
    setValue (rate, RatePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setSpeed (int speed, bool includeSelfCallback)
{
    setValue (speed, SpeedPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setFilterFrequency (int filterFrequency, bool includeSelfCallback)
{
    setValue (filterFrequency, FilterFrequencyPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setFilterResonance (int filterResonance, bool includeSelfCallback)
{
    setValue (filterResonance, FilterResonancePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setFilterType (int filter, bool includeSelfCallback)
{
    setValue (filter, FilterTypePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setQuant (int quant, bool includeSelfCallback)
{
    setValue (quant, QuantPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setLoopMode (int loopMode, bool includeSelfCallback)
{
    setValue (loopMode, LoopModePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setNumCueSets (int numCues, bool includeSelfCallback)
{
    setValue (numCues, NumCueSetsPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setXfade (int xfade, bool includeSelfCallback)
{
    setValue (xfade, XfadePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setReserved1Data (juce::String reservedData)
{
    setValue (reservedData, Reserved1DataPropertyId, false);
}
void SquidChannelProperties::setReserved2Data (juce::String reservedData)
{
    setValue (reservedData, Reserved2DataPropertyId, false);
}
void SquidChannelProperties::setReserved3Data (juce::String reservedData)
{
    setValue (reservedData, Reserved3DataPropertyId, false);
}
void SquidChannelProperties::setReserved4Data (juce::String reservedData)
{
    setValue (reservedData, Reserved4DataPropertyId, false);
}
void SquidChannelProperties::setReserved5Data (juce::String reservedData)
{
    setValue (reservedData, Reserved5DataPropertyId, false);
}
void SquidChannelProperties::setReserved6Data (juce::String reservedData)
{
    setValue (reservedData, Reserved6DataPropertyId, false);
}
void SquidChannelProperties::setReserved7Data (juce::String reservedData)
{
    setValue (reservedData, Reserved7DataPropertyId, false);
}
void SquidChannelProperties::setReserved8Data (juce::String reservedData)
{
    setValue (reservedData, Reserved8DataPropertyId, false);
}
void SquidChannelProperties::setReserved9Data (juce::String reservedData)
{
    setValue (reservedData, Reserved9DataPropertyId, false);
}
void SquidChannelProperties::setReserved10Data (juce::String reservedData)
{
    setValue (reservedData, Reserved10DataPropertyId, false);
}
void SquidChannelProperties::setReserved11Data (juce::String reservedData)
{
    setValue (reservedData, Reserved11DataPropertyId, false);
}
void SquidChannelProperties::setReserved12Data (juce::String reservedData)
{
    setValue (reservedData, Reserved12DataPropertyId, false);
}
void SquidChannelProperties::setReserved13Data (juce::String reservedData)
{
    setValue (reservedData, Reserved13DataPropertyId, false);
}

void SquidChannelProperties::setReverse (int reverse, bool includeSelfCallback)
{
    setValue (reverse, ReversePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setLevel (int level, bool includeSelfCallback)
{
    setValue (level, LevelPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setAttack (int attack, bool includeSelfCallback)
{
    setValue (attack, AttackPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setDecay (int decay, bool includeSelfCallback)
{
    setValue (decay, DecayPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setETrig (int eTrig, bool includeSelfCallback)
{
    setValue (eTrig, ETrigPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setSteps (int steps, bool includeSelfCallback)
{
    setValue (steps, StepsPropertyId, includeSelfCallback);
}

void SquidChannelProperties::setStartCue (uint32_t startCue, bool includeSelfCallback)
{
    setValue (static_cast<int> (startCue), StartCuePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setEndCue (uint32_t endCue, bool includeSelfCallback)
{
    setValue (static_cast<int> (endCue), EndCuePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setLoopCue (uint32_t loopCue, bool includeSelfCallback)
{
    setValue (static_cast<int> (loopCue), LoopCuePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setStartCueSet (int cueSetIndex, uint32_t startCue, bool includeSelfCallback)
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

void SquidChannelProperties::setEndCueSet (int cueSetIndex, uint32_t endCue, bool includeSelfCallback)
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

void SquidChannelProperties::setFileName (juce::String fileName, bool includeSelfCallback)
{
    setValue (fileName, FileNamePropertyId, includeSelfCallback);
}

void SquidChannelProperties::setLoopCueSet (int cueSetIndex, uint32_t loopCue, bool includeSelfCallback)
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

void SquidChannelProperties::triggerLoadBegin (bool includeSelfCallback)
{
    toggleValue (LoadBeginPropertyId, includeSelfCallback);
}

void SquidChannelProperties::triggerLoadComplete (bool includeSelfCallback)
{
    toggleValue (LoadCompletePropertyId, includeSelfCallback);
}

int SquidChannelProperties::getBits ()
{
    return getValue<int> (BitsPropertyId);
}

uint16_t SquidChannelProperties::getChannelFlags ()
{
    return static_cast<uint16_t>(getValue<int> (ChannelFlagsPropertyId));
}

uint8_t SquidChannelProperties::getChannelIndex ()
{
    return static_cast<uint8_t> (getValue<int> (ChannelIndexPropertyId));
}

uint8_t SquidChannelProperties::getChannelSource ()
{
    return static_cast<uint8_t> (getValue<int> (ChannelSourcePropertyId));
}

int SquidChannelProperties::getChoke ()
{
    return getValue<int> (ChokePropertyId);
}

int SquidChannelProperties::getCurCueSet ()
{
    return getValue<int> (CurCueSetPropertyId);
}

int SquidChannelProperties::getCvAssignAttenuate (int cvIndex, int parameterIndex)
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

bool SquidChannelProperties::getCvAssignEnabled (int cvIndex, int parameterIndex)
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

int SquidChannelProperties::getCvAssignOffset (int cvIndex, int parameterIndex)
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

int SquidChannelProperties::getRate ()
{
    return getValue<int> (RatePropertyId);
}

int SquidChannelProperties::getSpeed ()
{
    return getValue<int> (SpeedPropertyId);
}

int SquidChannelProperties::getFilterFrequency ()
{
    return getValue<int> (FilterFrequencyPropertyId);
}

int SquidChannelProperties::getFilterResonance ()
{
    return getValue<int> (FilterResonancePropertyId);
}

int SquidChannelProperties::getFilterType ()
{
    return getValue<int> (FilterTypePropertyId);
}

int SquidChannelProperties::getQuant ()
{
    return getValue<int> (QuantPropertyId);
}

int SquidChannelProperties::getLoopMode ()
{
    return getValue<int> (LoopModePropertyId);
}

int SquidChannelProperties::getNumCueSets ()
{
    return getValue<int> (NumCueSetsPropertyId);
}

int SquidChannelProperties::getXfade ()
{
    return getValue<int> (XfadePropertyId);
}

juce::String SquidChannelProperties::getReserved1Data ()
{
    return getValue<juce::String> (Reserved1DataPropertyId);
}
juce::String SquidChannelProperties::getReserved2Data ()
{
    return getValue<juce::String> (Reserved2DataPropertyId);
}
juce::String SquidChannelProperties::getReserved3Data ()
{
    return getValue<juce::String> (Reserved3DataPropertyId);
}
juce::String SquidChannelProperties::getReserved4Data ()
{
    return getValue<juce::String> (Reserved4DataPropertyId);
}
juce::String SquidChannelProperties::getReserved5Data ()
{
    return getValue<juce::String> (Reserved5DataPropertyId);
}
juce::String SquidChannelProperties::getReserved6Data ()
{
    return getValue<juce::String> (Reserved6DataPropertyId);
}
juce::String SquidChannelProperties::getReserved7Data ()
{
    return getValue<juce::String> (Reserved7DataPropertyId);
}
juce::String SquidChannelProperties::getReserved8Data ()
{
    return getValue<juce::String> (Reserved8DataPropertyId);
}
juce::String SquidChannelProperties::getReserved9Data ()
{
    return getValue<juce::String> (Reserved9DataPropertyId);
}
juce::String SquidChannelProperties::getReserved10Data ()
{
    return getValue<juce::String> (Reserved10DataPropertyId);
}
juce::String SquidChannelProperties::getReserved11Data ()
{
    return getValue<juce::String> (Reserved11DataPropertyId);
}
juce::String SquidChannelProperties::getReserved12Data ()
{
    return getValue<juce::String> (Reserved12DataPropertyId);
}
juce::String SquidChannelProperties::getReserved13Data ()
{
    return getValue<juce::String> (Reserved13DataPropertyId);
}

int SquidChannelProperties::getReverse ()
{
    return getValue<int> (ReversePropertyId);
}

int SquidChannelProperties::getLevel ()
{
    return getValue<int> (LevelPropertyId);
}

int SquidChannelProperties::getAttack ()
{
    return getValue<int> (AttackPropertyId);
}

int SquidChannelProperties::getDecay ()
{
    return getValue<int> (DecayPropertyId);
}

int SquidChannelProperties::getETrig ()
{
    return getValue<int> (ETrigPropertyId);
}

int SquidChannelProperties::getSteps ()
{
    return getValue<int> (StepsPropertyId);
}

uint32_t SquidChannelProperties::getStartCue ()
{
    return static_cast<uint32_t> (getValue<int> (StartCuePropertyId));
}

uint32_t SquidChannelProperties::getEndCue ()
{
    return static_cast<uint32_t> (getValue<int> (EndCuePropertyId));
}

uint32_t SquidChannelProperties::getLoopCue ()
{
    return static_cast<uint32_t> (getValue<int> (LoopCuePropertyId));
}

uint32_t SquidChannelProperties::getStartCueSet (int cueSetIndex)
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

uint32_t SquidChannelProperties::getEndCueSet (int cueSetIndex)
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

juce::String SquidChannelProperties::getFileName ()
{
    return getValue<juce::String> (FileNamePropertyId);
}

uint32_t SquidChannelProperties::getLoopCueSet (int cueSetIndex)
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

juce::ValueTree SquidChannelProperties::getCvParameterVT (int cvIndex, int parameterIndex)
{
    auto cvAssignsVT { data.getChildWithName (SquidChannelProperties::CvAssignsTypeId) };
    jassert (cvAssignsVT.isValid ());
    auto cvInputVT { cvAssignsVT.getChild (cvIndex) };
    jassert (cvInputVT.isValid ());
    jassert (cvInputVT.getType () == SquidChannelProperties::CvAssignInputTypeId);
    jassert (static_cast<int>(cvInputVT.getProperty (SquidChannelProperties::CvAssignInputIdPropertyId)) == cvIndex + 1);
    auto parameterVT { cvInputVT.getChild (parameterIndex) };
    jassert (parameterVT.isValid ());
    jassert (parameterVT.getType () == SquidChannelProperties::CvAssignInputParameterTypeId);
    jassert (static_cast<int> (parameterVT.getProperty (SquidChannelProperties::CvAssignInputParameterIdPropertyId)) == parameterIndex + 1);
    return parameterVT;
}

juce::ValueTree SquidChannelProperties::getCueSetVT (int cueSetIndex)
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

void SquidChannelProperties::copyFrom (juce::ValueTree sourceVT)
{
    SquidChannelProperties sourceMetaDataProperties (sourceVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
    // Copy CV Assigns
    for (auto curCvInputIndex { 0 }; curCvInputIndex < kCvInputsCount + kCvInputsExtra; ++curCvInputIndex)
    {
        for (auto curParameterIndex { 0 }; curParameterIndex < 15; ++curParameterIndex)
        {
            auto srcParameterVT { sourceMetaDataProperties.getCvParameterVT (curCvInputIndex, curParameterIndex) };
            auto dstParameterVT { getCvParameterVT (curCvInputIndex, curParameterIndex) };
            const auto enabled { static_cast<bool> (srcParameterVT.getProperty (SquidChannelProperties::CvAssignInputParameterEnabledPropertyId)) };
            const auto offset { static_cast<int> (srcParameterVT.getProperty (SquidChannelProperties::CvAssignInputParameterOffsetPropertyId)) };
            const auto attenuation { static_cast<int>(srcParameterVT.getProperty (SquidChannelProperties::CvAssignInputParameterAttenuatePropertyId)) };
            dstParameterVT.setProperty (SquidChannelProperties::CvAssignInputParameterEnabledPropertyId, enabled, nullptr);
            dstParameterVT.setProperty (SquidChannelProperties::CvAssignInputParameterAttenuatePropertyId, attenuation, nullptr);
            dstParameterVT.setProperty (SquidChannelProperties::CvAssignInputParameterOffsetPropertyId, offset, nullptr);
        }
    }

    // Clear old Cue Sets
    auto dstCueSetListVT { data.getChildWithName (SquidChannelProperties::CueSetListTypeId) };
    jassert (dstCueSetListVT.isValid ());
    dstCueSetListVT.removeAllChildren (nullptr);

    // Copy new Cue Sets
    auto srcCueSetListVT { sourceVT.getChildWithName (SquidChannelProperties::CueSetListTypeId) };
    jassert (srcCueSetListVT.isValid ());
    ValueTreeHelpers::forEachChildOfType (srcCueSetListVT, SquidChannelProperties::CueSetTypeId, [this, &dstCueSetListVT] (juce::ValueTree cueSetVT)
    {
        dstCueSetListVT.addChild (cueSetVT.createCopy (), -1, nullptr);
        return true;
    });

    // Copy properties
    setAttack (sourceMetaDataProperties.getAttack (), false);
    setBits (sourceMetaDataProperties.getBits (), false);
    setChannelFlags (sourceMetaDataProperties.getChannelFlags (), false);
    setChoke (sourceMetaDataProperties.getChoke (), false);
    setNumCueSets (sourceMetaDataProperties.getNumCueSets (), false);
    setCurCueSet (sourceMetaDataProperties.getCurCueSet (), false);
    setDecay (sourceMetaDataProperties.getDecay (), false);
    setFileName (sourceMetaDataProperties.getFileName (), false);
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

juce::ValueTree SquidChannelProperties::create (uint8_t channelIndex)
{
    SquidChannelProperties squidChannelProperties;
    squidChannelProperties.setChannelIndex (channelIndex, false);
    return squidChannelProperties.getValueTree ();
}

void SquidChannelProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
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

    if (vt == data)
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
        else if (property == ChannelFlagsPropertyId)
        {
            if (onChannelFlagsChange != nullptr)
                onChannelFlagsChange(getChannelFlags ());
        }
        else if (property == ChannelIndexPropertyId)
        {
            if (onChannelIndexChange != nullptr)
                onChannelIndexChange (getChannelIndex ());
        }
        else if (property == ChannelSourcePropertyId)
        {
            if (onChannelSourceChange != nullptr)
                onChannelSourceChange (getChannelSource ());
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
        else if (property == FileNamePropertyId)
        {
            if (onFileNameChange != nullptr)
                onFileNameChange (getFileName ());
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
        else if (property == LoadBeginPropertyId)
        {
            if (onLoadBegin != nullptr)
                onLoadBegin ();
        }
        else if (property == LoadCompletePropertyId)
        {
            if (onLoadComplete != nullptr)
                onLoadComplete ();
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
