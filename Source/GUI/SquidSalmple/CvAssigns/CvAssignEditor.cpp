#include "CvAssignEditor.h"

CvAssignEditor::CvAssignEditor ()
{
    upButton.setButtonText("/\\");
    upButton.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    upButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    upButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::white);
    upButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    upButton.onClick = [this] ()
    {
        if (curCvAssignIndex == 0)
            return;
        selectCvAssigns (curCvAssignIndex - 1);
    };
    addAndMakeVisible (upButton);

    downButton.setButtonText ("\\/");
    downButton.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    downButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    downButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::white);
    downButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    downButton.onClick = [this] ()
    {
        if (curCvAssignIndex == 7)
            return;
        selectCvAssigns (curCvAssignIndex + 1);
    };
    addAndMakeVisible (downButton);

    curCvAssignIndexLabel.setBorderSize ({ 0,0,0,0 });
    curCvAssignIndexLabel.setFont (16);
    addAndMakeVisible (curCvAssignIndexLabel);

    for (auto& cvAssignSection : cvAssignSectionList)
        addChildComponent (cvAssignSection);

    selectCvAssigns (curCvAssignIndex);
}

void CvAssignEditor::init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT)
{
    for (auto cvIndex { 0 }; cvIndex < cvAssignSectionList.size (); ++cvIndex)
    {
        auto& cvAssignSection { cvAssignSectionList [cvIndex] };
        cvAssignSection.init (rootPropertiesVT, channelPropertiesVT, cvIndex);
    }
}

void CvAssignEditor::setEnableState (int cvParameterIndex, bool enabled)
{
    for (auto& cvAssignSection : cvAssignSectionList)
        cvAssignSection.setEnableState (cvParameterIndex, enabled);
}

void CvAssignEditor::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::white);
    g.drawLine (0, 0, 15, 0);
    g.drawLine (0.f, 0.f, 0.f, static_cast<float> (getHeight ()));
    g.drawLine (0.f, static_cast<float> (getHeight ()), 15.f, static_cast<float> (getHeight ()));
}

void CvAssignEditor::resized ()
{
    auto localBounds { getLocalBounds () };
    auto cvAssignSelectSection {localBounds.removeFromLeft (15)};
    cvAssignSelectSection.removeFromLeft (1);
    cvAssignSelectSection.removeFromTop (1);
    upButton.setBounds (cvAssignSelectSection.removeFromTop (14));
    downButton.setBounds (cvAssignSelectSection.removeFromBottom (14));
    curCvAssignIndexLabel.setBounds (cvAssignSelectSection.withX (3));

    for (auto curCvAssignSection { 0 }; curCvAssignSection < cvAssignSectionList.size (); ++curCvAssignSection)
        cvAssignSectionList [curCvAssignSection].setBounds (localBounds);
}

void CvAssignEditor::selectCvAssigns (int newCvAssignIndex)
{
    curCvAssignIndex = newCvAssignIndex;
    curCvAssignIndexLabel.setText (juce::String (curCvAssignIndex + 1), juce::NotificationType::dontSendNotification);
    for (auto cvAssignSectionIndex { 0 }; cvAssignSectionIndex < cvAssignSectionList.size (); ++cvAssignSectionIndex)
        cvAssignSectionList [cvAssignSectionIndex].setVisible (cvAssignSectionIndex == curCvAssignIndex);
}
