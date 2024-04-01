#include "SquidMetaDataEditor.h"
#include "../../SquidSalmple/SquidMetaDataReader.h"
#include "../../Utility/PersistentRootProperties.h"

const auto kLargeLabelSize { 20.0f };
const auto kMediumLabelSize { 14.0f };
const auto kSmallLabelSize { 12.0f };
const auto kLargeLabelIntSize { static_cast<int> (kLargeLabelSize) };
const auto kMediumLabelIntSize { static_cast<int> (kMediumLabelSize) };
const auto kSmallLabelIntSize { static_cast<int> (kSmallLabelSize) };

const auto kParameterLineHeight { 20 };
const auto kInterControlYOffset { 2 };
const auto kInitialYOffset { 5 };

const auto kScaleMax { 65500. };

SquidMetaDataEditorComponent::SquidMetaDataEditorComponent ()
{
    setOpaque (true);
    setupComponents ();
    startTimer (250); // 
}

SquidMetaDataEditorComponent::~SquidMetaDataEditorComponent ()
{
    loopModeComboBox.setLookAndFeel (nullptr);
    filterTypeComboBox.setLookAndFeel (nullptr);
    quantComboBox.setLookAndFeel (nullptr);
    rateComboBox.setLookAndFeel (nullptr);
}

void SquidMetaDataEditorComponent::setupComponents ()
{
    auto setupLabel = [this] (juce::Label& label, juce::String text, float fontSize, juce::Justification justification)
    {
        const auto textColor { juce::Colours::white };
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setColour (juce::Label::ColourIds::textColourId, textColor);
        label.setFont (label.getFont ().withPointHeight (fontSize));
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    auto setupTextEditor = [this] (juce::TextEditor& textEditor, juce::Justification justification, int maxLen, juce::String validInputCharacters, juce::String parameterName)
    {
        textEditor.setJustification (justification);
        textEditor.setIndents (1, 0);
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        textEditor.setColour (juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::black);
        //textEditor.setTooltip (parameterToolTipData.getToolTip ("Channel", parameterName));
        addAndMakeVisible (textEditor);
    };
    auto setupComboBox = [this] (juce::ComboBox& comboBox, juce::String parameterName, std::function<void ()> onChangeCallback)
    {
        jassert (onChangeCallback != nullptr);
        comboBox.setColour (juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::black);
        //comboBox.setTooltip (parameterToolTipData.getToolTip ("Channel", parameterName));
        comboBox.onChange = onChangeCallback;
        addAndMakeVisible (comboBox);
    };
    auto setupButton = [this] (juce::TextButton& textButton, juce::String text, juce::String parameterName, std::function<void ()> onClickCallback)
    {
        textButton.setButtonText (text);
        textButton.setClickingTogglesState (true);
        textButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, textButton.findColour (juce::TextButton::ColourIds::buttonOnColourId).brighter (0.5));
        //textButton.setTooltip (parameterToolTipData.getToolTip ("Channel", parameterName));
        textButton.onClick = onClickCallback;
        addAndMakeVisible (textButton);
    };
    // BITS
    setupLabel (bitsLabel, "BITS", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (bitsTextEditor, juce::Justification::centred, 0, "0123456789", "Bits"); // 1-16
    // RATE
    setupLabel (rateLabel, "RATE", kMediumLabelSize, juce::Justification::centred);
    rateComboBox.addItem ("4", 4);
    rateComboBox.addItem ("6", 6);
    rateComboBox.addItem ("7", 7);
    rateComboBox.addItem ("9", 9);
    rateComboBox.addItem ("11", 11);
    rateComboBox.addItem ("14", 14);
    rateComboBox.addItem ("22", 22);
    rateComboBox.addItem ("44", 44);
    rateComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    setupComboBox (rateComboBox, "Rate", [] () {}); // 4,6,7,9,11,14,22,44
    // SPEED
    setupLabel (speedLabel, "SPEED", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (speedTextEditor, juce::Justification::centred, 0, "0123456789", "Speed"); // 1 - 99 (50 is normal, below that is negative speed? above is positive?)
    setupLabel (quantLabel, "QUANT", kMediumLabelSize, juce::Justification::centred);
    {
        auto quantId { 1 };
        quantComboBox.addItem ("Off", quantId++);
        quantComboBox.addItem ("Chromatic 12", quantId++);
        quantComboBox.addItem ("Full Octave", quantId++);
        quantComboBox.addItem ("Major", quantId++);
        quantComboBox.addItem ("Minor", quantId++);
        quantComboBox.addItem ("Harmonic Minor", quantId++);
        quantComboBox.addItem ("Pentatonic Major", quantId++);
        quantComboBox.addItem ("Pentatonic Minor", quantId++);
        quantComboBox.addItem ("Lydian", quantId++);
        quantComboBox.addItem ("Phrygian", quantId++);
        quantComboBox.addItem ("Japanese", quantId++);
        quantComboBox.addItem ("Root & Fifth", quantId++);
        quantComboBox.addItem ("I Chord", quantId++);
        quantComboBox.addItem ("IV Chord", quantId++);
        quantComboBox.addItem ("VI Chord", quantId++);
    }
    quantComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    setupComboBox (quantComboBox, "Quantize", [] () {}); // 0-14 (Off, 12, OT, MA, mi, Hm, PM, Pm, Ly, Ph, Jp, P5, C1, C4, C5)
    setupLabel (filterTypeLabel, "FILTER", kMediumLabelSize, juce::Justification::centred);
    {
        auto filterId { 1 };
        filterTypeComboBox.addItem ("Off", filterId++);
        filterTypeComboBox.addItem ("Low Pass", filterId++);
        filterTypeComboBox.addItem ("Band Pass", filterId++);
        filterTypeComboBox.addItem ("Notch", filterId++);
        filterTypeComboBox.addItem ("High Pass", filterId++);
    }
    filterTypeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    setupComboBox (filterTypeComboBox, "Filter", [] () {}); // Off, LP, BP, NT, HP (0-4)
    setupLabel (filterFrequencyLabel, "FREQ", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (filterFrequencyTextEditor, juce::Justification::centred, 0, "0123456789", "Frequency"); // 1-99?
    setupLabel (filterResonanceLabel, "RESO", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (filterResonanceTextEditor, juce::Justification::centred, 0, "0123456789", "Resonance"); // 1-99?
    setupLabel (levelLabel, "LEVEL", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (levelTextEditor, juce::Justification::centred, 0, "0123456789", "Level"); // 1-99
    setupLabel (attackLabel, "ATTACK", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (attackTextEditor, juce::Justification::centred, 0, "0123456789", "Attack"); // 0-99
    setupLabel (decayLabel, "DECAY", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (decayTextEditor, juce::Justification::centred, 0, "0123456789", "Decay"); // 0-99
    setupLabel (loopModeLabel, "LOOP MODE", kMediumLabelSize, juce::Justification::centred);
    {
        auto loopId { 1 };
        loopModeComboBox.addItem ("None", loopId++);
        loopModeComboBox.addItem ("Normal", loopId++);
        loopModeComboBox.addItem ("ZigZag", loopId++);
        loopModeComboBox.addItem ("Gate", loopId++);
        loopModeComboBox.addItem ("ZigZag Gate", loopId++);
    }
    loopModeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    setupComboBox (loopModeComboBox, "LoopMode", [] () {}); // none, normal, zigZag, gate, zigZagGate (0-4)
    setupLabel (xfadeLabel, "XFADE", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (xfadeTextEditor, juce::Justification::centred, 0, "0123456789", "XFade"); // 0 -99
    setupButton (reverseButton, "REVERSE", "Reverse", [] () {}); // 0-1
    setupLabel (startCueLabel, "START", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (startCueTextEditor, juce::Justification::centred, 0, "0123456789", "Start"); // 0 - sample length?
    setupLabel (loopCueLabel, "LOOP", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (loopCueTextEditor, juce::Justification::centred, 0, "0123456789", "Loop"); // 0 - sample length?, or sampleStart - sampleEnd
    setupLabel (endCueLabel, "END", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (endCueTextEditor, juce::Justification::centred, 0, "0123456789", "End"); // sampleStart - sample length

    loadButton.setButtonText ("LOAD");
    loadButton.onClick = [this] ()
    {
        fileChooser.reset (new juce::FileChooser ("Please select the file to load...", {}, ""));
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            {
                auto wavFileToLoad { fc.getURLResults () [0].getLocalFile () };
                appProperties.addRecentlyUsedFile (wavFileToLoad.getFullPathName ());

                // TODO - check for import errors and handle accordingly
                SquidMetaDataReader squidMetaDataReader;
                SquidMetaDataProperties loadedSquidMetaDataProperties { squidMetaDataReader.read (wavFileToLoad),
                                                                        SquidMetaDataProperties::WrapperType::owner,
                                                                        SquidMetaDataProperties::EnableCallbacks::no };

                auto oldCueSetsVT { squidMetaDataProperties.getValueTree ().getChildWithName (SquidMetaDataProperties::CueSetListTypeId) };
                jassert (oldCueSetsVT.isValid ());
                oldCueSetsVT.removeAllChildren (nullptr);
                squidMetaDataProperties.getValueTree ().copyPropertiesFrom (loadedSquidMetaDataProperties.getValueTree (), nullptr);
                auto newCueSetsVT { loadedSquidMetaDataProperties.getValueTree ().getChildWithName (SquidMetaDataProperties::CueSetListTypeId) };
                jassert (newCueSetsVT.isValid ());
                ValueTreeHelpers::forEachChildOfType (newCueSetsVT, SquidMetaDataProperties::CueSetTypeId, [this, &oldCueSetsVT] (juce::ValueTree cueSetVT)
                {
                    oldCueSetsVT.addChild (cueSetVT.createCopy (), -1, nullptr);
                    return true;
                });
                initCueSets ();
                setCurCue (0);
            }
        }, nullptr);
    };
    addAndMakeVisible (loadButton);
    waveformDisplay.onStartPointChange = [this] (int startPoint)
    {
        squidMetaDataProperties.setCuePoints (curCueSet, startPoint, squidMetaDataProperties.getLoopCueSet (curCueSet), squidMetaDataProperties.getEndCueSet (curCueSet));
    };
    waveformDisplay.onLoopPointChange = [this] (int loopPoint)
    {
        squidMetaDataProperties.setCuePoints (curCueSet, squidMetaDataProperties.getStartCueSet (curCueSet), loopPoint, squidMetaDataProperties.getEndCueSet (curCueSet));
    };
    waveformDisplay.onEndPointChange = [this] (int endPoint)
    {
        squidMetaDataProperties.setCuePoints (curCueSet, squidMetaDataProperties.getStartCueSet (curCueSet), squidMetaDataProperties.getLoopCueSet (curCueSet), endPoint);
    };
    addAndMakeVisible (waveformDisplay);
    for (auto cueSetIndex { 0 }; cueSetIndex < cueSetButtons.size (); ++cueSetIndex)
    {
        cueSetButtons [cueSetIndex].setButtonText (juce::String (cueSetIndex + 1));
        cueSetButtons [cueSetIndex].setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
        cueSetButtons [cueSetIndex].setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::white);
        cueSetButtons [cueSetIndex].setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
        cueSetButtons [cueSetIndex].setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
        cueSetButtons [cueSetIndex].onClick = [this, cueSetIndex] () { setCurCue (cueSetIndex); };
        addAndMakeVisible (cueSetButtons [cueSetIndex]);
    }
}

void SquidMetaDataEditorComponent::setCurCue (int cueSetIndex)
{
    if (squidMetaDataProperties.getNumCues () == 0)
        return;
    jassert (cueSetIndex < squidMetaDataProperties.getNumCues());

    cueSetButtons [curCueSet].setToggleState (false, juce::NotificationType::dontSendNotification);
    curCueSet = cueSetIndex;
    cueSetButtons [curCueSet].setToggleState (true, juce::NotificationType::dontSendNotification);
    waveformDisplay.setCuePoints (squidMetaDataProperties.getStartCueSet (curCueSet), squidMetaDataProperties.getLoopCueSet (curCueSet), squidMetaDataProperties.getEndCueSet (curCueSet));
}

void SquidMetaDataEditorComponent::initCueSets ()
{
    const auto numCueSets { squidMetaDataProperties.getNumCues () };
    for (auto cueSetButtonIndex { 0 }; cueSetButtonIndex < cueSetButtons.size (); ++cueSetButtonIndex)
        cueSetButtons [cueSetButtonIndex].setEnabled (cueSetButtonIndex < numCueSets);

};

void SquidMetaDataEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);
    runtimeRootProperties.onSystemRequestedQuit = [this] ()
    {
            // TODO - add code to check if data needs to be saved before exiting
//         runtimeRootProperties.setPreferredQuitState (RuntimeRootProperties::QuitState::idle, false);
//         overwritePresetOrCancel ([this] ()
//         {
//             juce::MessageManager::callAsync ([this] () { runtimeRootProperties.setQuitState (RuntimeRootProperties::QuitState::now, false); });
//         }, [this] ()
//         {
//             // do nothing
//         });
    };

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFileChange = [this] (juce::String fileName)
    {
//         //DebugLog ("Assimil8orEditorComponent", "Assimil8orEditorComponent::init/appProperties.onMostRecentFileChange: " + fileName);
//         //dumpStacktrace (-1, [this] (juce::String logLine) { DebugLog ("Assimil8orEditorComponent", logLine); });
        waveformDisplay.init (fileName);
    };

//     PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
//     unEditedPresetProperties.wrap (presetManagerProperties.getPreset ("unedited"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
//     presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);

    squidMetaDataProperties.wrap (runtimeRootProperties.getValueTree (), SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::yes);

    initCueSets ();
    // TODO - is 'cur cue' stored in the metadata, should I use it to initialize from and also update when changed in the UI?
    setCurCue (0);

    // put initial data into the UI
    attackDataChanged (squidMetaDataProperties.getAttack());
    bitsDataChanged (squidMetaDataProperties.getBits ());
    decayDataChanged (squidMetaDataProperties.getDecay ());
    endCueDataChanged (squidMetaDataProperties.getEndCue ());
    filterTypeDataChanged (squidMetaDataProperties.getFilterType ());
    filterFrequencyDataChanged (squidMetaDataProperties.getFilterFrequency ());
    filterResonanceDataChanged (squidMetaDataProperties.getFilterResonance ());
    levelDataChanged (squidMetaDataProperties.getLevel ());
    loopCueDataChanged (squidMetaDataProperties.getLoopCue ());
    loopModeDataChanged (squidMetaDataProperties.getLoopMode ());
    quantDataChanged (squidMetaDataProperties.getQuant ());
    rateDataChanged (squidMetaDataProperties.getRate ());
    reverseDataChanged (squidMetaDataProperties.getReverse ());
    speedDataChanged (squidMetaDataProperties.getSpeed ());
    startCueDataChanged (squidMetaDataProperties.getStartCue ());
    xfadeDataChanged (squidMetaDataProperties.getXfade ());

    initializeCallbacks ();
}

void SquidMetaDataEditorComponent::initializeCallbacks ()
{
    jassert (squidMetaDataProperties.isValid ());
    squidMetaDataProperties.onAttackChange = [this] (int attack) { attackDataChanged (attack); };
    squidMetaDataProperties.onBitsChange = [this] (int bits) { bitsDataChanged (bits); };
    squidMetaDataProperties.onDecayChange = [this] (int decay) { decayDataChanged (decay); };
    squidMetaDataProperties.onEndCueChange = [this] (int endCue) { endCueDataChanged (endCue); };
    squidMetaDataProperties.onEndCueSetChange = [this] (int cueIndex, int endCue) { if (cueIndex == curCueSet) endCueDataChanged (endCue); };
    squidMetaDataProperties.onFilterTypeChange = [this] (int filter) { filterTypeDataChanged (filter); };
    squidMetaDataProperties.onFilterFrequencyChange = [this] (int filterFrequency) { filterFrequencyDataChanged (filterFrequency); };
    squidMetaDataProperties.onFilterResonanceChange = [this] (int filterResonance) { filterResonanceDataChanged (filterResonance); };
    squidMetaDataProperties.onLevelChange = [this] (int level) { levelDataChanged (level); };
    squidMetaDataProperties.onLoopCueChange = [this] (int loopCue) { loopCueDataChanged (loopCue); };
    squidMetaDataProperties.onLoopCueSetChange = [this] (int cueIndex, int loopCue) { if (cueIndex == curCueSet) loopCueDataChanged (loopCue); };
    squidMetaDataProperties.onLoopModeChange = [this] (int loopMode) { loopModeDataChanged (loopMode); };
    squidMetaDataProperties.onQuantChange = [this] (int quant) { quantDataChanged (quant); };
    squidMetaDataProperties.onRateChange = [this] (int rate) { rateDataChanged (rate); };
    squidMetaDataProperties.onReverseChange = [this] (int reverse) { reverseDataChanged (reverse); };
    squidMetaDataProperties.onSpeedChange = [this] (int speed) { speedDataChanged (speed); };
    squidMetaDataProperties.onStartCueChange = [this] (int startCue) { startCueDataChanged (startCue); };
    squidMetaDataProperties.onStartCueSetChange = [this] (int cueIndex, int startCue) { if (cueIndex == curCueSet) startCueDataChanged (startCue); };
    squidMetaDataProperties.onXfadeChange = [this] (int xfade) { xfadeDataChanged (xfade); };
}

// Data Changed functions
void SquidMetaDataEditorComponent::attackDataChanged (int attack)
{
    attackTextEditor.setText (juce::String(static_cast <int> (attack / kScaleMax * 100.)), false);
}

void SquidMetaDataEditorComponent::bitsDataChanged (int bits)
{
    bitsTextEditor.setText (juce::String (bits), false);
}

void SquidMetaDataEditorComponent::decayDataChanged (int decay)
{
    decayTextEditor.setText (juce::String (static_cast <int> (decay / kScaleMax * 100.)), false);
}

void SquidMetaDataEditorComponent::endCueDataChanged (int endCue)
{
    endCueTextEditor.setText (juce::String (endCue), false);
    if (curCueSet == 0)
        waveformDisplay.setCuePoints (squidMetaDataProperties.getStartCueSet (curCueSet), squidMetaDataProperties.getLoopCueSet (curCueSet), endCue);
}

void SquidMetaDataEditorComponent::filterTypeDataChanged (int filterType)
{
    filterTypeComboBox.setSelectedItemIndex (filterType, juce::NotificationType::dontSendNotification);
}

void SquidMetaDataEditorComponent::filterFrequencyDataChanged (int filterFrequency)
{
    filterFrequencyTextEditor.setText (juce::String (filterFrequency), false);
}

void SquidMetaDataEditorComponent::filterResonanceDataChanged (int filterResonance)
{
    filterResonanceTextEditor.setText (juce::String (static_cast <int> (filterResonance / kScaleMax * 100.)), false);
}

void SquidMetaDataEditorComponent::levelDataChanged (int level)
{
    levelTextEditor.setText (juce::String (level), false);
}

void SquidMetaDataEditorComponent::loopCueDataChanged (int loopCue)
{
    loopCueTextEditor.setText (juce::String (loopCue), false);
    if (curCueSet == 0)
        waveformDisplay.setCuePoints (squidMetaDataProperties.getStartCueSet (curCueSet), loopCue, squidMetaDataProperties.getEndCueSet (curCueSet));
}

void SquidMetaDataEditorComponent::loopModeDataChanged (int loopMode)
{
    loopModeComboBox.setSelectedItemIndex (loopMode, juce::NotificationType::dontSendNotification);
}

void SquidMetaDataEditorComponent::quantDataChanged (int quant)
{
    quantComboBox.setSelectedItemIndex (quant, juce::NotificationType::dontSendNotification);
}

void SquidMetaDataEditorComponent::rateDataChanged (int rate)
{
    rateComboBox.setSelectedItemIndex (rate, juce::NotificationType::dontSendNotification);
}

void SquidMetaDataEditorComponent::reverseDataChanged (int reverse)
{
    reverseButton.setToggleState (reverse == 1, juce::NotificationType::dontSendNotification);
}

void SquidMetaDataEditorComponent::speedDataChanged (int speed)
{
    speedTextEditor.setText (juce::String (speed), juce::NotificationType::dontSendNotification);
}

void SquidMetaDataEditorComponent::startCueDataChanged (int startCue)
{
    startCueTextEditor.setText (juce::String (startCue), juce::NotificationType::dontSendNotification);
    if (curCueSet == 0)
        waveformDisplay.setCuePoints (startCue, squidMetaDataProperties.getLoopCueSet (curCueSet), squidMetaDataProperties.getEndCueSet (curCueSet));
}

void SquidMetaDataEditorComponent::xfadeDataChanged (int xfade)
{
    xfadeTextEditor.setText (juce::String (xfade), juce::NotificationType::dontSendNotification);
}

// UI Changed functions
void SquidMetaDataEditorComponent::attackUiChanged (int attack)
{
    const auto newAttackValue { static_cast<int> (attack / 100. * kScaleMax) };
    squidMetaDataProperties.setAttack (newAttackValue, false);
}

void SquidMetaDataEditorComponent::bitsUiChanged (int bits)
{
    squidMetaDataProperties.setBits (bits, false);
}

void SquidMetaDataEditorComponent::decayUiChanged (int decay)
{
    const auto newDecayValue { static_cast<int> (decay / 100. * kScaleMax) };
    squidMetaDataProperties.setDecay (newDecayValue, false);
}

void SquidMetaDataEditorComponent::endCueUiChanged (int endCue)
{
    squidMetaDataProperties.setEndCue (endCue, false);
}

void SquidMetaDataEditorComponent::filterTypeUiChanged (int filter)
{
    squidMetaDataProperties.setFilterType (filter, false);
}

void SquidMetaDataEditorComponent::filterFrequencyUiChanged (int filterFrequency)
{
    squidMetaDataProperties.setFilterFrequency (filterFrequency, false);
}

void SquidMetaDataEditorComponent::filterResonanceUiChanged (int filterResonance)
{
    const auto newResonanceValue { static_cast<int> (filterResonance/ 100. * kScaleMax) };
    squidMetaDataProperties.setFilterResonance (newResonanceValue, false);
}

void SquidMetaDataEditorComponent::levelUiChanged (int level)
{
    squidMetaDataProperties.setLevel (level, false);
}

void SquidMetaDataEditorComponent::loopCueUiChanged (int loopCue)
{
    squidMetaDataProperties.setLoopCue (loopCue, false);
}

void SquidMetaDataEditorComponent::loopModeUiChanged (int loopMode)
{
    squidMetaDataProperties.setLoopMode (loopMode, false);
}

void SquidMetaDataEditorComponent::quantUiChanged (int quant)
{
    squidMetaDataProperties.setQuant (quant, false);
}

void SquidMetaDataEditorComponent::rateUiChanged (int rate)
{
    squidMetaDataProperties.setRate (rate, false);
}

void SquidMetaDataEditorComponent::reverseUiChanged (int reverse)
{
    squidMetaDataProperties.setReverse (reverse, false);
}

void SquidMetaDataEditorComponent::speedUiChanged (int speed)
{
    squidMetaDataProperties.setSpeed (speed, false);
}

void SquidMetaDataEditorComponent::startCueUiChanged (int startCue)
{
    squidMetaDataProperties.setStartCue (startCue, false);
}

void SquidMetaDataEditorComponent::xfadeUiChanged (int xfade)
{
    squidMetaDataProperties.setXfade (xfade, false);
}

void SquidMetaDataEditorComponent::timerCallback ()
{
    // check if data has changed
}

void SquidMetaDataEditorComponent::resized ()
{
    const auto columnWidth { 160 };
    const auto spaceBetweenColumns { 40 };

    const auto fieldWidth { columnWidth / 2 - 2 };
    auto xInitialOffSet { 15 };
    auto yInitialOffSet { 15 };

    auto xOffset { xInitialOffSet };
    auto yOffset { kInitialYOffset };

    // column one
    bitsLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    bitsTextEditor.setBounds (bitsLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = bitsTextEditor.getBottom () + 3;
    rateLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    rateComboBox.setBounds (rateLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = rateComboBox.getBottom () + 3;
    speedLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    speedTextEditor.setBounds (speedLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = speedTextEditor.getBottom () + 3;
    quantLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    quantComboBox.setBounds (quantLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = quantComboBox.getBottom () + 3;
    filterTypeLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    filterTypeComboBox.setBounds (filterTypeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = filterTypeComboBox.getBottom () + 3;
    filterFrequencyLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    filterFrequencyTextEditor.setBounds (filterFrequencyLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = filterFrequencyTextEditor.getBottom () + 3;
    filterResonanceLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    filterResonanceTextEditor.setBounds (filterResonanceLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = filterResonanceTextEditor.getBottom () + 3;
    levelLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    levelTextEditor.setBounds (levelLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);

    xOffset += columnWidth + 10;
    yOffset = kInitialYOffset;
    // column two
    attackLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    attackTextEditor.setBounds (attackLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = attackTextEditor.getBottom () + 3;
    decayLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    decayTextEditor.setBounds (decayLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = decayTextEditor.getBottom () + 3;
    loopModeLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    loopModeComboBox.setBounds (loopModeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = loopModeComboBox.getBottom () + 3;
    xfadeLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    xfadeTextEditor.setBounds (xfadeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = xfadeTextEditor.getBottom () + 3;
    reverseButton.setBounds (xOffset + 15, yOffset, fieldWidth * 2 - 25, kParameterLineHeight);
    yOffset = reverseButton.getBottom () + 3;
    startCueLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    startCueTextEditor.setBounds (startCueLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = startCueTextEditor.getBottom () + 3;
    loopCueLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    loopCueTextEditor.setBounds (loopCueLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = loopCueTextEditor.getBottom () + 3;
    endCueLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    endCueTextEditor.setBounds (endCueLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);


    xOffset += columnWidth + 10;
    yOffset = kInitialYOffset;
    loadButton.setBounds (xOffset, yOffset, fieldWidth, kParameterLineHeight);

    const auto kHeightOfCueSetButton { 20 };
    const auto kWidthOfCueSetButton { 30 };
    const auto kWidthOfWaveformEditor { 960 };
    xOffset = xInitialOffSet;
    yOffset = levelTextEditor.getBottom () + 10 + kHeightOfCueSetButton;
    waveformDisplay.setBounds (xOffset, yOffset, kWidthOfWaveformEditor, getHeight () - yOffset - (kHeightOfCueSetButton * 2));
    for (auto cueSetIndex { 0 }; cueSetIndex < cueSetButtons.size () / 2; ++cueSetIndex)
    {
        const auto buttonX { xOffset + cueSetIndex * kWidthOfCueSetButton };
        cueSetButtons [cueSetIndex].setBounds (buttonX, waveformDisplay.getY () - kHeightOfCueSetButton, kWidthOfCueSetButton, kHeightOfCueSetButton);
        cueSetButtons [cueSetIndex + 32].setBounds (buttonX, waveformDisplay.getBottom (), kWidthOfCueSetButton, kHeightOfCueSetButton);
    }
}

void SquidMetaDataEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}
