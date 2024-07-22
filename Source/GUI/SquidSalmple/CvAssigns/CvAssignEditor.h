#pragma once
#include <JuceHeader.h>
#include "CvAssignSection.h"

class CvAssignEditor : public juce::Component
{
public:
    CvAssignEditor ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::Label curCvAssignIndexLabel;
    juce::TextButton upButton;
    juce::TextButton downButton;
    std::array<CvAssignSection, 8> cvAssignSectionList;
    int curCvAssignIndex { 0 };

    void paint (juce::Graphics& g) override;
    void resized () override;
    void selectCvAssigns (int cvSelectButtonIndex);
};
