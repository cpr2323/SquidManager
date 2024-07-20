#include "CvAssignSection.h"
#include "../../../SquidSalmple/Metadata/SquidSalmpleDefs.h"

struct ParameterEntry
{
    int parameterIndex { 0 };
    juce::String parameterName;
};
const auto kParameterList { std::vector<ParameterEntry> 
{
    {CvParameterIndex::Bits, "BITS"},
    {CvParameterIndex::Rate, "RATE"},
    {CvParameterIndex::Speed, "SPEED"},
    {CvParameterIndex::Level, "LEVEL"},
    {CvParameterIndex::Attack, "ATTACK"},
    {CvParameterIndex::Decay, "DECAY"},
    {CvParameterIndex::FiltFreq, "FREQ"},
    {CvParameterIndex::FiltRes, "RES"},
    {CvParameterIndex::StartCue, "STARTQ"},
    {CvParameterIndex::EndCue, "ENDQ"},
    {CvParameterIndex::LoopCue, "LOOPQ"},
    {CvParameterIndex::LoopMode, "LPMODE"},
    {CvParameterIndex::Reverse, "REVERSE"},
    {CvParameterIndex::CueSet, "CUE SET"},
    {CvParameterIndex::ETrig, "ETRIG"},
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

void CvAssignSection::init (juce::ValueTree rootPropertiesVT, int theCvIndex)
{
    for (auto curParameterIndex { 0 }; curParameterIndex < cvAssignParameterList.size (); ++curParameterIndex)
    {
        auto& curParameter { cvAssignParameterList [curParameterIndex] };
        curParameter.init (rootPropertiesVT, theCvIndex, kParameterList [curParameterIndex].parameterIndex);
    }
}

void CvAssignSection::resized ()
{
    const auto assignParameterWidth { static_cast<int>(getWidth () / cvAssignParameterList.size ()) };
    for (auto curCvAssignParameter { 0 }; curCvAssignParameter < cvAssignParameterList.size (); ++curCvAssignParameter)
        cvAssignParameterList [curCvAssignParameter].setBounds (assignParameterWidth * curCvAssignParameter, 0, assignParameterWidth, getHeight ());
}
