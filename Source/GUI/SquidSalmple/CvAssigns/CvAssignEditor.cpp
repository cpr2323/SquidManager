#include "CvAssignEditor.h"
#include "../../../Utility/RuntimeRootProperties.h"

CvAssignParameter::CvAssignParameter ()
{
    parameterLabel.setJustificationType (juce::Justification::centred);
    parameterLabel.setText ("---", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (parameterLabel);

    // ENABLE BUTTON
    assignEnableButton.setLookAndFeel (&textOnLeftToggleButtonLnF);
    assignEnableButton.setButtonText ("ON");
    assignEnableButton.onClick = [this] () { cvAssignEnableUiChanged (assignEnableButton.getToggleState ()); };
    addAndMakeVisible (assignEnableButton);

    // ATTENUATE TEXT EDITOR
    cvAttenuateLabel.setText ("ATN", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (cvAttenuateLabel);
    cvAttenuateEditor.getMinValueCallback = [this] () { return 0; };
    cvAttenuateEditor.getMaxValueCallback = [this] () { return 99; };
    cvAttenuateEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    cvAttenuateEditor.updateDataCallback = [this] (int value) { cvAssignAttenuateUiChanged (value); };
    cvAttenuateEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 10;
            else
                return 25;
        } ();
        const auto newValue { squidMetaDataProperties.getCvAssignAttenuate (cvIndex, parameterIndex) + (multiplier * direction) };
        cvAttenuateEditor.setValue (newValue);
    };
    addAndMakeVisible (cvAttenuateEditor);

    // OFFSET TEXT EDITOR
    cvOffsetLabel.setText ("OFS", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (cvOffsetLabel);
    cvOffsetEditor.getMinValueCallback = [this] () { return 0; };
    cvOffsetEditor.getMaxValueCallback = [this] () { return 99; };
    cvOffsetEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    cvOffsetEditor.updateDataCallback = [this] (int value) { cvAssignOffsetUiChanged (value); };
    cvOffsetEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 10;
            else
                return 25;
        } ();
        const auto newValue { squidMetaDataProperties.getCvAssignOffset (cvIndex, parameterIndex) + (multiplier * direction) };
        cvOffsetEditor.setValue (newValue);
    };
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

    RuntimeRootProperties runtimeRootProperties { rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no };
    squidMetaDataProperties.wrap (runtimeRootProperties.getValueTree (), SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::yes);

    squidMetaDataProperties.onCvAssignEnabledChange = [this] (int inCvIndex, int inParameterIndex, bool isEnabled)
    {
        if (inCvIndex == cvIndex && inParameterIndex == parameterIndex)
            cvAssignEnableDataChanged (isEnabled);
    };
    squidMetaDataProperties.onCvAssignAttenuateChange = [this] (int inCvIndex, int inParameterIndex, int attenuation)
    {
        if (inCvIndex == cvIndex && inParameterIndex == parameterIndex)
            cvAssignAttenuateDataChanged (attenuation);
    };
    squidMetaDataProperties.onCvAssignOffsetChange = [this] (int inCvIndex, int inParameterIndex, int offset)
    {
        if (inCvIndex == cvIndex && inParameterIndex == parameterIndex)
            cvAssignOffsetDataChanged (offset);
    };

    cvAssignEnableDataChanged (squidMetaDataProperties.getCvAssignEnabled (cvIndex,parameterIndex));
    cvAssignAttenuateDataChanged (squidMetaDataProperties.getCvAssignAttenuate (cvIndex, parameterIndex));
    cvAssignOffsetDataChanged (squidMetaDataProperties.getCvAssignOffset (cvIndex, parameterIndex));
}

void CvAssignParameter::setParameterLabel (juce::String parameterText)
{
    parameterLabel.setText (parameterText, juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::cvAssignEnableDataChanged (bool enabled)
{
    assignEnableButton.setToggleState (enabled, juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::cvAssignEnableUiChanged (bool enabled)
{
    squidMetaDataProperties.setCvAssignEnabled (cvIndex, parameterIndex, enabled, false);
}

void CvAssignParameter::cvAssignAttenuateDataChanged (int attenuation)
{
    cvAttenuateEditor.setText (juce::String (attenuation), juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::cvAssignAttenuateUiChanged (int attenuation)
{
    squidMetaDataProperties.setCvAssignAttenuate (cvIndex, parameterIndex, attenuation, false);
}

void CvAssignParameter::cvAssignOffsetDataChanged (int offset)
{
    cvOffsetEditor.setText (juce::String (offset), juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::cvAssignOffsetUiChanged (int offset)
{
    squidMetaDataProperties.setCvAssignOffset (cvIndex, parameterIndex, offset, false);
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
