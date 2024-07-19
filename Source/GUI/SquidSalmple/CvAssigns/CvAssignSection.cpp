#include "CvAssignSection.h"

CvAssignSection::CvAssignSection ()
{
    const auto kParameterNameList { std::vector<juce::String> { "BITS", "RATE", "LEVEL", "DECAY", "SPEED", "MODE", "REVERSE", "START",
                                                                "END", "LOOP", "ATTACK", "CUE SET", "ETRIG", "FREQ", "RES" } };
    for (auto curParameterIndex { 0 }; curParameterIndex < cvAssignParameterList.size (); ++curParameterIndex)
    {
        auto& curParameter { cvAssignParameterList [curParameterIndex] };
        curParameter.setParameterLabel (kParameterNameList [curParameterIndex]);
        addAndMakeVisible (curParameter);
    }
}

void CvAssignSection::init (juce::ValueTree rootPropertiesVT, int theCvIndex)
{
    for (auto curParameterIndex { 0 }; curParameterIndex < cvAssignParameterList.size (); ++curParameterIndex)
    {
        auto& curParameter { cvAssignParameterList [curParameterIndex] };
        curParameter.init (rootPropertiesVT, theCvIndex, curParameterIndex);
    }
}

void CvAssignSection::resized ()
{
    const auto assignParameterWidth { static_cast<int>(getWidth () / cvAssignParameterList.size ()) };
    for (auto curCvAssignParameter { 0 }; curCvAssignParameter < cvAssignParameterList.size (); ++curCvAssignParameter)
        cvAssignParameterList [curCvAssignParameter].setBounds (assignParameterWidth * curCvAssignParameter, 0, assignParameterWidth, getHeight ());
}
