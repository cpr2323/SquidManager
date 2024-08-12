#pragma once
#include <JuceHeader.h>
#include "../../../SquidSalmple/EditManager/EditManager.h"
#include "../../../SquidSalmple/SquidChannelProperties.h"
#include "../../../Utility/CustomTextEditor.h"
#include "../../../Utility/RoundedSlideSwitch.h"

class CvAssignParameter : public juce::Component
{
public:
    CvAssignParameter ();
    ~CvAssignParameter ();
    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree squidChannelPropertiesVT, int theCvIndex, int theParameterIndex);
    void setParameterLabel (juce::String parameterText);
    int getParameterIndex ();

private:
    int cvIndex { -1 };
    int parameterIndex { -1 };
    SquidChannelProperties squidChannelProperties;
    EditManager* editManager { nullptr };

    juce::Label parameterLabel;
    juce::Label assignEnableLabel;
    RoundedSlideSwitch assignEnableButton;
    juce::Label cvAttenuateLabel;
    CustomTextEditorInt cvAttenuateEditor;
    juce::Label cvOffsetLabel;
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
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized () override;
};
