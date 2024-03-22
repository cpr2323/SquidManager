#include "SquidMetaDataEditor.h"
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

SquidMetaDataEditorComponent::SquidMetaDataEditorComponent ()
{
    setOpaque (true);

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
        quantComboBox.addItem ("Harmionic Minor", quantId++);
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
    setupLabel (filterLabel, "FILTER", kMediumLabelSize, juce::Justification::centred);
    {
        auto filterId { 1 };
        filterComboBox.addItem ("Off", filterId++);
        filterComboBox.addItem ("Low Pass", filterId++);
        filterComboBox.addItem ("Band Pass", filterId++);
        filterComboBox.addItem ("Notch", filterId++);
        filterComboBox.addItem ("High Pass", filterId++);
    }
    filterComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    setupComboBox (filterComboBox, "Filter", [] () {}); // Off, LP, BP, NT, HP (0-4)
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
    setupLabel (sampleStartLabel, "START", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (sampleStartTextEditor, juce::Justification::centred, 0, "0123456789", "Start"); // 0 - sample length?
    setupLabel (loopLabel, "LOOP", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (loopTextEditor, juce::Justification::centred, 0, "0123456789", "Loop"); // 0 - sample length?, or sampleStart - sampleEnd
    setupLabel (sampleEndLabel, "END", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (sampleEndTextEditor, juce::Justification::centred, 0, "0123456789", "End"); // sampleStart - sample length

    startTimer (250);
}

SquidMetaDataEditorComponent::~SquidMetaDataEditorComponent ()
{
    loopModeComboBox.setLookAndFeel (nullptr);
    filterComboBox.setLookAndFeel (nullptr);
    quantComboBox.setLookAndFeel (nullptr);
    rateComboBox.setLookAndFeel (nullptr);
}

void SquidMetaDataEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);
    runtimeRootProperties.onSystemRequestedQuit = [this] ()
    {
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
//         audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
//         channelTabs.setCurrentTabIndex (0);
    };

//     PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
//     unEditedPresetProperties.wrap (presetManagerProperties.getPreset ("unedited"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
//     presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);

    squidMetaDataProperties.wrap (runtimeRootProperties.getValueTree (), SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::yes);

//     idDataChanged (presetProperties.getId ());
//     midiSetupDataChanged (presetProperties.getMidiSetup ());
//     nameDataChanged (presetProperties.getName ());
}

void SquidMetaDataEditorComponent::timerCallback ()
{

}

void SquidMetaDataEditorComponent::resized ()
{

    const auto columnWidth { 160 };
    const auto spaceBetweenColumns { 40 };

    const auto fieldWidth { columnWidth / 2 - 2 };
    auto xInitialOffSet { 15 };
    auto yInitialOffSet { 15 };

    auto xOffSet { xInitialOffSet };
    auto yOffset { kInitialYOffset };

    // column one
    bitsLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    bitsTextEditor.setBounds (bitsLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = bitsTextEditor.getBottom () + 3;
    rateLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    rateComboBox.setBounds (rateLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = rateComboBox.getBottom () + 3;
    speedLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    speedTextEditor.setBounds (speedLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = speedTextEditor.getBottom () + 3;
    quantLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    quantComboBox.setBounds (quantLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = quantComboBox.getBottom () + 3;
    filterLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    filterComboBox.setBounds (filterLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = filterComboBox.getBottom () + 3;
    filterFrequencyLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    filterFrequencyTextEditor.setBounds (filterFrequencyLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = filterFrequencyTextEditor.getBottom () + 3;
    filterResonanceLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    filterResonanceTextEditor.setBounds (filterResonanceLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = filterResonanceTextEditor.getBottom () + 3;
    levelLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    levelTextEditor.setBounds (levelLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);

    xOffSet += columnWidth + 10;
    yOffset = kInitialYOffset;
    // column two
    attackLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    attackTextEditor.setBounds (attackLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = attackTextEditor.getBottom () + 3;
    decayLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    decayTextEditor.setBounds (decayLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = decayTextEditor.getBottom () + 3;
    loopModeLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    loopModeComboBox.setBounds (loopModeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = loopModeComboBox.getBottom () + 3;
    xfadeLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    xfadeTextEditor.setBounds (xfadeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = xfadeTextEditor.getBottom () + 3;
    reverseButton.setBounds (xOffSet + 15, yOffset, fieldWidth * 2 - 25, kParameterLineHeight);
    yOffset = reverseButton.getBottom () + 3;
    sampleStartLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    sampleStartTextEditor.setBounds (sampleStartLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = sampleStartTextEditor.getBottom () + 3;
    loopLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    loopTextEditor.setBounds (loopLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = loopTextEditor.getBottom () + 3;
    sampleEndLabel.setBounds (xOffSet, yOffset, fieldWidth, kMediumLabelIntSize);
    sampleEndTextEditor.setBounds (sampleEndLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
}

void SquidMetaDataEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}
