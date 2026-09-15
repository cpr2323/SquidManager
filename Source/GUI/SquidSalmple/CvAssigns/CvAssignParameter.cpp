#include "CvAssignParameter.h"
#include "../../Theme/SquidColourIds.h"
#include "../../Theme/UiComponents.h"
#include "../../../SystemServices.h"
#include "../../../SquidSalmple/CvParameterProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

CvAssignParameter::CvAssignParameter ()
{
    // set as the parameter labels in the panel above, which name the same parameters
    parameterLabel.setFont (SquidType::parameterLabel ());
    parameterLabel.setBorderSize ({ 0, 0, 0, 0 });
    parameterLabel.setJustificationType (juce::Justification::centred);
    parameterLabel.setText ("---", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (parameterLabel);

    // ENABLE BUTTON
    assignEnableButton.setThumbShape (RoundedSlideSwitch::ThumbShape::circle);
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
    cvAttenuateLabel.setFont (SquidType::cvFieldLabel ());
    cvAttenuateLabel.setBorderSize ({ 0, 0, 0, 0 });
    cvAttenuateLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (cvAttenuateLabel);
    cvAttenuateEditor.setTooltip ("Attenuate CV Input. Adjusts the amount of attenuation applied to the CV before sending it to this parameter. Attenuation can be postive or negative.");
    cvAttenuateEditor.getMinValueCallback = [this] () { return -99; };
    cvAttenuateEditor.getMaxValueCallback = [this] () { return 99; };
    cvAttenuateEditor.setFont (SquidType::cvValue ());
    cvAttenuateEditor.setJustification (juce::Justification::centredRight);
    cvAttenuateEditor.setBorder ({ 0, 5, 0, 5 });
    cvAttenuateEditor.setIndents (0, 0);
    HoverHighlight::attach (cvAttenuateEditor);
    cvAttenuateEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    cvAttenuateEditor.updateDataCallback = [this] (int value) { cvAssignAttenuateUiChanged (value); };
    cvAttenuateEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { getCvAttenuatonUiValue (squidChannelProperties.getCvAssignAttenuate (cvIndex, parameterId)) + static_cast<int> (valueDelta) };
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
    cvOffsetLabel.setFont (SquidType::cvFieldLabel ());
    cvOffsetLabel.setBorderSize ({ 0, 0, 0, 0 });
    cvOffsetLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (cvOffsetLabel);
    cvOffsetEditor.setTooltip ("Offset CV Input. Adjusts the amount of offset applied to the CV before sending it to this parameter.");
    cvOffsetEditor.getMinValueCallback = [this] () { return 0; };
    cvOffsetEditor.getMaxValueCallback = [this] () { return 99; };
    cvOffsetEditor.setFont (SquidType::cvValue ());
    cvOffsetEditor.setJustification (juce::Justification::centredRight);
    cvOffsetEditor.setBorder ({ 0, 5, 0, 5 });
    cvOffsetEditor.setIndents (0, 0);
    HoverHighlight::attach (cvOffsetEditor);
    cvOffsetEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    cvOffsetEditor.updateDataCallback = [this] (int value) { cvAssignOffsetUiChanged (value); };
    cvOffsetEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { squidChannelProperties.getCvAssignOffset (cvIndex, parameterId) + static_cast<int> (valueDelta) };
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

    cvAttenuateLabelWidth = SquidPaint::textWidth (cvAttenuateLabel.getFont (), cvAttenuateLabel.getText ()) + 1;
    cvOffsetLabelWidth = SquidPaint::textWidth (cvOffsetLabel.getFont (), cvOffsetLabel.getText ()) + 1;
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

    CvParameterProperties cvParameterProperties { squidChannelProperties.getCvParameterVT (cvIndex, parameterId), CvParameterProperties::WrapperType::client, CvParameterProperties::EnableCallbacks::no };
    parameterLabel.setText (cvParameterProperties.getName (), juce::NotificationType::dontSendNotification);
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
    assignEnableButton.setEnabled (enabled);
    cvAttenuateLabel.setEnabled (enabled);
    cvAttenuateEditor.setEnabled (enabled);
    cvOffsetLabel.setEnabled (enabled);
    cvOffsetEditor.setEnabled (enabled);
}

void CvAssignParameter::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    for (auto* label : { &parameterLabel, &cvAttenuateLabel, &cvOffsetLabel })
        label->setColour (juce::Label::textColourId, findColour (SquidColours::textDim));
}

void CvAssignParameter::paintOverChildren (juce::Graphics& g)
{
    if (! isEnabled ())
    {
        g.setColour (findColour (SquidColours::disabledOverlay));
        g.fillAll ();
    }
}

void CvAssignParameter::resized ()
{
    // name, switch, then the two values, all centred in the column
    static constexpr auto kGap { 5 };
    static constexpr auto kNameHeight { 14 };
    static constexpr auto kSwitchWidth { 28 };
    static constexpr auto kSwitchHeight { 15 };
    static constexpr auto kValueHeight { 17 };
    static constexpr auto kValueWidth { 32 };
    static constexpr auto kLabelGap { 4 };

    auto localBounds { getLocalBounds ().withTrimmedTop (6).reduced (5, 0) };
    parameterLabel.setBounds (localBounds.removeFromTop (kNameHeight));
    localBounds.removeFromTop (kGap);
    assignEnableButton.setBounds (localBounds.removeFromTop (kSwitchHeight).withSizeKeepingCentre (kSwitchWidth, kSwitchHeight));

    auto placeValue = [&localBounds] (juce::Label& label, int labelWidth, juce::Component& editor)
    {
        localBounds.removeFromTop (kGap);
        auto row { localBounds.removeFromTop (kValueHeight)
                              .withSizeKeepingCentre (std::min (localBounds.getWidth (), labelWidth + kLabelGap + kValueWidth), kValueHeight) };
        label.setBounds (row.removeFromLeft (labelWidth));
        row.removeFromLeft (kLabelGap);
        editor.setBounds (row);
    };
    placeValue (cvAttenuateLabel, cvAttenuateLabelWidth, cvAttenuateEditor);
    placeValue (cvOffsetLabel, cvOffsetLabelWidth, cvOffsetEditor);
}
