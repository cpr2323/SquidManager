#include "CvAssignParameter.h"
#include "../../../SystemServices.h"
#include "../../../Utility/RuntimeRootProperties.h"

CvAssignParameter::CvAssignParameter ()
{
    parameterLabel.setJustificationType (juce::Justification::centred);
    parameterLabel.setText ("---", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (parameterLabel);

    // ENABLE BUTTON
    assignEnableLabel.setText ("ON", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (assignEnableLabel);
    assignEnableButton.onClick = [this] () { cvAssignEnableUiChanged (assignEnableButton.getToggleState ()); };
    assignEnableButton.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
//                destChannelProperties.setChannelSource (squidChannelProperties.getChannelSource (), false);
            },
            [this] ()
            {
//                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
//                                                                    SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
//                squidChannelProperties.setChannelSource (defaultChannelProperties.getChannelSource (), true);
            },
            [this] ()
            {
//                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
//                                                                    SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
//                squidChannelProperties.setChannelSource (uneditedChannelProperties.getChannelSource (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (assignEnableButton);

    // ATTENUATE TEXT EDITOR
    cvAttenuateLabel.setText ("ATN", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (cvAttenuateLabel);
    cvAttenuateEditor.getMinValueCallback = [this] () { return -99; };
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
        const auto newValue { squidChannelProperties.getCvAssignAttenuate (cvIndex, parameterIndex) + (multiplier * direction) };
        cvAttenuateEditor.setValue (newValue);
    };
    cvAttenuateEditor.onPopupMenuCallback = [this] ()
    {
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
        const auto newValue { squidChannelProperties.getCvAssignOffset (cvIndex, parameterIndex) + (multiplier * direction) };
        cvOffsetEditor.setValue (newValue);
    };
    cvAttenuateEditor.onPopupMenuCallback = [this] ()
    {
    };
    addAndMakeVisible (cvOffsetEditor);
}

CvAssignParameter::~CvAssignParameter ()
{
    assignEnableButton.setLookAndFeel (nullptr);
}

void CvAssignParameter::init (juce::ValueTree rootPropertiesVT, juce::ValueTree squidChannelPropertiesVT, int theCvIndex, int theParameterIndex)
{
    cvIndex = theCvIndex;
    parameterIndex = theParameterIndex;

    RuntimeRootProperties runtimeRootProperties { rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no };
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();

    squidChannelProperties.wrap (squidChannelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);

    squidChannelProperties.onCvAssignEnabledChange = [this] (int inCvIndex, int inParameterIndex, bool isEnabled)
    {
        if (inCvIndex == cvIndex && inParameterIndex == parameterIndex)
            cvAssignEnableDataChanged (isEnabled);
    };
    squidChannelProperties.onCvAssignAttenuateChange = [this] (int inCvIndex, int inParameterIndex, int attenuation)
    {
        if (inCvIndex == cvIndex && inParameterIndex == parameterIndex)
            cvAssignAttenuateDataChanged (attenuation);
    };
    squidChannelProperties.onCvAssignOffsetChange = [this] (int inCvIndex, int inParameterIndex, int offset)
    {
        if (inCvIndex == cvIndex && inParameterIndex == parameterIndex)
            cvAssignOffsetDataChanged (offset);
    };

    cvAssignEnableDataChanged (squidChannelProperties.getCvAssignEnabled (cvIndex, parameterIndex));
    cvAssignAttenuateDataChanged (squidChannelProperties.getCvAssignAttenuate (cvIndex, parameterIndex));
    cvAssignOffsetDataChanged (squidChannelProperties.getCvAssignOffset (cvIndex, parameterIndex));
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
    squidChannelProperties.setCvAssignEnabled (cvIndex, parameterIndex, enabled, false);
}

void CvAssignParameter::cvAssignAttenuateDataChanged (int attenuation)
{
    const auto uiAttenuationValue = [attenuation] ()
    {
        if (attenuation > 99)
            return 100 - attenuation;
        else
            return attenuation;
    } ();
    cvAttenuateEditor.setText (juce::String (uiAttenuationValue), juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::cvAssignAttenuateUiChanged (int attenuation)
{
    const auto rawAttenuationValue = [attenuation] ()
    {
        if (attenuation < 0)
            return 100 + std::abs (attenuation);
        else
            return attenuation;
    } ();
    squidChannelProperties.setCvAssignAttenuate (cvIndex, parameterIndex, rawAttenuationValue, false);
}

void CvAssignParameter::cvAssignOffsetDataChanged (int offset)
{
    cvOffsetEditor.setText (juce::String (offset), juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::cvAssignOffsetUiChanged (int offset)
{
    squidChannelProperties.setCvAssignOffset (cvIndex, parameterIndex, offset, false);
}

void CvAssignParameter::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::white.darker (0.3f));
    g.drawRect (getLocalBounds ());
}

void CvAssignParameter::resized ()
{
    auto localBounds { getLocalBounds ().reduced (2,4) };
    const auto lineHeight { localBounds.getHeight () / 4 };
    parameterLabel.setBounds (localBounds.removeFromTop (lineHeight));
    localBounds.removeFromTop (1);
    auto assignEnableLine { localBounds.removeFromTop (lineHeight).withTrimmedBottom(1) };
    assignEnableLabel.setBounds (assignEnableLine.removeFromLeft (assignEnableLine.getWidth () / 2));
    assignEnableButton.setBounds (assignEnableLine.withTrimmedTop(3).withTrimmedBottom(5).withTrimmedRight(3));

    localBounds.removeFromTop (1);
    auto cvAttenuateBounds { localBounds.removeFromTop (lineHeight) };
    cvAttenuateLabel.setBounds (cvAttenuateBounds.removeFromLeft (cvAttenuateBounds.getWidth () / 2));
    cvAttenuateEditor.setBounds (cvAttenuateBounds);

    localBounds.removeFromTop (1);
    auto cvOffsetBounds { localBounds.removeFromTop (lineHeight) };
    cvOffsetLabel.setBounds (cvOffsetBounds.removeFromLeft (cvOffsetBounds.getWidth () / 2));
    cvOffsetEditor.setBounds (cvOffsetBounds);
}
