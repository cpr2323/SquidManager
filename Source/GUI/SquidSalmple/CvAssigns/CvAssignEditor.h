#pragma once
#include <JuceHeader.h>
#include "CvAssignSection.h"

class CvAssignEditor : public juce::Component
{
public:
    CvAssignEditor ();
    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT);
    void setEnableState (int cvParameterId, bool enabled);

private:
    juce::Label curCvAssignIndexLabel;
    juce::ArrowButton upButton { juce::String ("up"), 0.75, juce::Colours::white };
    juce::ArrowButton downButton { juce::String ("down"), 0.25f, juce::Colours::white };
    std::array<CvAssignSection, 7> cvAssignSectionList;
    int curCvAssignIndex { 0 };

    void paint (juce::Graphics& g) override;
    void resized () override;
    void selectCvAssigns (int cvSelectButtonIndex);
};
