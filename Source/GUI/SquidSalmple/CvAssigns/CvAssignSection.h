#pragma once
#include <JuceHeader.h>
#include "CvAssignParameter.h"

class CvAssignSection : public juce::Component
{
public:
    CvAssignSection ();
    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT, int theCvIndex);
    void setEnableState (int cvParameterIndex, bool enabled);

private:
    juce::Label cvAssignLabel;
    // TODO - the following array might be configured differently because hardcoding the size means we need to change it 
    std::array<CvAssignParameter, 16>  cvAssignParameterList;

    void resized () override;
};
