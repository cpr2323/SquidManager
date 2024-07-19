#pragma once
#include <JuceHeader.h>
#include "CvAssignSection.h"

class CvAssignEditor : public juce::Component
{
public:
    CvAssignEditor ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    std::array<juce::TextButton, 8> cvSelectButtons;
    std::array<CvAssignSection, 8> cvAssignSectionList;

    void paint (juce::Graphics& g);
    void resized () override;
    void selectCvAssigns (int cvSelectButtonIndex);
};