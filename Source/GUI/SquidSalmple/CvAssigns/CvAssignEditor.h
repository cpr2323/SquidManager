#pragma once
#include <JuceHeader.h>
#include "CvAssignSection.h"

class CvAssignEditor : public juce::Component
{
public:
    CvAssignEditor ();
    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT);
    void setEnableState (int cvParameterIndex, bool enabled);

private:
    juce::Label curCvAssignIndexLabel;
    juce::TextButton upButton;
    juce::TextButton downButton;
    std::array<CvAssignSection, 7> cvAssignSectionList;
    int curCvAssignIndex { 0 };

    void paint (juce::Graphics& g) override;
    void resized () override;
    void selectCvAssigns (int cvSelectButtonIndex);
};
