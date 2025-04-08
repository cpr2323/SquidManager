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
    assignEnableButton.setTooltip ("CV Assign Enable. Enables control of this parameter via CV.");
    assignEnableButton.onClick = [this] () { cvAssignEnableUiChanged (assignEnableButton.getToggleState ()); };
    assignEnableButton.onPopupMenuCallback = [this] ()
    {
            auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setCvAssignEnabled (cvIndex, parameterId, squidChannelProperties.getCvAssignEnabled (cvIndex, parameterId), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setCvAssignEnabled (cvIndex, parameterId, defaultChannelProperties.getCvAssignEnabled (cvIndex, parameterId), false);
            },
            [this] ()
            {
               SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
               squidChannelProperties.setCvAssignEnabled (cvIndex, parameterId, uneditedChannelProperties.getCvAssignEnabled (cvIndex, parameterId), false);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (assignEnableButton);

    // ATTENUATE TEXT EDITOR
    cvAttenuateLabel.setText ("ATN", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (cvAttenuateLabel);
    cvAttenuateEditor.setTooltip ("Attenuate CV Input. Adjusts the amount of attenuation applied to the CV before sending it to this parameter. Attenuation can be postive or negative.");
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
        const auto newValue { getCvAttenuatonUiValue (squidChannelProperties.getCvAssignAttenuate (cvIndex, parameterId)) + (multiplier * direction) };
        cvAttenuateEditor.setValue (newValue);
    };
    cvAttenuateEditor.onPopupMenuCallback = [this] ()
    {
            auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setCvAssignAttenuate (cvIndex, parameterId, squidChannelProperties.getCvAssignAttenuate (cvIndex, parameterId), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setCvAssignAttenuate (cvIndex, parameterId, defaultChannelProperties.getCvAssignAttenuate (cvIndex, parameterId), false);
            },
            [this] ()
            {
               SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
               squidChannelProperties.setCvAssignAttenuate (cvIndex, parameterId, uneditedChannelProperties.getCvAssignAttenuate (cvIndex, parameterId), false);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (cvAttenuateEditor);

    // OFFSET TEXT EDITOR
    cvOffsetLabel.setText ("OFS", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (cvOffsetLabel);
    cvOffsetEditor.setTooltip ("Offset CV Input. Adjusts the amount of offset applied to the CV before sending it to this parameter.");
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
        const auto newValue { squidChannelProperties.getCvAssignOffset (cvIndex, parameterId) + (multiplier * direction) };
        cvOffsetEditor.setValue (newValue);
    };
    cvOffsetEditor.onPopupMenuCallback = [this] ()
    {
            auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setCvAssignOffset (cvIndex, parameterId, squidChannelProperties.getCvAssignOffset (cvIndex, parameterId), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setCvAssignOffset (cvIndex, parameterId, defaultChannelProperties.getCvAssignOffset (cvIndex, parameterId), false);
            },
            [this] ()
            {
               SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
               squidChannelProperties.setCvAssignOffset (cvIndex, parameterId, uneditedChannelProperties.getCvAssignOffset (cvIndex, parameterId), false);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (cvOffsetEditor);
}

CvAssignParameter::~CvAssignParameter ()
{
    assignEnableButton.setLookAndFeel (nullptr);
}

void CvAssignParameter::init (juce::ValueTree rootPropertiesVT, juce::ValueTree squidChannelPropertiesVT, int theCvIndex, int theParameterId)
{
    cvIndex = theCvIndex;
    parameterId = theParameterId;

    RuntimeRootProperties runtimeRootProperties { rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no };
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();

    squidChannelProperties.wrap (squidChannelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);

    squidChannelProperties.onCvAssignEnabledChange = [this] (int inCvIndex, int inParameterId, bool isEnabled)
    {
        if (inCvIndex == cvIndex && inParameterId == parameterId)
            cvAssignEnableDataChanged (isEnabled);
    };
    squidChannelProperties.onCvAssignAttenuateChange = [this] (int inCvIndex, int inParameterId, int attenuation)
    {
        if (inCvIndex == cvIndex && inParameterId == parameterId)
            cvAssignAttenuateDataChanged (attenuation);
    };
    squidChannelProperties.onCvAssignOffsetChange = [this] (int inCvIndex, int inParameterId, int offset)
    {
        if (inCvIndex == cvIndex && inParameterId == parameterId)
            cvAssignOffsetDataChanged (offset);
    };

    cvAssignEnableDataChanged (squidChannelProperties.getCvAssignEnabled (cvIndex, parameterId));
    cvAssignAttenuateDataChanged (squidChannelProperties.getCvAssignAttenuate (cvIndex, parameterId));
    cvAssignOffsetDataChanged (squidChannelProperties.getCvAssignOffset (cvIndex, parameterId));
}

void CvAssignParameter::setParameterLabel (juce::String parameterText)
{
    parameterLabel.setText (parameterText, juce::NotificationType::dontSendNotification);
}

int CvAssignParameter::getParameterId ()
{
    return parameterId;
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
    squidChannelProperties.setCvAssignEnabled (cvIndex, parameterId, enabled, false);
}

void CvAssignParameter::cvAssignAttenuateDataChanged (int attenuation)
{
    cvAttenuateEditor.setText (juce::String (getCvAttenuatonUiValue (attenuation)), juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::cvAssignAttenuateUiChanged (int attenuation)
{
    squidChannelProperties.setCvAssignAttenuate (cvIndex, parameterId, getCvAttenuatonInternalValue (attenuation), false);
}

void CvAssignParameter::cvAssignOffsetDataChanged (int offset)
{
    cvOffsetEditor.setText (juce::String (offset), juce::NotificationType::dontSendNotification);
}

void CvAssignParameter::cvAssignOffsetUiChanged (int offset)
{
    squidChannelProperties.setCvAssignOffset (cvIndex, parameterId, offset, false);
}

void CvAssignParameter::enablementChanged ()
{
    const auto enabled { isEnabled () };
    parameterLabel.setEnabled (enabled);
    assignEnableLabel.setEnabled (enabled);
    assignEnableButton.setEnabled (enabled);
    cvAttenuateLabel.setEnabled (enabled);
    cvAttenuateEditor.setEnabled (enabled);
    cvOffsetLabel.setEnabled (enabled);
    cvOffsetEditor.setEnabled (enabled);
}

void CvAssignParameter::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::white.darker (0.3f));
    g.drawRect (getLocalBounds ());
}

void CvAssignParameter::paintOverChildren (juce::Graphics& g)
{
    if (! isEnabled ())
    {
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.fillAll ();
    }
}

void CvAssignParameter::resized ()
{
    auto localBounds { getLocalBounds ().reduced (2,4) };
    const auto lineHeight { localBounds.getHeight () / 4 };
    parameterLabel.setBounds (localBounds.removeFromTop (lineHeight));
    localBounds.removeFromTop (1);
    auto assignEnableLine { localBounds.removeFromTop (lineHeight).withTrimmedBottom (1) };
    assignEnableLabel.setBounds (assignEnableLine.removeFromLeft (assignEnableLine.getWidth () / 2));
    assignEnableButton.setBounds (assignEnableLine.withTrimmedTop (3).withTrimmedBottom (5).withTrimmedRight (3));

    localBounds.removeFromTop (1);
    auto cvAttenuateBounds { localBounds.removeFromTop (lineHeight) };
    cvAttenuateLabel.setBounds (cvAttenuateBounds.removeFromLeft (cvAttenuateBounds.getWidth () / 2));
    cvAttenuateEditor.setBounds (cvAttenuateBounds);

    localBounds.removeFromTop (1);
    auto cvOffsetBounds { localBounds.removeFromTop (lineHeight) };
    cvOffsetLabel.setBounds (cvOffsetBounds.removeFromLeft (cvOffsetBounds.getWidth () / 2));
    cvOffsetEditor.setBounds (cvOffsetBounds);
}
