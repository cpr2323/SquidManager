#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../SquidSalmple/SquidMetaDataProperties.h"
#include "../../../Utility/CustomComboBox.h"
#include "../../../Utility/CustomTextEditor.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/NoArrowComboBoxLnF.h"

class CvAssignParameter : public juce::Component
{
public:
    CvAssignParameter ();
    void setParameterLabel (juce::String parameterText);

private:
    juce::Label parameterLabel;
    juce::TextButton assignEnable;
    CustomTextEditorInt cvAttenuateEditor;
    CustomTextEditorInt cvOffsetEditor;

    void paint (juce::Graphics& g);
    void resized () override;
};

class CvAssignSection : public juce::Component
{
public:
    CvAssignSection ();

private:
    juce::Label cvAssignLabel;
    std::array<CvAssignParameter, 15>  cvAssignParameterList;

    void paint (juce::Graphics& g) override;
    void resized () override;
};

class CvAssignEditor : public juce::Component
{
public:

    CvAssignEditor ();

private:
    juce::Label cvAssignLabel;
    std::array<CvAssignSection, 8>  cvAssignSectionList;
    int numCvSections { 3 };

    void paint (juce::Graphics& g);
    void resized () override;
};