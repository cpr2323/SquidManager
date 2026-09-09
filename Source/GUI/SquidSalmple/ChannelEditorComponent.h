#pragma once

#include <JuceHeader.h>
#include "../Theme/SquidColourIds.h"
#include "../Theme/SquidLookAndFeel.h"
#include "CueSets/WaveformDisplay.h"
#include "CvAssigns/CvAssignEditor.h"
#include "LoopPoints/LoopPointsView.h"
#include "../../AppProperties.h"
#include "../../SquidSalmple/Audio/AudioPlayerProperties.h"
#include "../../SquidSalmple/EditManager/EditManager.h"
#include "../../SquidSalmple/SquidChannelProperties.h"
#include "oolib/GUI/CustomComboBox.h"
#include "oolib/GUI/CustomTextEditor.h"
#include "oolib/GUI/FileSelectLabel.h"
#include "oolib/GUI/NoArrowComboBoxLnF.h"
#include "oolib/GUI/RoundedSlideSwitch.h"

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

    // Names the groups the parameters are divided into. These carry the accent
    // colour rather than the default text colour, so they are refreshed in
    // applyExplicitColours when the palette changes.
    juce::Label levelEnvHeaderLabel;
    juce::Label filterHeaderLabel;
    juce::Label qualityHeaderLabel;
    juce::Label loopHeaderLabel;
    juce::Label triggerHeaderLabel;
    juce::Label cueTriggerHeaderLabel;
    juce::Label cuePointsHeaderLabel;
    juce::Label loopTunerHeaderLabel;
    juce::Label cvAssignHeaderLabel;

    /*
        A short colour bar beside a cue point field, drawn in the same colour as
        that point's marker on the waveform, so the number and the thing it moves
        read as one object. It resolves at paint time, so it follows the palette
        with no extra work.
    */
    class MarkerSwatch : public juce::Component
    {
    public:
        explicit MarkerSwatch (int theColourId) : colourId (theColourId) {}
        void paint (juce::Graphics& g) override
        {
            g.setColour (findColour (colourId));
            g.fillRoundedRectangle (getLocalBounds ().toFloat (), 1.0f);
        }
    private:
        int colourId;
    };
    MarkerSwatch startCueSwatch { SquidColours::markerStart };
    MarkerSwatch loopCueSwatch { SquidColours::markerLoop };
    MarkerSwatch endCueSwatch { SquidColours::markerEnd };

    // set in resized, drawn in paint - the panel outline and the hairlines that
    // separate the six parameter groups
    juce::Rectangle<int> parameterPanelBounds;
    std::array<int, 5> parameterDividerX { { 0, 0, 0, 0, 0 } };

    // LOWER PANE
    WaveformDisplay waveformDisplay;
    class CueSetButton : public juce::TextButton
    {
    public:
        CueSetButton ()
            : TextButton ()
        {
            applyColours ();
        }
        void enablementChanged () override
        {
            applyColours ();
        }
        void lookAndFeelChanged () override
        {
            juce::TextButton::lookAndFeelChanged ();
            applyColours ();
        }
    private:
        // The explicit colours are re-applied rather than set once, so that a
        // palette change reaches them; an explicit setColour always wins over
        // the LookAndFeel, so it has to be refreshed by hand.
        void applyColours ()
        {
            const auto background { isEnabled () ? SquidColours::buttonBackground
                                                 : SquidColours::windowBackground };
            setColour (juce::TextButton::ColourIds::buttonColourId, findColour (background));
            setColour (juce::TextButton::ColourIds::textColourOffId, findColour (SquidColours::text));
            setColour (juce::TextButton::ColourIds::buttonOnColourId, findColour (SquidColours::text));
            setColour (juce::TextButton::ColourIds::textColourOnId, findColour (SquidColours::windowBackground));
        }
    };
    std::array<CueSetButton, 64> cueSetButtons;
    CvAssignEditor cvAssignEditor;


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
    void updateWaveformDisplay ();

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
    void lookAndFeelChanged () override;
    void applyExplicitColours ();
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
};
