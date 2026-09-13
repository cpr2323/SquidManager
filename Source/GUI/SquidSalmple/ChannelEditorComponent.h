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

    MenuButton toolsButton { "CHANNEL TOOLS", ActionButton::Size::small };
    // the sample length, painted beside the file chip: numbers in the dim ink,
    // their units in the muted one
    juce::String sampleSecondsText;
    juce::String sampleCountText;

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

    /*
        The sample file, as a chip. Only the name is shown: .wav is the only type
        the Squid plays, so the extension would say nothing. Click it to browse.

        A channel can play another channel's sample. Then the name is dimmed, since
        it cannot be changed from here, and the channel it comes from is named at
        the right hand end, as (C2), in normal ink.
    */
    class SampleFileChip : public FileSelectLabel
    {
    public:
        SampleFileChip () { setRepaintsOnMouseActivity (true); }

        void setFileName (const juce::String& fileName)
        {
            name = juce::File (fileName).getFileNameWithoutExtension ();
            setText (name, juce::NotificationType::dontSendNotification);
            repaint ();
        }

        // the channel the sample is taken from, or -1 when it is this channel's own
        void setSourceChannel (int newSourceChannelIndex)
        {
            sourceTag = newSourceChannelIndex < 0 ? juce::String () : "(C" + juce::String (newSourceChannelIndex + 1) + ")";
            repaint ();
        }

        int getIdealWidth () const
        {
            const auto chipFont { SquidType::fileName () };
            auto width { SquidPaint::textWidth (chipFont, name.isEmpty () ? juce::String (kNoSampleText) : name) + (kPadding * 2) + 2 };
            if (sourceTag.isNotEmpty ())
                width += kTagGap + SquidPaint::textWidth (chipFont, sourceTag);
            return width;
        }

        void paint (juce::Graphics& g) override
        {
            const auto area { getLocalBounds ().toFloat ().reduced (0.5f) };
            const auto enabled { isEnabled () };
            g.setColour (findColour (SquidColours::fieldBackground));
            g.fillRoundedRectangle (area, 2.0f);
            g.setColour (findColour (enabled && isMouseOver (true) ? SquidColours::accentDeep : SquidColours::outline));
            g.drawRoundedRectangle (area, 2.0f, 1.0f);

            auto content { getLocalBounds ().reduced (kPadding, 0) };
            const auto chipFont { SquidType::fileName () };
            g.setFont (chipFont);

            const auto ink { findColour (SquidColours::text) };
            if (sourceTag.isNotEmpty ())
            {
                g.setColour (ink);
                g.drawText (sourceTag, content.removeFromRight (SquidPaint::textWidth (chipFont, sourceTag)), juce::Justification::centredRight, false);
                content.removeFromRight (kTagGap);
            }

            if (name.isEmpty ())
            {
                g.setColour (findColour (SquidColours::textGhost));
                g.drawText (kNoSampleText, content, juce::Justification::centredLeft, true);
                return;
            }

            // Dimmed by mixing the text toward the field it sits on, rather than using
            // the dim ink. That ink is tuned to be legible on its own, so in the middle
            // of the background slider it comes out nearly as strong as normal text;
            // a mix keeps the dimmed name clearly weaker at every ground level.
            g.setColour (enabled ? ink : ink.interpolatedWith (findColour (SquidColours::fieldBackground), kDimmedMix));
            g.drawText (name, content, juce::Justification::centredLeft, true);
        }

    private:
        static constexpr const char* kNoSampleText { "no sample" };
        static constexpr int kPadding { 9 };
        static constexpr int kTagGap { 8 };
        // how far toward the field a dimmed name goes: at 0.45 it stays at least 1.8
        // times less contrasty than normal text, and above 2.4:1, across the slider
        static constexpr float kDimmedMix { 0.45f };
        juce::String name;
        juce::String sourceTag;

        // the outline FileSelectLabel draws is part of the chip here
        void paintOverChildren (juce::Graphics&) override {}
    };
    SampleFileChip sampleFileNameSelectLabel;
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
    TransportButton oneShotPlayButton { "ONCE", TransportButton::Glyph::play };
    TransportButton loopPlayButton { "LOOP", TransportButton::Glyph::loop };

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

    // set in resized, drawn in paint - the section cards, the hairlines that
    // separate the six parameter groups, and the bands inside the cue card
    juce::Rectangle<int> sampleCardBounds;
    juce::Rectangle<int> sampleMetaBounds;
    juce::Rectangle<int> parameterPanelBounds;
    juce::Rectangle<int> cueSetsCardBounds;
    juce::Rectangle<int> cueHeaderBounds;
    juce::Rectangle<int> topCueChipBounds;
    juce::Rectangle<int> bottomCueChipBounds;
    juce::Rectangle<int> cueToolBounds;
    juce::Rectangle<int> cvAssignCardBounds;
    std::array<int, 5> parameterDividerX { { 0, 0, 0, 0, 0 } };

    // LOWER PANE
    WaveformDisplay waveformDisplay;
    /*
        One chip per cue set. The current set is filled with the accent; a set
        that does not exist yet is dimmed and cannot be picked.
    */
    class CueSetButton : public juce::TextButton
    {
    public:
        void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
        {
            const auto area { getLocalBounds ().toFloat ().reduced (0.5f) };
            const auto enabled { isEnabled () };
            const auto current { enabled && getToggleState () };
            const auto hovered { enabled && ! current && (isMouseOver || isMouseDown) };

            g.setColour (findColour (current ? SquidColours::accent
                                             : (enabled ? SquidColours::buttonBackground : SquidColours::listBackground)));
            g.fillRoundedRectangle (area, 2.0f);
            g.setColour (findColour (current ? SquidColours::accentEdge
                                             : (hovered ? SquidColours::accentDeep
                                                        : (enabled ? SquidColours::outline : SquidColours::outlineDim))));
            g.drawRoundedRectangle (area, 2.0f, 1.0f);

            g.setFont (current ? SquidType::cueChipActive () : SquidType::cueChip ());
            g.setColour (findColour (current ? SquidColours::accentInk
                                             : (! enabled ? SquidColours::textGhost
                                                          : (hovered ? SquidColours::text : SquidColours::menuHeaderText))));
            g.drawText (getButtonText (), getLocalBounds (), juce::Justification::centred, false);
        }
    };

    /*
        Add and delete cue set, stacked beside the waveform. Deleting is the one
        destructive tool in the editor, so it warns in red under the pointer.
    */
    class CueToolButton : public juce::Button
    {
    public:
        CueToolButton (juce::String glyphText, bool isDestructive)
            : juce::Button ({}), glyph (std::move (glyphText)), destructive (isDestructive)
        {
        }

        void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
        {
            const auto enabled { isEnabled () };
            const auto hovered { enabled && (isMouseOver || isMouseDown) };
            g.fillAll (findColour (hovered ? (destructive ? SquidColours::dangerBackground : SquidColours::hoverBackground)
                                           : SquidColours::panelHeader));
            if (hovered)
            {
                g.setColour (findColour (SquidColours::accentDeep));
                g.drawRect (getLocalBounds (), 1);
            }
            g.setFont (SquidType::glyph ());
            g.setColour (findColour (! enabled ? SquidColours::textGhost
                                               : (hovered ? (destructive ? SquidColours::danger : SquidColours::text)
                                                          : SquidColours::textDim)));
            g.drawText (glyph, getLocalBounds (), juce::Justification::centred, false);
        }

    private:
        juce::String glyph;
        bool destructive;
    };
    std::array<CueSetButton, 64> cueSetButtons;
    CvAssignEditor cvAssignEditor;


    CueToolButton addCueSetButton { "+", false };
    CueToolButton deleteCueSetButton { juce::String (juce::CharPointer_UTF8 ("\xe2\x88\x92")), true };

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
