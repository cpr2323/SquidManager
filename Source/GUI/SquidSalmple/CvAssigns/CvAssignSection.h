#pragma once
#include <JuceHeader.h>
#include "CvAssignParameter.h"

class CvAssignSection : public juce::Component
{
public:
    CvAssignSection ();
    ~CvAssignSection ();
    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT, int theCvIndex);
    void setEnableState (int cvParameterId, bool enabled);

private:
    juce::Label cvAssignLabel;
    //std::vector<CvAssignParameter> cvAssignParameterList;

    void resized () override;
};