#pragma once

#include <JuceHeader.h>
#include "CueSets/WaveformDisplay.h"
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
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TextButton saveButton;

    // Edit fields
    juce::Label attackLabel;
    CustomTextEditorInt attackTextEditor; // 0-99
    juce::Label bitsLabel;
    CustomTextEditorInt bitsTextEditor; // 1-16
    juce::Label decayLabel;
    CustomTextEditorInt decayTextEditor; // 0-99
    juce::Label endCueLabel;
    CustomTextEditorInt endCueTextEditor; // sampleStart - sample length
    juce::Label filterTypeLabel;
    CustomComboBox filterTypeComboBox; // Off, LP, BP, NT, HP (0-4)
    juce::Label filterFrequencyLabel;
    CustomTextEditorInt filterFrequencyTextEditor; // 1-99?
    juce::Label filterResonanceLabel;
    CustomTextEditorInt filterResonanceTextEditor; // 1-99?
    juce::Label levelLabel;
    CustomTextEditorInt levelTextEditor; // 1-99
    juce::Label loopCueLabel;
    CustomTextEditorInt loopCueTextEditor; // 0 - sample length?, or sampleStart - sampleEnd
    juce::Label loopModeLabel;
    CustomComboBox loopModeComboBox; // none, normal, zigZag, gate, zigZagGate (0-4)
    juce::Label quantLabel;
    CustomComboBox quantComboBox; // 0-14 (Off, 12, OT, MA, mi, Hm, PM, Pm, Ly, Ph, Jp, P5, C1, C4, C5)
    juce::Label rateLabel;
    CustomComboBox rateComboBox; // 4, 6, 7, 9, 11, 14, 22, 44
    juce::TextButton reverseButton; // 0-1
    juce::Label speedLabel;
    CustomTextEditorInt speedTextEditor; // 1 - 99 (50 is normal?, below that is negative speed? above is positive?)
    juce::Label startCueLabel;
    CustomTextEditorInt startCueTextEditor;  // 0 - sampleEnd
    juce::Label xfadeLabel;
    CustomTextEditorInt xfadeTextEditor; // 0 - 99
    WaveformDisplay waveformDisplay;
    std::array<juce::TextButton, 64> cueSetButtons;

    NoArrowComboBoxLnF noArrowComboBoxLnF;
    juce::TextButton loadButton;

    struct CueSet
    {
        juce::int64 start { 0 };
        juce::int64 loop { 0 };
        juce::int64 end { 0 };
    };
    int numCueSets { 0 }; // should it default to 1?
    int curCueSet { 0 };
    std::array<CueSet, 64> cueSets;

    void initializeCallbacks ();

    void attackDataChanged (int attack);
    void bitsDataChanged (int bits);
    void decayDataChanged (int decay);
    void endCueDataChanged (int endCue);
    void filterTypeDataChanged (int filterType);
    void filterFrequencyDataChanged (int filterFrequency);
    void filterResonanceDataChanged (int filterResonance);
    void levelDataChanged (int level);
    void loopCueDataChanged (int loopCue);
    void loopModeDataChanged (int loopMode);
    void quantDataChanged (int quant);
    void rateDataChanged (int rate);
    void reverseDataChanged (int reverse);
    void speedDataChanged (int speed);
    void startCueDataChanged (int startCue);
    void xfadeDataChanged (int xfade);

    void attackUiChanged (int attack);
    void bitsUiChanged (int bits);
    void decayUiChanged (int decay);
    void endCueUiChanged (int endCue);
    void filterTypeUiChanged (int filterType);
    void filterFrequencyUiChanged (int filterFrequency);
    void filterResonanceUiChanged (int filterResonance);
    void levelUiChanged (int level);
    void loopCueUiChanged (int loopCue);
    void loopModeUiChanged (int loopMode);
    void quantUiChanged (int quant);
    void rateUiChanged (int rate);
    void reverseUiChanged (int reverse);
    void speedUiChanged (int speed);
    void startCueUiChanged (int startCue);
    void xfadeUiChanged (int xfade);

    void setupComponents ();
    void setCurCue (int cueSetIndex);

    void timerCallback () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};
