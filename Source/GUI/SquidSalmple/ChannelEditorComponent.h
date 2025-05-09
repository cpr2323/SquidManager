#pragma once

#include <JuceHeader.h>
#include "CueSets/WaveformDisplay.h"
#include "CvAssigns/CvAssignEditor.h"
#include "LoopPoints/LoopPointsView.h"
#include "../../AppProperties.h"
#include "../../SquidSalmple/Audio/AudioPlayerProperties.h"
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
    bool loadFile (juce::String sampleFileName);

    juce::ValueTree getChannelPropertiesVT ();

private:
    SquidChannelProperties squidChannelProperties;
    AudioPlayerProperties audioPlayerProperties;
    AppProperties appProperties;
    EditManager* editManager { nullptr };
    bool draggingFiles { false };
    bool supportedFile { false };
    juce::String dropMsg;
    juce::String dropDetails;

    std::unique_ptr<juce::AlertWindow> renameAlertWindow;

    juce::TextButton toolsButton;
    juce::Label sampleLengthLabel;

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
    CustomTextEditorInt32 endCueTextEditor;
    juce::Label eTrigLabel;
    CustomComboBox eTrigComboBox; // Off, > 1, > 2, > 3, > 4, > 5, > 6, > 7, > 8, On
    juce::Label sampleFileNameLabel;
    FileSelectLabel sampleFileNameSelectLabel;
    juce::Label filterTypeLabel;
    CustomComboBox filterTypeComboBox; // Off, LP, BP, NT, HP (0-4)
    juce::Label filterFrequencyLabel;
    CustomTextEditorInt filterFrequencyTextEditor; // 1-99?
    juce::Label filterResonanceLabel;
    CustomTextEditorInt filterResonanceTextEditor; // 1-99?
    juce::Label levelLabel;
    CustomTextEditorInt levelTextEditor; // 1-99
    juce::Label loopCueLabel;
    CustomTextEditorInt32 loopCueTextEditor;
    juce::Label loopModeLabel;
    CustomComboBox loopModeComboBox; // none, normal, zigZag, gate, zigZagGate (0-4)
    juce::Label outputLabel;
    CustomComboBox outputComboBox; // chan 1-4 = 1-2,3-4 / chan 5-8 = 5-6,7-8
    juce::Label quantLabel;
    CustomComboBox quantComboBox; // 0-14 (Off, 12, OT, MA, mi, Hm, PM, Pm, Ly, Ph, Jp, P5, C1, C4, C5)
    juce::Label pitchShiftLabel;
    CustomTextEditorDouble pitchShiftTextEditor; // 0-4000 displayed as 0.00 to 4.00
    juce::Label rateLabel;
    CustomComboBox rateComboBox; // 4, 6, 7, 9, 11, 14, 22, 44
    juce::Label reverseLabel;
    RoundedSlideSwitch reverseButton; // 0-1
    juce::Label speedLabel;
    CustomTextEditorInt speedTextEditor; // 1 - 99 (50 is normal?, below that is negative speed? above is positive?)
    juce::Label startCueLabel;
    CustomTextEditorInt32 startCueTextEditor;
    juce::Label stepsLabel;
    CustomComboBox stepsComboBox; // 0-7 (Off, - 2, - 3, - 4, - 5, - 6, - 7, - 8)
    juce::Label xfadeLabel;
    CustomTextEditorInt xfadeTextEditor; // 0 - 99
    juce::Label cueRandomLabel;
    RoundedSlideSwitch cueRandomButton;
    juce::Label cueStepLabel;
    RoundedSlideSwitch cueStepButton;

    LoopPointsView loopPointsView;
    juce::TextButton oneShotPlayButton;
    juce::TextButton loopPlayButton;

    // LOWER PANE
    WaveformDisplay waveformDisplay;
    class CueSetButton : public juce::TextButton
    {
    public:
        CueSetButton ()
            : TextButton ()
        {
            setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgrey);
            setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
            setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::white);
            setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
        }
        void enablementChanged () override
        {
            if (isEnabled ())
            {
                setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgrey);
                setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
            }
            else
            {
                setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
                setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
            }
        }
    };
    std::array<CueSetButton, 64> cueSetButtons;
    CvAssignEditor cvAssignEditor;

    NoArrowComboBoxLnF noArrowComboBoxLnF;

    class CueEditButtonLnF : public juce::LookAndFeel_V4
    {
    public:
        juce::Font getTextButtonFont (juce::TextButton&, int /*buttonHeight*/) override
        {
            return juce::Font (11);
        }
    };
    CueEditButtonLnF cueEditButtonLnF;

    juce::TextButton addCueSetButton;
    juce::TextButton deleteCueSetButton;

    int curCueSetIndex { 0 };

    void appendCueSet ();
    void configFileSelectorFromChannelSource ();
    void deleteCueSet (int cueSetIndex);
    int getFilterFrequencyUiValue (int internalValue);
    int getFilterFrequencyInternalValue (int uiValue);
    int getUiValue (int internalValue);
    int getInternalValue (int uiValue);
    void filesDroppedOnCueSetEditor (const juce::StringArray& files, juce::String outputFileName, juce::ValueTree cueSets);
    bool handleSampleAssignment (const juce::StringArray& fileNames);
    void initOutputComboBox ();
    void initializeCallbacks ();
    void setCueEditButtonsEnableState ();
    void setCurCue (int cueSetIndex);
    void setFilterEnableState ();
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
    void filterTypeDataChanged (int filterType);
    void filterFrequencyDataChanged (int filterFrequency);
    void filterResonanceDataChanged (int filterResonance);
    void levelDataChanged (int level);
    void loopCueDataChanged (juce::int32 loopCue);
    void loopModeDataChanged (int loopMode);
    void quantDataChanged (int quant);
    void pitchShiftDataChanged (int pitch);
    void rateDataChanged (int rate);
    void reverseDataChanged (int reverse);
    void sampleFileNameDataChanged (juce::String sampleFileName);
    void speedDataChanged (int speed);
    void startCueDataChanged (juce::int32 startCue);
    void stepsDataChanged (int steps);
    void xfadeDataChanged (int xfade);

    void attackUiChanged (int attack);
    void bitsUiChanged (int bits);
    //void channelFlagsUiChanged (uint16_t channelFlags);
    void channelSourceUiChanged (uint8_t channelSourceIndex);
    void chokeUiChanged (int choke);
    void decayUiChanged (int decay);
    void endCueUiChanged (juce::int32 endCue);
    void eTrigUiChanged (int eTrig);
    //void fileNameUiChanged (juce::String fileName);
    void filterTypeUiChanged (int filterType);
    void filterFrequencyUiChanged (int filterFrequency);
    void filterResonanceUiChanged (int filterResonance);
    void levelUiChanged (int level);
    void loopCueUiChanged (juce::int32 loopCue);
    void loopModeUiChanged (int loopMode);
    void outputUiChanged (int selectedIndex);
    void quantUiChanged (int quant);
    void pitchShiftUiChanged (float pitchShift);
    void rateUiChanged (int rate);
    void reverseUiChanged (int reverse);
    void speedUiChanged (int speed);
    void startCueUiChanged (juce::int32 startCue);
    void stepsUiChanged (int steps);
    void xfadeUiChanged (int xfade);

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int, int) override;
    void fileDragEnter (const juce::StringArray& files, int, int) override;
    void fileDragExit (const juce::StringArray& files) override;

    void resized () override;
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
};
