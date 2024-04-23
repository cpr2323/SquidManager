#include "CvAssignEditor.h"

CvAssignParameter::CvAssignParameter ()
{
    parameterLabel.setText ("---", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (parameterLabel);
    assignEnable.setButtonText ("ENABLE");
    addAndMakeVisible (assignEnable);
    cvAttenuateEditor.getMinValueCallback = [this] () { return 0; };
    cvAttenuateEditor.getMaxValueCallback = [this] () { return 99; };
    cvAttenuateEditor.setText ("50");
    addAndMakeVisible (cvAttenuateEditor);
    cvOffsetEditor.getMinValueCallback = [this] () { return 0; };
    cvOffsetEditor.getMaxValueCallback = [this] () { return 99; };
    cvOffsetEditor.setText ("0");
    addAndMakeVisible (cvOffsetEditor);
}

void CvAssignParameter::setParameterLabel (juce::String parameterText)
{
    parameterLabel.setText (parameterText, juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::red);
    g.drawRect (getLocalBounds ());
}

void CvAssignParameter::resized ()
{
    auto localBounds { getLocalBounds() };
    const auto lineHeight { localBounds.getHeight () / 4 };
    parameterLabel.setBounds (localBounds.removeFromTop (lineHeight));
    assignEnable.setBounds (localBounds.removeFromTop (lineHeight));
    cvAttenuateEditor.setBounds (localBounds.removeFromTop (lineHeight));
    cvOffsetEditor.setBounds (localBounds.removeFromTop (lineHeight));
}

CvAssignSection::CvAssignSection ()
{
    for (auto& cvAssignParameter : cvAssignParameterList)
        addAndMakeVisible (cvAssignParameter);

    cvAssignParameterList [0].setParameterLabel ("BITS");
    cvAssignParameterList [1].setParameterLabel ("RATE");
    cvAssignParameterList [2].setParameterLabel ("LEVEL");
    cvAssignParameterList [3].setParameterLabel ("DECAY");
    cvAssignParameterList [4].setParameterLabel ("SPEED");
    cvAssignParameterList [5].setParameterLabel ("MODE");
    cvAssignParameterList [6].setParameterLabel ("REVERSE");
    cvAssignParameterList [7].setParameterLabel ("START");
    cvAssignParameterList [8].setParameterLabel ("END");
    cvAssignParameterList [9].setParameterLabel ("LOOP");
    cvAssignParameterList [10].setParameterLabel ("ATTACK");
    cvAssignParameterList [11].setParameterLabel ("CUE SET");
    cvAssignParameterList [12].setParameterLabel ("ETRIG");
    cvAssignParameterList [13].setParameterLabel ("FREQ");
    cvAssignParameterList [14].setParameterLabel ("RES");
}

void CvAssignSection::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::green);
    g.drawRect (getLocalBounds ());
}

void CvAssignSection::resized ()
{
    const auto assignParameterWidth { getWidth () / cvAssignParameterList.size () };
    for (auto curCvAssignParameter { 0 }; curCvAssignParameter < cvAssignParameterList.size (); ++curCvAssignParameter)
        cvAssignParameterList [curCvAssignParameter].setBounds (assignParameterWidth * curCvAssignParameter, 0, assignParameterWidth, getHeight());
}

CvAssignEditor::CvAssignEditor ()
{
    for (auto& cvAssignSection : cvAssignSectionList)
        addAndMakeVisible (cvAssignSection);
}

void CvAssignEditor::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::yellow);
    g.drawRect (getLocalBounds ());
}

void CvAssignEditor::resized ()
{
    for (auto curCvAssignSection { 0 }; curCvAssignSection < numCvSections; ++curCvAssignSection)
        cvAssignSectionList [curCvAssignSection].setBounds (0, 100 * curCvAssignSection, getWidth (), 100);
}
