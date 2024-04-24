#include "CvAssignEditor.h"
#include "../../../Utility/RuntimeRootProperties.h"

CvAssignParameter::CvAssignParameter ()
{
    parameterLabel.setJustificationType (juce::Justification::centred);
    parameterLabel.setText ("---", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (parameterLabel);

    assignEnableButton.setLookAndFeel (&textOnLeftToggleButtonLnF);
    assignEnableButton.setButtonText ("ON");
    assignEnableButton.setConnectedEdges (juce::Button::ConnectedEdgeFlags::ConnectedOnLeft);
    addAndMakeVisible (assignEnableButton);

    cvAttenuateLabel.setText ("ATN", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (cvAttenuateLabel);
    cvAttenuateEditor.getMinValueCallback = [this] () { return 0; };
    cvAttenuateEditor.getMaxValueCallback = [this] () { return 99; };
    cvAttenuateEditor.setText ("50");
    addAndMakeVisible (cvAttenuateEditor);

    cvOffsetLabel.setText ("OFS", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (cvOffsetLabel);
    cvOffsetEditor.getMinValueCallback = [this] () { return 0; };
    cvOffsetEditor.getMaxValueCallback = [this] () { return 99; };
    cvOffsetEditor.setText ("0");
    addAndMakeVisible (cvOffsetEditor);
}

CvAssignParameter::~CvAssignParameter ()
{
    assignEnableButton.setLookAndFeel (nullptr);
}

void CvAssignParameter::init (juce::ValueTree rootPropertiesVT, int theCvIndex, int theParameterIndex)
{
    cvIndex = theCvIndex;
    parameterIndex = theParameterIndex;

    RuntimeRootProperties runtimeRootProperties { rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes };
    squidMetaDataProperties.wrap (runtimeRootProperties.getValueTree (), SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::yes);
}

void CvAssignParameter::setParameterLabel (juce::String parameterText)
{
    parameterLabel.setText (parameterText, juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::white.darker(0.3f));
    g.drawRect (getLocalBounds ());
}

void CvAssignParameter::resized ()
{
    auto localBounds { getLocalBounds().reduced (2,4)};
    const auto lineHeight { localBounds.getHeight () / 4 };
    parameterLabel.setBounds (localBounds.removeFromTop (lineHeight));
    localBounds.removeFromTop (1);
    assignEnableButton.setBounds (localBounds.removeFromTop (lineHeight));

    localBounds.removeFromTop (1);
    auto cvAttenuateBounds { localBounds.removeFromTop (lineHeight) };
    cvAttenuateLabel.setBounds (cvAttenuateBounds.removeFromLeft (cvAttenuateBounds.getWidth () / 2));
    cvAttenuateEditor.setBounds (cvAttenuateBounds);

    localBounds.removeFromTop (1);
    auto cvOffsetBounds { localBounds.removeFromTop (lineHeight) };
    cvOffsetLabel.setBounds (cvOffsetBounds.removeFromLeft (cvOffsetBounds.getWidth () / 2));
    cvOffsetEditor.setBounds (cvOffsetBounds);
}

CvAssignSection::CvAssignSection ()
{
    const auto kParameterNameList { std::vector<juce::String> { "BITS", "RATE", "LEVEL", "DECAY", "SPEED", "MODE", "REVERSE", "START",
                                                                "END", "LOOP", "ATTACK", "CUE SET", "ETRIG", "FREQ", "RES" } };
    for (auto curParameterIndex { 0 }; curParameterIndex < cvAssignParameterList.size(); ++curParameterIndex)
    {
        auto& curParameter { cvAssignParameterList [curParameterIndex] };
        curParameter.setParameterLabel (kParameterNameList[curParameterIndex]);
        addAndMakeVisible (curParameter);
    }
}

void CvAssignSection::init (juce::ValueTree rootPropertiesVT, int theCvIndex)
{
    for (auto curParameterIndex { 0 }; curParameterIndex < cvAssignParameterList.size (); ++curParameterIndex)
    {
        auto& curParameter { cvAssignParameterList [curParameterIndex] };
        curParameter.init (rootPropertiesVT, theCvIndex, curParameterIndex);
    }
}

// void CvAssignSection::paint (juce::Graphics& g)
// {
//     g.setColour (juce::Colours::green);
//     g.drawRect (getLocalBounds ());
// }

void CvAssignSection::resized ()
{
    const auto assignParameterWidth { getWidth () / cvAssignParameterList.size () };
    const auto diff {getWidth() - (assignParameterWidth * cvAssignParameterList.size ())};
    for (auto curCvAssignParameter { 0 }; curCvAssignParameter < cvAssignParameterList.size (); ++curCvAssignParameter)
        cvAssignParameterList [curCvAssignParameter].setBounds (assignParameterWidth * curCvAssignParameter, 0, assignParameterWidth, getHeight());
}

CvAssignEditor::CvAssignEditor ()
{
    for (auto& cvAssignSection : cvAssignSectionList)
        addAndMakeVisible (cvAssignSection);
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
    for (auto curCvAssignSection { 0 }; curCvAssignSection < numCvSections; ++curCvAssignSection)
        cvAssignSectionList [curCvAssignSection].setBounds (1, 1 + (100 * curCvAssignSection), getWidth () - 2, 100);
}
