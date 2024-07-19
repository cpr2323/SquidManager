#include "CvAssignEditor.h"

CvAssignEditor::CvAssignEditor ()
{
    for (auto cvSelectButtonIndex { 0 };cvSelectButtonIndex < cvSelectButtons.size(); ++cvSelectButtonIndex)
    {
        auto& cvSelectButton { cvSelectButtons [cvSelectButtonIndex] };
        cvSelectButton.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgrey);
        cvSelectButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
        cvSelectButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::white);
        cvSelectButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
        cvSelectButton.setButtonText (juce::String (cvSelectButtonIndex + 1));
        cvSelectButton.onClick = [this, cvSelectButtonIndex] () { selectCvAssigns (cvSelectButtonIndex); };
        cvSelectButton.setRadioGroupId (1234);
        addAndMakeVisible (cvSelectButton);
    }
    for (auto& cvAssignSection : cvAssignSectionList)
        addChildComponent (cvAssignSection);

    selectCvAssigns (0);
}

void CvAssignEditor::init (juce::ValueTree rootPropertiesVT)
{
    for (auto cvIndex { 0 }; cvIndex < cvAssignSectionList.size (); ++cvIndex)
    {
        auto& cvAssignSection { cvAssignSectionList [cvIndex] };
        cvAssignSection.init (rootPropertiesVT, cvIndex);
    }
}

void CvAssignEditor::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::white);
    g.drawRect (getLocalBounds ());
}

void CvAssignEditor::resized ()
{
    for (auto cvSelectButtonIndex { 0 }; cvSelectButtonIndex < cvSelectButtons.size (); ++cvSelectButtonIndex)
    {
        cvSelectButtons [cvSelectButtonIndex].setBounds (2, 2 + (cvSelectButtonIndex * 20), 20, 20);
    }
    for (auto curCvAssignSection { 0 }; curCvAssignSection < cvAssignSectionList.size (); ++curCvAssignSection)
        cvAssignSectionList [curCvAssignSection].setBounds (cvSelectButtons [0].getRight () + 2, 1, getWidth () - 2, 100);
}

void CvAssignEditor::selectCvAssigns (int cvSelectButtonIndex)
{
    cvSelectButtons [cvSelectButtonIndex].setToggleState (true, juce::NotificationType::dontSendNotification);
    for (auto cvAssignSectionIndex { 0 }; cvAssignSectionIndex < cvAssignSectionList.size (); ++cvAssignSectionIndex)
        cvAssignSectionList [cvAssignSectionIndex].setVisible (cvAssignSectionIndex == cvSelectButtonIndex);
}
