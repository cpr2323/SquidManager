#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../SquidSalmple/SquidChannelProperties.h"
#include "../../../Utility/CustomComboBox.h"
#include "../../../Utility/CustomTextEditor.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/NoArrowComboBoxLnF.h"

class TextOnLeftToggleButtonLnF : public juce::LookAndFeel_V4
{
public:
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
    {
        auto localBounds { button.getLocalBounds () };
        auto fontSize = juce::jmin (15.0f, (float) button.getHeight () * 0.75f);
        g.setColour (button.findColour (juce::ToggleButton::textColourId));
        g.setFont (fontSize);

        if (!button.isEnabled ())
            g.setOpacity (0.5f);
        g.drawFittedText (button.getButtonText (),
            localBounds.removeFromLeft (localBounds.getWidth () / 2).withTrimmedRight (2),
            juce::Justification::centredRight, 10);

        auto tickWidth = fontSize * 1.1f;

        drawTickBox (g, button, localBounds.getX () + ((localBounds.getWidth () / 2) - (tickWidth / 2)), ((float) button.getHeight () - tickWidth) * 0.5f,
            tickWidth, tickWidth,
            button.getToggleState (),
            button.isEnabled (),
            shouldDrawButtonAsHighlighted,
            shouldDrawButtonAsDown);
    }
};

class CvAssignParameter : public juce::Component
{
public:
    CvAssignParameter ();
    ~CvAssignParameter ();
    void init (juce::ValueTree squidChannelPropertiesVT, int theCvIndex, int theParameterIndex);
    void setParameterLabel (juce::String parameterText);

private:
    int cvIndex { -1 };
    int parameterIndex { -1 };
    SquidChannelProperties squidChannelProperties;
    juce::Label parameterLabel;
    juce::ToggleButton assignEnableButton;
    juce::Label cvAttenuateLabel;
    CustomTextEditorInt cvAttenuateEditor;
    juce::Label cvOffsetLabel;
    CustomTextEditorInt cvOffsetEditor;

    TextOnLeftToggleButtonLnF textOnLeftToggleButtonLnF;

    void cvAssignEnableDataChanged (bool enabled);
    void cvAssignEnableUiChanged (bool enabled);
    void cvAssignAttenuateDataChanged (int attenuation);
    void cvAssignAttenuateUiChanged (int attenuation);
    void cvAssignOffsetDataChanged (int offset);
    void cvAssignOffsetUiChanged (int offset);

    void paint (juce::Graphics& g);
    void resized () override;
};

class CvAssignSection : public juce::Component
{
public:
    CvAssignSection ();
    void init (juce::ValueTree rootPropertiesVT, int theCvIndex);

private:
//    int cvIndex { -1 };
    juce::Label cvAssignLabel;
    std::array<CvAssignParameter, 15>  cvAssignParameterList;

    //void paint (juce::Graphics& g) override;
    void resized () override;
};

class CvAssignEditor : public juce::Component
{
public:
    CvAssignEditor ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::Label cvAssignLabel;
    std::array<CvAssignSection, 8>  cvAssignSectionList;
    int numCvSections { 3 };

    void paint (juce::Graphics& g);
    void resized () override;
};