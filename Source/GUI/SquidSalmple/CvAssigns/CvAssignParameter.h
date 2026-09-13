#pragma once
#include <JuceHeader.h>
#include "../../../SquidSalmple/EditManager/EditManager.h"
#include "../../../SquidSalmple/SquidChannelProperties.h"
#include "oolib/GUI/CustomTextEditor.h"
#include "oolib/GUI/RoundedSlideSwitch.h"

class CvAssignParameter : public juce::Component
{
public:
    CvAssignParameter ();
    ~CvAssignParameter ();
    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree squidChannelPropertiesVT, int theCvIndex, int theParameterId);
    void setParameterLabel (juce::String parameterText);
    int getParameterId ();

private:
    int cvIndex { -1 };
    int parameterId { -1 };
    SquidChannelProperties squidChannelProperties;
    EditManager* editManager { nullptr };

    juce::Label parameterLabel;
    RoundedSlideSwitch assignEnableButton;
    juce::Label cvAttenuateLabel;
    CustomTextEditorInt cvAttenuateEditor;
    juce::Label cvOffsetLabel;
    // The ATN and OFS labels never change, so they are measured once rather than
    // on every layout: shaping text is expensive, and there are 112 of these
    // components laid out together.
    int cvAttenuateLabelWidth { 0 };
    int cvOffsetLabelWidth { 0 };
    CustomTextEditorInt cvOffsetEditor;

    int getCvAttenuatonUiValue (int internalValue);
    int getCvAttenuatonInternalValue (int uiValue);

    void cvAssignEnableDataChanged (bool enabled);
    void cvAssignEnableUiChanged (bool enabled);
    void cvAssignAttenuateDataChanged (int attenuation);
    void cvAssignAttenuateUiChanged (int attenuation);
    void cvAssignOffsetDataChanged (int offset);
    void cvAssignOffsetUiChanged (int offset);

    void enablementChanged () override;
    void lookAndFeelChanged () override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized () override;
};
