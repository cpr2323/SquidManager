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
                destChannelProperties.setCvAssignEnabled (cvIndex, parameterIndex, squidChannelProperties.getCvAssignEnabled (cvIndex, parameterIndex), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setCvAssignEnabled (cvIndex, parameterIndex, defaultChannelProperties.getCvAssignEnabled (cvIndex, parameterIndex), false);
            },
            [this] ()
            {
               SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
               squidChannelProperties.setCvAssignEnabled (cvIndex, parameterIndex, uneditedChannelProperties.getCvAssignEnabled (cvIndex, parameterIndex), false);
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
        const auto newValue { getCvAttenuatonUiValue (squidChannelProperties.getCvAssignAttenuate (cvIndex, parameterIndex)) + (multiplier * direction) };
        cvAttenuateEditor.setValue (newValue);
    };
    cvAttenuateEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setCvAssignAttenuate (cvIndex, parameterIndex, squidChannelProperties.getCvAssignAttenuate (cvIndex, parameterIndex), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setCvAssignAttenuate (cvIndex, parameterIndex, defaultChannelProperties.getCvAssignAttenuate (cvIndex, parameterIndex), false);
            },
            [this] ()
            {
               SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
               squidChannelProperties.setCvAssignAttenuate (cvIndex, parameterIndex, uneditedChannelProperties.getCvAssignAttenuate (cvIndex, parameterIndex), false);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
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
    cvOffsetEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setCvAssignOffset (cvIndex, parameterIndex, squidChannelProperties.getCvAssignOffset (cvIndex, parameterIndex), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setCvAssignOffset (cvIndex, parameterIndex, defaultChannelProperties.getCvAssignOffset (cvIndex, parameterIndex), false);
            },
            [this] ()
            {
               SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
               squidChannelProperties.setCvAssignOffset (cvIndex, parameterIndex, uneditedChannelProperties.getCvAssignOffset (cvIndex, parameterIndex), false);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
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

int CvAssignParameter::getCvAttenuatonUiValue (int internalValue)
{
    if (internalValue > 99)
        return 100 - internalValue;
    else
        return internalValue;
}

int CvAssignParameter::getCvAttenuatonInternalValue (int uiValue)
{
    if (uiValue < 0)
        return 100 + std::abs (uiValue);
    else
        return uiValue;
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
    cvAttenuateEditor.setText (juce::String (getCvAttenuatonUiValue (attenuation)), juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::cvAssignAttenuateUiChanged (int attenuation)
{
    squidChannelProperties.setCvAssignAttenuate (cvIndex, parameterIndex, getCvAttenuatonInternalValue (attenuation), false);
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
