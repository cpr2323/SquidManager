#pragma once
#include <JuceHeader.h>
#include "CvAssignParameter.h"

class CvAssignSection : public juce::Component
{
public:
    CvAssignSection ();
    void init (juce::ValueTree rootPropertiesVT, int theCvIndex);

private:
    juce::Label cvAssignLabel;
    std::array<CvAssignParameter, 15>  cvAssignParameterList;

    void resized () override;
};
