#pragma once

#include <JuceHeader.h>
#include "CueSets/WaveformDisplay.h"
#include "CvAssigns/CvAssignEditor.h"
#include "LoopPoints/LoopPointsView.h"
#include "../../AppProperties.h"
#include "../../SquidSalmple/EditManager/EditManager.h"
#include "../../SquidSalmple/SquidChannelProperties.h"
#include "../../Utility/CustomComboBox.h"
#include "../../Utility/CustomTextEditor.h"
#include "../../Utility/FileSelectLabel.h"
#include "../../Utility/NoArrowComboBoxLnF.h"
#include "../../Utility/RoundedSlideSwitch.h"

class ChannelEditorComponent : public juce::Component,
                               public juce::FileDragAndDropTarget
{
public:
    ChannelEditorComponent ();
    ~ChannelEditorComponent ();

    void init (juce::ValueTree squidChannelPropertiesVT, juce::ValueTree rootPropertiesVT);
    void initCueSetTabs ();

private:
    SquidChannelProperties squidChannelProperties;
    AppProperties appProperties;
    EditManager* editManager;

    // Edit fields
    juce::Label attackLabel;
    CustomTextEditorInt attackTextEditor; // 0-99
    juce::Label bitsLabel;
    CustomTextEditorInt bitsTextEditor; // 1-16
    juce::Label channelSourceLabel;
    CustomComboBox channelSourceComboBox;
    juce::Label chokeLabel;
    CustomComboBox chokeComboBox; // C1, C2, C3, C4, C5, C6, C7, C8
    juce::Label decayLabel;
    CustomTextEditorInt decayTextEditor; // 0-99
    juce::Label endCueLabel;
    CustomTextEditorInt32 endCueTextEditor; // sampleStart - sample length
    juce::Label eTrigLabel;
    CustomComboBox eTrigComboBox; // Off, > 1, > 2, > 3, > 4, > 5, > 6, > 7, > 8, On
    juce::Label fileNameLabel;
    FileSelectLabel fileNameSelectLabel;
    juce::Label filterTypeLabel;
    CustomComboBox filterTypeComboBox; // Off, LP, BP, NT, HP (0-4)
    juce::Label filterFrequencyLabel;
    CustomTextEditorInt filterFrequencyTextEditor; // 1-99?
    juce::Label filterResonanceLabel;
    CustomTextEditorInt filterResonanceTextEditor; // 1-99?
    juce::Label levelLabel;
    CustomTextEditorInt levelTextEditor; // 1-99
    juce::Label loopCueLabel;
    CustomTextEditorInt32 loopCueTextEditor; // 0 - sample length?, or sampleStart - sampleEnd
    juce::Label loopModeLabel;
    CustomComboBox loopModeComboBox; // none, normal, zigZag, gate, zigZagGate (0-4)
    juce::Label outputLabel;
    CustomComboBox outputComboBox; // chan 1-4 = 1-2,3-4 / chan 5-8 = 5-6,7-8
    juce::Label quantLabel;
    CustomComboBox quantComboBox; // 0-14 (Off, 12, OT, MA, mi, Hm, PM, Pm, Ly, Ph, Jp, P5, C1, C4, C5)
    juce::Label rateLabel;
    CustomComboBox rateComboBox; // 4, 6, 7, 9, 11, 14, 22, 44
    juce::Label reverseLabel;
    RoundedSlideSwitch reverseButton; // 0-1
    juce::Label speedLabel;
    CustomTextEditorInt speedTextEditor; // 1 - 99 (50 is normal?, below that is negative speed? above is positive?)
    juce::Label startCueLabel;
    CustomTextEditorInt32 startCueTextEditor;  // 0 - sampleEnd
    juce::Label stepsLabel;
    CustomComboBox stepsComboBox; // 0-7 (Off, - 2, - 3, - 4, - 5, - 6, - 7, - 8)
    juce::Label xfadeLabel;
    CustomTextEditorInt xfadeTextEditor; // 0 - 99
    juce::Label cueRandomLabel;
    RoundedSlideSwitch cueRandomButton;
    juce::Label cueStepLabel;
    RoundedSlideSwitch cueStepButton;

    LoopPointsView loopPointsView;

    // LOWER PANE
    WaveformDisplay waveformDisplay;
    std::array<juce::TextButton, 64> cueSetButtons;
    CvAssignEditor cvAssignEditor;

    NoArrowComboBoxLnF noArrowComboBoxLnF;

    juce::TextButton addCueSetButton;
    juce::TextButton deleteCueSetButton;

    juce::TextButton cueSetViewButton;
    juce::TextButton cvAssignViewButton;
    enum class LowerPaneView
    {
        cueSets,
        cvAssigns
    };
    LowerPaneView lowerPaneView { LowerPaneView::cueSets };
    int curCueSetIndex { 0 };

    void appendCueSet ();
    void configFileSelectorFromChannelSource ();
    void deleteCueSet (int cueSetIndex);
    int getUiValue (int internalValue);
    int getInternalValue (int uiValue);
    bool handleSampleAssignment (juce::String fileName);
    void initOutputComboBox ();
    void initializeCallbacks ();
    void setCueEditButtonsEnableState ();
    void setCurCue (int cueSetIndex);
    void setFilterEnableState ();
    void setLowerPaneView (LowerPaneView whichView);
    void setupComponents ();
    void updateLoopPointsView ();

    void attackDataChanged (int attack);
    void bitsDataChanged (int bits);
    void channelSourceDataChanged (uint8_t channelSourceIndex);
    void channelFlagsDataChanged (uint16_t channelFlags);
    void chokeDataChanged (int choke);
    void decayDataChanged (int decay);
    void endCueDataChanged (juce::int32 endCue);
    void eTrigDataChanged (int eTrig);
    void fileNameDataChanged (juce::String fileName);
    void filterTypeDataChanged (int filterType);
    void filterFrequencyDataChanged (int filterFrequency);
    void filterResonanceDataChanged (int filterResonance);
    void levelDataChanged (int level);
    void loopCueDataChanged (juce::int32 loopCue);
    void loopModeDataChanged (int loopMode);
    void quantDataChanged (int quant);
    void rateDataChanged (int rate);
    void reverseDataChanged (int reverse);
    void speedDataChanged (int speed);
    void startCueDataChanged (juce::int32 startCue);
    void stepsDataChanged (int steps);
    void xfadeDataChanged (int xfade);

    void attackUiChanged (int attack);
    void bitsUiChanged (int bits);
    void channelFlagsUiChanged (uint16_t channelFlags);
    void channelSourceUiChanged (uint8_t channelSourceIndex);
    void chokeUiChanged (int choke);
    void decayUiChanged (int decay);
    void endCueUiChanged (juce::int32 endCue);
    void eTrigUiChanged (int eTrig);
    void fileNameUiChanged (juce::String fileName);
    void filterTypeUiChanged (int filterType);
    void filterFrequencyUiChanged (int filterFrequency);
    void filterResonanceUiChanged (int filterResonance);
    void levelUiChanged (int level);
    void loopCueUiChanged (juce::int32 loopCue);
    void loopModeUiChanged (int loopMode);
    void quantUiChanged (int quant);
    void rateUiChanged (int rate);
    void reverseUiChanged (int reverse);
    void speedUiChanged (int speed);
    void startCueUiChanged (juce::int32 startCue);
    void stepsUiChanged (int steps);
    void xfadeUiChanged (int xfade);

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int, int) override;
//     void fileDragEnter (const juce::StringArray& files, int, int) override;
//     void fileDragMove (const juce::StringArray& files, int, int) override;
//     void fileDragExit (const juce::StringArray& files) override;

    void resized () override;
    void paint (juce::Graphics& g) override;
};
