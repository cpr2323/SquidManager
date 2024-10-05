#include "CvAssignSection.h"
#include "../../../SquidSalmple/Metadata/SquidSalmpleDefs.h"

struct ParameterEntry
{
    int parameterIndex { 0 };
    juce::String parameterName;
};
const auto kParameterList { std::vector<ParameterEntry>
{
    {CvParameterIndex::Level, "LEVEL"},
    {CvParameterIndex::Attack, "ATTACK"},
    {CvParameterIndex::Decay, "DECAY"},
    {CvParameterIndex::StartCue, "STARTQ"},
    {CvParameterIndex::LoopCue, "LOOPQ"},
    {CvParameterIndex::EndCue, "ENDQ"},
    {CvParameterIndex::FiltFreq, "FREQ"},
    {CvParameterIndex::FiltRes, "RES"},
    {CvParameterIndex::Reverse, "REVERSE"},
    {CvParameterIndex::Speed, "SPEED"},
    {CvParameterIndex::Bits, "BITS"},
    {CvParameterIndex::Rate, "RATE"},
    {CvParameterIndex::LoopMode, "LPMODE"},
    {CvParameterIndex::ETrig, "ETRIG"},
    {CvParameterIndex::CueSet, "CUE SET"},
} };

CvAssignSection::CvAssignSection ()
{
    for (auto curParameterIndex { 0 }; curParameterIndex < cvAssignParameterList.size (); ++curParameterIndex)
    {
        auto& curParameter { cvAssignParameterList [curParameterIndex] };
        curParameter.setParameterLabel (kParameterList [curParameterIndex].parameterName);
        addAndMakeVisible (curParameter);
    }
}

void CvAssignSection::init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT, int theCvIndex)
{
    for (auto curParameterIndex { 0 }; curParameterIndex < cvAssignParameterList.size (); ++curParameterIndex)
    {
        auto& curParameter { cvAssignParameterList [curParameterIndex] };
        curParameter.init (rootPropertiesVT, channelPropertiesVT, theCvIndex, kParameterList [curParameterIndex].parameterIndex);
    }
}

void CvAssignSection::setEnableState (int cvParameterIndex, bool enabled)
{
    for (auto& cvAssignParameter : cvAssignParameterList)
    {
        if (cvAssignParameter.getParameterIndex () == cvParameterIndex)
        {
            cvAssignParameter.setEnabled (enabled);
            break;
        }
    }
}

void CvAssignSection::resized ()
{
    const auto assignParameterWidth { static_cast<int> (getWidth () / cvAssignParameterList.size ()) };
    for (auto curCvAssignParameter { 0 }; curCvAssignParameter < cvAssignParameterList.size (); ++curCvAssignParameter)
        cvAssignParameterList [curCvAssignParameter].setBounds (assignParameterWidth * curCvAssignParameter, 0, assignParameterWidth, getHeight ());
}
