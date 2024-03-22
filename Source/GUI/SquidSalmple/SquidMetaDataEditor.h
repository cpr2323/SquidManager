#pragma once

#include <JuceHeader.h>
#include "../../AppProperties.h"
#include "../../SquidSalmple/SquidMetaDataProperties.h"
#include "../../Utility/CustomComboBox.h"
#include "../../Utility/CustomTextEditor.h"
#include "../../Utility/DebugLog.h"
#include "../../Utility/RuntimeRootProperties.h"
#include "../../Utility/NoArrowComboBoxLnF.h"

class SquidMetaDataEditorComponent : public juce::Component,
                                     public juce::Timer
{
public:
    SquidMetaDataEditorComponent ();
    ~SquidMetaDataEditorComponent ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    SquidMetaDataProperties squidMetaDataProperties;

    juce::TextButton saveButton;

    // Edit fields
    juce::Label attackLabel;
    CustomTextEditorInt attackTextEditor; // 0-99
    juce::Label bitsLabel;
    CustomTextEditorInt bitsTextEditor; // 1-16
    juce::Label decayLabel;
    CustomTextEditorInt decayTextEditor; // 0-99
    juce::Label filterLabel;
    CustomComboBox filterComboBox; // Off, LP, BP, NT, HP (0-4)
    juce::Label filterFrequencyLabel;
    CustomTextEditorInt filterFrequencyTextEditor; // 1-99?
    juce::Label filterResonanceLabel;
    CustomTextEditorInt filterResonanceTextEditor; // 1-99?
    juce::Label levelLabel;
    CustomTextEditorInt levelTextEditor; // 1-99
    juce::Label loopLabel;
    CustomTextEditorInt loopTextEditor; // 0 - sample length?, or sampleStart - sampleEnd
    juce::Label loopModeLabel;
    CustomComboBox loopModeComboBox; // none, normal, zigZag, gate, zigZagGate (0-4)
    juce::Label quantLabel;
    CustomComboBox quantComboBox; // 0-14 (Off, 12, OT, MA, mi, Hm, PM, Pm, Ly, Ph, Jp, P5, C1, C4, C5)
    juce::Label rateLabel;
    CustomComboBox rateComboBox; // 4, 6, 7, 9, 11, 14, 22, 44
    juce::TextButton reverseButton; // 0-1
    juce::Label sampleEndLabel;
    CustomTextEditorInt sampleEndTextEditor; // sampleStart - sample length
    juce::Label sampleStartLabel;
    CustomTextEditorInt sampleStartTextEditor;  // 0 - sampleEnd
    juce::Label speedLabel;
    CustomTextEditorInt speedTextEditor; // 1 - 99 (50 is normal?, below that is negative speed? above is positive?)
    juce::Label xfadeLabel;
    CustomTextEditorInt xfadeTextEditor; // 0 - 99

    NoArrowComboBoxLnF noArrowComboBoxLnF;

    void timerCallback () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};
