#include "ChannelEditorComponent.h"
#include "../../SystemServices.h"
#include "../../SquidSalmple/Metadata/SquidSalmpleDefs.h"
#include "../../Utility/PersistentRootProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

const auto kLargeLabelSize { 20.0f };
const auto kMediumLabelSize { 14.0f };
const auto kSmallLabelSize { 12.0f };
const auto kLargeLabelIntSize { static_cast<int> (kLargeLabelSize) };
const auto kMediumLabelIntSize { static_cast<int> (kMediumLabelSize) };
const auto kSmallLabelIntSize { static_cast<int> (kSmallLabelSize) };

const auto kParameterLineHeight { 20 };
const auto kInterControlYOffset { 2 };
const auto kInitialYOffset { 5 };

static const auto kScaleMax { 65535. };
static const auto kScaleStep { kScaleMax / 100 };

uint32_t byteOffsetToSampleOffset (uint32_t byteOffset)
{
    return byteOffset / 2;
}

uint32_t sampleOffsetToByteOffset (uint32_t sampleOffset)
{
    return sampleOffset * 2;
}

ChannelEditorComponent::ChannelEditorComponent ()
{
    setOpaque (true);
    setupComponents ();

#if 0
    // test our 1-99 scaling code
    for (auto uiValue { 1 }; uiValue < 100; ++uiValue)
    {
        const auto internalValue { getInternalValue (uiValue) };
        const auto recalculatedUiValue { getUiValue (internalValue) };
        jassert (uiValue == recalculatedUiValue);
        DebugLog ("ChannelEditorComponent", juce::String (uiValue) + " : " + juce::String (internalValue) + " : " + juce::String (recalculatedUiValue));
    }
#endif
}

ChannelEditorComponent ::~ChannelEditorComponent ()
{
    outputComboBox.setLookAndFeel (nullptr);
    stepsComboBox.setLookAndFeel (nullptr);
    eTrigComboBox.setLookAndFeel (nullptr);
    chokeComboBox.setLookAndFeel (nullptr);
    loopModeComboBox.setLookAndFeel (nullptr);
    filterTypeComboBox.setLookAndFeel (nullptr);
    quantComboBox.setLookAndFeel (nullptr);
    rateComboBox.setLookAndFeel (nullptr);
    channelSourceComboBox.setLookAndFeel (nullptr);
}

void ChannelEditorComponent::setupComponents ()
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
    // FILENAME
    setupLabel (fileNameLabel, "FILE", kMediumLabelSize, juce::Justification::centred);
    fileNameSelectLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
    fileNameSelectLabel.setColour (juce::Label::ColourIds::backgroundColourId, juce::Colours::black);
    fileNameSelectLabel.setOutline (juce::Colours::white);
    fileNameSelectLabel.onFilesSelected = [this] (const juce::StringArray& files)
    {
        if (! handleSampleAssignment (files[0]))
        {
            // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
        }
    };
    fileNameSelectLabel.onPopupMenuCallback = [this] () {};
    setupLabel (fileNameSelectLabel, "", 15.0, juce::Justification::centredLeft);

    setupLabel (channelSourceLabel, "SAMPLE CHANNEL", kMediumLabelSize, juce::Justification::centred);
    {
        for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
        {
            // TODO - the channel index is not known until ::init is called
            //const auto channelString { juce::String ("C") + juce::String (curChannelIndex + 1) + (squidChannelProperties.getChannelIndex () == curChannelIndex ? "(self)" : "") };
            const auto channelString { juce::String ("C") + juce::String (curChannelIndex + 1) };
            channelSourceComboBox.addItem (channelString, curChannelIndex + 1);
        }
    }
    channelSourceComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    channelSourceComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        squidChannelProperties.setChannelSource (static_cast<uint8_t>(std::clamp (rateComboBox.getSelectedItemIndex () + scrollAmount, 0, channelSourceComboBox.getNumItems () - 1)), true);
    };
    setupComboBox (channelSourceComboBox, "SampleChannel", [this] () { channelSourceUiChanged (static_cast<uint8_t>(channelSourceComboBox.getSelectedItemIndex ())); });

    // BITS
    setupLabel (bitsLabel, "BITS", kMediumLabelSize, juce::Justification::centred);
    bitsTextEditor.getMinValueCallback = [this] () { return 1; };
    bitsTextEditor.getMaxValueCallback = [this] () { return 16; };
    bitsTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    bitsTextEditor.updateDataCallback = [this] (int value) { bitsUiChanged (value == 16 ? 0 : value); };
    bitsTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 1;
            else
                return 3;
        } ();
        const auto newValue { squidChannelProperties.getBits () + (multiplier * direction) };
        bitsTextEditor.setValue (newValue);
    };
    setupTextEditor (bitsTextEditor, juce::Justification::centred, 0, "0123456789", "Bits"); // 1-16
    // RATE
    setupLabel (rateLabel, "RATE", kMediumLabelSize, juce::Justification::centred);
    rateComboBox.addItem ("4", 8);
    rateComboBox.addItem ("6", 7);
    rateComboBox.addItem ("7", 6);
    rateComboBox.addItem ("9", 5);
    rateComboBox.addItem ("11", 4);
    rateComboBox.addItem ("14", 3);
    rateComboBox.addItem ("22", 2);
    rateComboBox.addItem ("44", 1);
    rateComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    rateComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        squidChannelProperties.setRate (rateComboBox.getItemId (std::clamp (rateComboBox.getSelectedItemIndex () + scrollAmount, 0, rateComboBox.getNumItems () - 1)), true);
    };
    setupComboBox (rateComboBox, "Rate", [this] () { rateUiChanged (rateComboBox.getSelectedId () - 1); }); // 4,6,7,9,11,14,22,44
    // SPEED
    setupLabel (speedLabel, "SPEED", kMediumLabelSize, juce::Justification::centred);
    speedTextEditor.getMinValueCallback = [this] () { return 1; };
    speedTextEditor.getMaxValueCallback = [this] () { return 99; };
    speedTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    speedTextEditor.updateDataCallback = [this] (int value) { speedUiChanged (value); };
    speedTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 10;
            else
                return 25;
        } ();
        const auto newValue { getUiValue (squidChannelProperties.getSpeed ()) + (multiplier * direction) };
        speedTextEditor.setValue (newValue);
    };
    setupTextEditor (speedTextEditor, juce::Justification::centred, 0, "0123456789", "Speed"); // 1 - 99 (50 is normal, below that is negative speed? above is positive?)
    // QUANTIZE
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
    quantComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        squidChannelProperties.setQuant (std::clamp (quantComboBox.getSelectedItemIndex () + scrollAmount, 0, quantComboBox.getNumItems () - 1), true);
    };
    setupComboBox (quantComboBox, "Quantize", [this] () { quantUiChanged (quantComboBox.getSelectedId () - 1); }); // 0-14 (Off, 12, OT, MA, mi, Hm, PM, Pm, Ly, Ph, Jp, P5, C1, C4, C5)
    // FILTER TYPE
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
    filterTypeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        squidChannelProperties.setFilterType (std::clamp (filterTypeComboBox.getSelectedItemIndex () + scrollAmount, 0, filterTypeComboBox.getNumItems () - 1), true);
    };
    setupComboBox (filterTypeComboBox, "Filter", [this] () { filterTypeUiChanged (filterTypeComboBox.getSelectedId () - 1); }); // Off, LP, BP, NT, HP (0-4)
    // FILTER FREQUENCY
    setupLabel (filterFrequencyLabel, "FREQ", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (filterFrequencyTextEditor, juce::Justification::centred, 0, "0123456789", "Frequency"); // 1-99?
    // FILTER RESONANCE
    setupLabel (filterResonanceLabel, "RESO", kMediumLabelSize, juce::Justification::centred);
    setupTextEditor (filterResonanceTextEditor, juce::Justification::centred, 0, "0123456789", "Resonance"); // 1-99?
    // LEVEL
    setupLabel (levelLabel, "LEVEL", kMediumLabelSize, juce::Justification::centred);
    levelTextEditor.getMinValueCallback = [this] () { return 1; };
    levelTextEditor.getMaxValueCallback = [this] () { return 99; };
    levelTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    levelTextEditor.updateDataCallback = [this] (int value) { levelUiChanged (value); };
    levelTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 10;
            else
                return 25;
        } ();
        const auto newValue { getUiValue (squidChannelProperties.getLevel ()) + (multiplier * direction) };
        levelTextEditor.setValue (newValue);
    };
    setupTextEditor (levelTextEditor, juce::Justification::centred, 0, "0123456789", "Level"); // 1-99
    // ATTACK
    setupLabel (attackLabel, "ATTACK", kMediumLabelSize, juce::Justification::centred);
    attackTextEditor.getMinValueCallback = [this] () { return 1; };
    attackTextEditor.getMaxValueCallback = [this] () { return 99; };
    attackTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    attackTextEditor.updateDataCallback = [this] (int value) { attackUiChanged (value); };
    attackTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 10;
            else
                return 25;
        } ();
        const auto newValue { getUiValue (squidChannelProperties.getAttack ()) + (multiplier * direction) };
        attackTextEditor.setValue (newValue);
    };
    setupTextEditor (attackTextEditor, juce::Justification::centred, 0, "0123456789", "Attack"); // 0-99
    // DECAY
    setupLabel (decayLabel, "DECAY", kMediumLabelSize, juce::Justification::centred);
    decayTextEditor.getMinValueCallback = [this] () { return 1; };
    decayTextEditor.getMaxValueCallback = [this] () { return 99; };
    decayTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    decayTextEditor.updateDataCallback = [this] (int value) { decayUiChanged (value); };
    decayTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 10;
            else
                return 25;
        } ();
        const auto newValue { getUiValue (squidChannelProperties.getDecay ()) + (multiplier * direction) };
        decayTextEditor.setValue (newValue);
    };
    setupTextEditor (decayTextEditor, juce::Justification::centred, 0, "0123456789", "Decay"); // 0-99
    // LOOP MODE
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
    setupComboBox (loopModeComboBox, "LoopMode", [this] () { loopModeUiChanged (loopModeComboBox.getSelectedItemIndex ()); }); // none, normal, zigZag, gate, zigZagGate (0-4)
    // XFADE
    setupLabel (xfadeLabel, "XFADE", kMediumLabelSize, juce::Justification::centred);
    xfadeTextEditor.getMinValueCallback = [this] () { return 0; };
    xfadeTextEditor.getMaxValueCallback = [this] () { return 99; };
    xfadeTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    xfadeTextEditor.updateDataCallback = [this] (int value) { xfadeUiChanged (value); };
    xfadeTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 10;
            else
                return 25;
        } ();
        const auto newValue { squidChannelProperties.getXfade () + (multiplier * direction) };
        xfadeTextEditor.setValue (newValue);
    };
    setupTextEditor (xfadeTextEditor, juce::Justification::centred, 0, "0123456789", "XFade"); // 0 -99
    // REVERSE
    setupLabel (reverseLabel, "REVERSE", kMediumLabelSize, juce::Justification::centred);
    reverseButton.onClick = [this] () { reverseUiChanged (reverseButton.getToggleState ()); };
    addAndMakeVisible (reverseButton);
    // START
    setupLabel (startCueLabel, "START", kMediumLabelSize, juce::Justification::centred);
    startCueTextEditor.getMinValueCallback = [this] () { return 0; };
    startCueTextEditor.getMaxValueCallback = [this] () { return byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()); };
    startCueTextEditor.toStringCallback = [this] (juce::int32 value) { return juce::String (value); };
    startCueTextEditor.updateDataCallback = [this] (juce::int32 value) { startCueUiChanged (value); };
    startCueTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 20;
            else
                return 100;
        } ();
        const auto valueOffset { multiplier * direction };
        auto newValue { 0 };
        if (valueOffset < 0 && std::abs (valueOffset) > static_cast<int> (byteOffsetToSampleOffset (squidChannelProperties.getStartCue ())))
            newValue = 0;
        else
            newValue = byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()) + valueOffset;
        if (newValue > static_cast<int> (byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
            loopCueTextEditor.setValue (newValue);
        startCueTextEditor.setValue (newValue);
    };
    setupTextEditor (startCueTextEditor, juce::Justification::centred, 0, "0123456789", "Start"); // 0 - sample length?
    // LOOP
    setupLabel (loopCueLabel, "LOOP", kMediumLabelSize, juce::Justification::centred);
    loopCueTextEditor.getMinValueCallback = [this] () { return byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()); };
    loopCueTextEditor.getMaxValueCallback = [this] () { return byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()); };
    loopCueTextEditor.toStringCallback = [this] (juce::int32 value) { return juce::String (value); };
    loopCueTextEditor.updateDataCallback = [this] (juce::int32 value) { loopCueUiChanged (value); };
    loopCueTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
             else if (dragSpeed == DragSpeed::medium)
                 return 20;
            else
                return 100;
        } ();
        const auto valueOffset { multiplier * direction };
        auto newValue { 0 };
        if (valueOffset < 0 && std::abs (valueOffset) > static_cast<int> (byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
            newValue = 0;
        else
            newValue = byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ()) + valueOffset;
        loopCueTextEditor.setValue (newValue);
    };
    setupTextEditor (loopCueTextEditor, juce::Justification::centred, 0, "0123456789", "Loop"); // 0 - sample length?, or sampleStart - sampleEnd
    // END
    setupLabel (endCueLabel, "END", kMediumLabelSize, juce::Justification::centred);
    endCueTextEditor.getMinValueCallback = [this] () { return byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()); };
    endCueTextEditor.getMaxValueCallback = [this] () { return squidChannelProperties.getSampleDataNumSamples (); };
    endCueTextEditor.toStringCallback = [this] (juce::int32 value) { return juce::String (value); };
    endCueTextEditor.updateDataCallback = [this] (juce::int32 value) { endCueUiChanged (value); };
    endCueTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 20;
            else
                return 100;
        } ();
        const auto valueOffset { multiplier * direction };
        auto newValue { 0 };
        if (valueOffset < 0 && std::abs (valueOffset) > static_cast<int> (byteOffsetToSampleOffset (squidChannelProperties.getEndCue ())))
            newValue = 0;
        else
            newValue = byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()) + valueOffset;
        if (newValue < static_cast<int> (byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
            loopCueTextEditor.setValue (newValue);
        endCueTextEditor.setValue (newValue);
    };
    setupTextEditor (endCueTextEditor, juce::Justification::centred, 0, "0123456789", "End"); // sampleStart - sample length
    // CHOKE
    setupLabel (chokeLabel, "CHOKE", kMediumLabelSize, juce::Justification::centred);
    {
        for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
        {
            // TODO - the channel index is not known until ::init is called
            //const auto channelString { juce::String ("C") + juce::String (curChannelIndex + 1) + (squidChannelProperties.getChannelIndex () == curChannelIndex ? "(self)" : "") };
            const auto channelString { juce::String ("C") + juce::String (curChannelIndex + 1) };
            chokeComboBox.addItem (channelString, curChannelIndex + 1);
        }
    }
    chokeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    setupComboBox (chokeComboBox, "Choke", [this] () { chokeUiChanged (chokeComboBox.getSelectedItemIndex ()); }); // C1, C2, C3, C4, C5, C6, C7, C8
    // ETrig
    setupLabel (eTrigLabel, "EOS TRIG", kMediumLabelSize, juce::Justification::centred);
    eTrigComboBox.addItem ("Off", 1);
    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
        eTrigComboBox.addItem ("> " + juce::String (curChannelIndex + 1), curChannelIndex + 2);
    eTrigComboBox.addItem ("On", 10);
    eTrigComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    setupComboBox (eTrigComboBox, "EOS Trig", [this] () { eTrigUiChanged (eTrigComboBox.getSelectedItemIndex ()); }); // Off, > 1, > 2, > 3, > 4, > 5, > 6, > 7, > 8, On
    // Steps 
    setupLabel (stepsLabel, "STEPS", kMediumLabelSize, juce::Justification::centred);
    stepsComboBox.addItem ("Off", 1);
    for (auto curNumSteps { 0 }; curNumSteps < 7; ++curNumSteps)
        stepsComboBox.addItem ("- " + juce::String (curNumSteps + 2), curNumSteps + 2);
    stepsComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    setupComboBox (stepsComboBox, "Steps", [this] () {stepsUiChanged (stepsComboBox.getSelectedItemIndex ()); }); // 0-7 (Off, - 2, - 3, - 4, - 5, - 6, - 7, - 8)
    // Output
    setupLabel (outputLabel, "OUTPUT", kMediumLabelSize, juce::Justification::centred);
    outputComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    setupComboBox (outputComboBox, "Output", [this] ()
    {
        //           alt out 
        // chan | false | true
        // -----+-------+------
        //  1   |  1-2  | 3-4
        //  2   |  1-2  | 3-4
        //  3   |  3-4  | 1-2
        //  4   |  3-4  | 1-2
        // -----+-------+------
        //  5   |  5-6  | 7-8
        //  6   |  5-6  | 7-8
        //  7   |  7-8  | 5-6
        //  8   |  7-8  | 5-6
        const auto channelIndex = squidChannelProperties.getChannelIndex (); // 0-7
        const auto selectedIndex { outputComboBox.getSelectedItemIndex () }; // 0-1
        auto disableAltOut = [this] ()
        {
            const auto offFlag { ~ChannelFlags::kNeighborOutput };
            const auto newFlags { static_cast<uint16_t> (squidChannelProperties.getChannelFlags () & offFlag) };
            squidChannelProperties.setChannelFlags (newFlags, false);
        };
        auto enableAltOut = [this] ()
        {
            const auto newFlags { static_cast<uint16_t> (squidChannelProperties.getChannelFlags () | ChannelFlags::kNeighborOutput) };
            squidChannelProperties.setChannelFlags (newFlags, false);
        };
        if (channelIndex < 2) // 1-2
        {
            if (selectedIndex == 0)
                disableAltOut ();
            else
                enableAltOut ();
        }
        else if (channelIndex < 4) // 3-4
        {
            if (selectedIndex == 1)
                disableAltOut ();
            else
                enableAltOut ();
        }
        else if (channelIndex < 6) // 5-6
        {
            if (selectedIndex == 0)
                disableAltOut ();
            else
                enableAltOut ();
        }
        else // 7-8
        {
            if (selectedIndex == 1)
                disableAltOut ();
            else
                enableAltOut ();
        }
    });

    setupLabel (cueRandomLabel, "CUE RANDOM", kMediumLabelSize, juce::Justification::centred);
    cueRandomButton.onClick = [this] ()
    {
        if (cueRandomButton.getToggleState ())
        {
            // turn cue random on
            squidChannelProperties.setChannelFlags (squidChannelProperties.getChannelFlags () & ChannelFlags::kCueRandom, false);
            // turn cue stepped off (in case it is on)
            squidChannelProperties.setChannelFlags (squidChannelProperties.getChannelFlags () | ~ChannelFlags::kCueStepped, false);
            cueStepButton.setToggleState (false, juce::NotificationType::dontSendNotification);
        }
        else
        {
            // turn cue random off
            squidChannelProperties.setChannelFlags (squidChannelProperties.getChannelFlags () | ~ChannelFlags::kCueRandom, false);
        }
    };
    addAndMakeVisible (cueRandomButton);
    setupLabel (cueStepLabel, "CUE STEP", kMediumLabelSize, juce::Justification::centred);
    cueStepButton.onClick = [this] ()
    {
        if (cueStepButton.getToggleState ())
        {
            // turn cue step on
            squidChannelProperties.setChannelFlags (squidChannelProperties.getChannelFlags () & ChannelFlags::kCueStepped, false);
            // turn cue random off (in case it is on)
            squidChannelProperties.setChannelFlags (squidChannelProperties.getChannelFlags () | ~ChannelFlags::kCueRandom, false);
            cueRandomButton.setToggleState (false, juce::NotificationType::dontSendNotification);
        }
        else
        {
            // turn cue step off
            squidChannelProperties.setChannelFlags (squidChannelProperties.getChannelFlags () | ~ChannelFlags::kCueStepped, false);
        }
    };
    addAndMakeVisible (cueStepButton);

    addAndMakeVisible (loopPointsView);

    auto setupPlayButton = [this] (juce::TextButton& playButton, juce::String text, bool initilalEnabledState, juce::String otherButtonText, AudioPlayerProperties::PlayMode playMode)
    {
        playButton.setButtonText (text);
        playButton.setEnabled (initilalEnabledState);
        playButton.onClick = [this, text, &playButton, playMode, otherButtonText] ()
        {
            if (playButton.getButtonText () == "STOP")
            {
                // stopping
                audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
                playButton.setButtonText (text);
            }
            else
            {
                audioPlayerProperties.setSampleSource (squidChannelProperties.getChannelIndex (), false);
                audioPlayerProperties.setPlayMode (playMode, false);
                audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::play, false);
                playButton.setButtonText ("STOP");
                if (&playButton == &oneShotPlayButton)
                    loopPlayButton.setButtonText (otherButtonText);
                else
                    oneShotPlayButton.setButtonText (otherButtonText);
            }
        };
        addAndMakeVisible (playButton);
    };
    setupPlayButton (loopPlayButton, "LOOP", false, "ONCE", AudioPlayerProperties::PlayMode::loop);
    setupPlayButton (oneShotPlayButton, "ONCE", false, "LOOP", AudioPlayerProperties::PlayMode::once);

    addCueSetButton.setButtonText ("ADD CUE");
    addCueSetButton.onClick = [this] () { appendCueSet (); };
    addCueSetButton.setEnabled (false);
    addAndMakeVisible (addCueSetButton);
    deleteCueSetButton.setButtonText ("DELETE CUE");
    deleteCueSetButton.onClick = [this] () { deleteCueSet (squidChannelProperties.getCurCueSet ()); };
    deleteCueSetButton.setEnabled (false);
    addAndMakeVisible (deleteCueSetButton);

    // LOWER PANEL SELECT BUTTONS
    cueSetViewButton.setButtonText ("CUE SETS");
    cueSetViewButton.setToggleState (true, juce::NotificationType::dontSendNotification);
    cueSetViewButton.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    cueSetViewButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::white);
    cueSetViewButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    cueSetViewButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    cueSetViewButton.onClick = [this] ()
    {
        if (cueSetViewButton.getToggleState ())
            return;
        cueSetViewButton.setToggleState (true, juce::NotificationType::dontSendNotification);
        cvAssignViewButton.setToggleState (false, juce::NotificationType::dontSendNotification);
        setLowerPaneView (LowerPaneView::cueSets);
    };
    addAndMakeVisible (cueSetViewButton);
    cvAssignViewButton.setButtonText ("CV ASSIGN");
    cvAssignViewButton.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    cvAssignViewButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::white);
    cvAssignViewButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    cvAssignViewButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    cvAssignViewButton.onClick = [this] ()
    {
        if (cvAssignViewButton.getToggleState ())
            return;
        cvAssignViewButton.setToggleState (true, juce::NotificationType::dontSendNotification);
        cueSetViewButton.setToggleState (false, juce::NotificationType::dontSendNotification);
        setLowerPaneView (LowerPaneView::cvAssigns);
    };
    addAndMakeVisible (cvAssignViewButton);

    // WAVEFORM DISPLAY
    waveformDisplay.onStartPointChange = [this] (juce::int64 startPoint)
    {
        const auto startCueByteOffset { static_cast<int>(startPoint * 2) };
        squidChannelProperties.setCueSetStartPoint (curCueSetIndex, startCueByteOffset);
        squidChannelProperties.setStartCue (startCueByteOffset, true);
    };
    waveformDisplay.onLoopPointChange = [this] (juce::int64 loopPoint)
    {
        const auto loopCueByteOffset { static_cast<int>(loopPoint * 2) };
        squidChannelProperties.setCueSetLoopPoint (curCueSetIndex, loopCueByteOffset);
        squidChannelProperties.setLoopCue (loopCueByteOffset, true);
    };
    waveformDisplay.onEndPointChange = [this] (juce::int64 endPoint)
    {
        const auto endCueByteOffset { static_cast<int>(endPoint * 2) };
        squidChannelProperties.setCueSetEndPoint (curCueSetIndex, endCueByteOffset);
        squidChannelProperties.setEndCue (endCueByteOffset, true);
    };
    addAndMakeVisible (waveformDisplay);
    // WAVEFORM DISPLAY TABS
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
    // CV ASSIGN EDITOR
    addChildComponent (cvAssignEditor);
}

int ChannelEditorComponent::getUiValue (int internalValue)
{
    return static_cast<int> (std::round (internalValue / kScaleStep));
}

int ChannelEditorComponent::getInternalValue (int uiValue)
{
    return static_cast<int> (uiValue * kScaleStep);
}

void ChannelEditorComponent::setCueEditButtonsEnableState ()
{
    addCueSetButton.setEnabled (squidChannelProperties.getNumCueSets () < 64);
    deleteCueSetButton.setEnabled (squidChannelProperties.getNumCueSets () > 1);
}

void ChannelEditorComponent::setFilterEnableState ()
{
    const auto filterEnabled { squidChannelProperties.getFilterType () != 0 };
    filterFrequencyTextEditor.setEnabled (filterEnabled);
    filterResonanceTextEditor.setEnabled (filterEnabled);
}

void ChannelEditorComponent::setCurCue (int cueSetIndex)
{
    jassert (cueSetIndex < squidChannelProperties.getNumCueSets ());
    if (cueSetIndex >= squidChannelProperties.getNumCueSets ())
        return;
    cueSetButtons [curCueSetIndex].setToggleState (false, juce::NotificationType::dontSendNotification);
    curCueSetIndex = cueSetIndex;
    squidChannelProperties.setCurCueSet (cueSetIndex, false);
    cueSetButtons [cueSetIndex].setToggleState (true, juce::NotificationType::dontSendNotification);
    waveformDisplay.setCuePoints (byteOffsetToSampleOffset (squidChannelProperties.getStartCueSet (cueSetIndex)),
                                  byteOffsetToSampleOffset (squidChannelProperties.getLoopCueSet (cueSetIndex)), 
                                  byteOffsetToSampleOffset (squidChannelProperties.getEndCueSet (cueSetIndex)));
    squidChannelProperties.setStartCue (squidChannelProperties.getStartCueSet (cueSetIndex), true);
    squidChannelProperties.setLoopCue (squidChannelProperties.getLoopCueSet (cueSetIndex), true);
    squidChannelProperties.setEndCue (squidChannelProperties.getEndCueSet (cueSetIndex), true);
}

void ChannelEditorComponent::initCueSetTabs ()
{
    const auto numCueSets { squidChannelProperties.getNumCueSets () };
    for (auto cueSetButtonIndex { 0 }; cueSetButtonIndex < cueSetButtons.size (); ++cueSetButtonIndex)
        cueSetButtons [cueSetButtonIndex].setEnabled (cueSetButtonIndex < numCueSets);
};

bool ChannelEditorComponent::loadFile (juce::String fileName)
{
    return handleSampleAssignment (fileName);
}

void ChannelEditorComponent::init (juce::ValueTree squidChannelPropertiesVT, juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties { rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no };
    RuntimeRootProperties runtimeRootProperties { rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no };
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();

    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::client, AudioPlayerProperties::EnableCallbacks::yes);
    audioPlayerProperties.onPlayStateChange = [this] (AudioPlayerProperties::PlayState playState)
    {
        if (playState == AudioPlayerProperties::PlayState::stop)
        {
            juce::MessageManager::callAsync ([this] ()
            {
                oneShotPlayButton.setButtonText ("ONCE");
                loopPlayButton.setButtonText ("LOOP");
            });
        }
        else if (playState == AudioPlayerProperties::PlayState::play)
        {
            if (audioPlayerProperties.getPlayMode() == AudioPlayerProperties::PlayMode::once)
            {
                juce::MessageManager::callAsync ([this] ()
                {
                    oneShotPlayButton.setButtonText ("STOP");
                    loopPlayButton.setButtonText ("LOOP");
                });
            }
            else
            {
                juce::MessageManager::callAsync ([this] ()
                {
                    oneShotPlayButton.setButtonText ("ONCE");
                    loopPlayButton.setButtonText ("STOP");
                });
            }
        }
        else
        {
            jassertfalse;
        }
    };

    squidChannelProperties.wrap (squidChannelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);
    cvAssignEditor.init (squidChannelPropertiesVT);

    // TODO - we need to call this when the sample changes
    updateLoopPointsView ();

    initOutputComboBox ();

    initCueSetTabs ();
    setCurCue (squidChannelProperties.getCurCueSet ());

    // put initial data into the UI
    attackDataChanged (squidChannelProperties.getAttack ());
    channelSourceDataChanged (squidChannelProperties.getChannelSource ());
    channelFlagsDataChanged (squidChannelProperties.getChannelFlags ());
    chokeDataChanged (squidChannelProperties.getChoke ());
    bitsDataChanged (squidChannelProperties.getBits ());
    decayDataChanged (squidChannelProperties.getDecay ());
    endCueDataChanged (squidChannelProperties.getEndCue ());
    eTrigDataChanged (squidChannelProperties.getETrig ());
    fileNameDataChanged (squidChannelProperties.getFileName ());
    filterTypeDataChanged (squidChannelProperties.getFilterType ());
    filterFrequencyDataChanged (squidChannelProperties.getFilterFrequency ());
    filterResonanceDataChanged (squidChannelProperties.getFilterResonance ());
    levelDataChanged (squidChannelProperties.getLevel ());
    loopCueDataChanged (squidChannelProperties.getLoopCue ());
    loopModeDataChanged (squidChannelProperties.getLoopMode ());
    quantDataChanged (squidChannelProperties.getQuant ());
    rateDataChanged (squidChannelProperties.getRate ());
    reverseDataChanged (squidChannelProperties.getReverse ());
    speedDataChanged (squidChannelProperties.getSpeed ());
    startCueDataChanged (squidChannelProperties.getStartCue ());
    stepsDataChanged (squidChannelProperties.getSteps ());
    xfadeDataChanged (squidChannelProperties.getXfade ());

    initializeCallbacks ();

    const auto channelIndex { squidChannelProperties.getChannelIndex () };
    if (channelIndex > 4)
    {
        speedLabel.setVisible (false);
        speedTextEditor.setVisible (false);
    }
    else
    {
        quantLabel.setVisible (false);
        quantComboBox.setVisible (false);
    }
    setFilterEnableState ();
    setCueEditButtonsEnableState ();
}

void ChannelEditorComponent::initializeCallbacks ()
{
    jassert (squidChannelProperties.isValid ());
    squidChannelProperties.onAttackChange = [this] (int attack) { attackDataChanged (attack); };
    squidChannelProperties.onBitsChange = [this] (int bits) { bitsDataChanged (bits); };
    squidChannelProperties.onChannelFlagsChange = [this] (uint16_t channelFlags) { channelFlagsDataChanged (channelFlags); };
    squidChannelProperties.onChannelSourceChange = [this] (uint8_t channelSourceIndex) { channelSourceDataChanged (channelSourceIndex); };
    squidChannelProperties.onChokeChange = [this] (int choke) { chokeDataChanged (choke); };
    squidChannelProperties.onCurCueSetChange = [this] (int cueSetIndex) { setCurCue (cueSetIndex); };
    squidChannelProperties.onDecayChange = [this] (int decay) { decayDataChanged (decay); };
    squidChannelProperties.onFileNameChange = [this] (juce::String fileName) { fileNameDataChanged (fileName); };
    squidChannelProperties.onEndCueChange = [this] (int endCue) { endCueDataChanged (endCue); };
    squidChannelProperties.onEndCueSetChange = [this] (int cueIndex, int endCue)
    {
        if (cueIndex == curCueSetIndex)
            waveformDisplay.setCueEndPoint (byteOffsetToSampleOffset (endCue));
    };
    squidChannelProperties.onETrigChange = [this] (int eTrig) { eTrigDataChanged (eTrig); };
    squidChannelProperties.onFilterTypeChange = [this] (int filter) { filterTypeDataChanged (filter); };
    squidChannelProperties.onFilterFrequencyChange = [this] (int filterFrequency) { filterFrequencyDataChanged (filterFrequency); };
    squidChannelProperties.onFilterResonanceChange = [this] (int filterResonance) { filterResonanceDataChanged (filterResonance); };
    squidChannelProperties.onLevelChange = [this] (int level) { levelDataChanged (level); };
    squidChannelProperties.onLoadBegin = [this] ()
    {
        audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
    };
    squidChannelProperties.onLoadComplete = [this] ()
    {
        initCueSetTabs ();
        auto sampleFileName { juce::File (appProperties.getRecentlyUsedFile (0)).getChildFile (juce::String (squidChannelProperties.getChannelIndex () + 1)).getChildFile (squidChannelProperties.getFileName ()) };
        setCurCue (squidChannelProperties.getCurCueSet ());
    };
    squidChannelProperties.onLoopCueChange = [this] (int loopCue) { loopCueDataChanged (loopCue); };
    squidChannelProperties.onLoopCueSetChange = [this] (int cueIndex, int loopCue)
    {
        if (cueIndex == curCueSetIndex)
            waveformDisplay.setCueLoopPoint (byteOffsetToSampleOffset (loopCue));
    };
    squidChannelProperties.onLoopModeChange = [this] (int loopMode) { loopModeDataChanged (loopMode); };
    squidChannelProperties.onNumCueSetsChange = [this] (int /*numCueSets*/)
    {
        initCueSetTabs ();
        setCueEditButtonsEnableState ();
    };
    squidChannelProperties.onQuantChange = [this] (int quant) { quantDataChanged (quant); };
    squidChannelProperties.onRateChange = [this] (int rate) { rateDataChanged (rate); };
    squidChannelProperties.onReverseChange = [this] (int reverse) { reverseDataChanged (reverse); };
    squidChannelProperties.onSpeedChange = [this] (int speed) { speedDataChanged (speed); };
    squidChannelProperties.onStartCueChange = [this] (int startCue) { startCueDataChanged (startCue); };
    squidChannelProperties.onStartCueSetChange = [this] (int cueIndex, int startCue)
    {
        if (cueIndex == curCueSetIndex)
            waveformDisplay.setCueStartPoint (byteOffsetToSampleOffset (startCue));
    };
    squidChannelProperties.onStepsChange = [this] (int steps) { stepsDataChanged (steps); };
    squidChannelProperties.onXfadeChange = [this] (int xfade) { xfadeDataChanged (xfade); };

    squidChannelProperties.onSampleDataAudioBufferChange = [this] (AudioBufferRefCounted::RefCountedPtr audioBufferPtr)
    {
        updateLoopPointsView ();
        if (squidChannelProperties.getSampleDataAudioBuffer () != nullptr)
            waveformDisplay.init (squidChannelProperties.getSampleDataAudioBuffer ()->getAudioBuffer ());
        else
            waveformDisplay.init (nullptr);
        oneShotPlayButton.setEnabled (squidChannelProperties.getSampleDataAudioBuffer () != nullptr);
        loopPlayButton.setEnabled (squidChannelProperties.getSampleDataAudioBuffer () != nullptr);
    };
}

void ChannelEditorComponent::updateLoopPointsView ()
{
    uint32_t startSample { 0 };
    uint32_t numBytes { 0 };
    if (squidChannelProperties.getSampleDataAudioBuffer () != nullptr)
    {
        startSample = squidChannelProperties.getLoopCue ();
        numBytes = squidChannelProperties.getEndCue () - startSample;
        loopPointsView.setAudioBuffer (squidChannelProperties.getSampleDataAudioBuffer ()->getAudioBuffer());
        waveformDisplay.init (squidChannelProperties.getSampleDataAudioBuffer ()->getAudioBuffer ());
    }
    else
    {
        loopPointsView.setAudioBuffer (nullptr);
    }
    oneShotPlayButton.setEnabled (squidChannelProperties.getSampleDataAudioBuffer () != nullptr);
    loopPlayButton.setEnabled (squidChannelProperties.getSampleDataAudioBuffer () != nullptr);
    loopPointsView.setLoopPoints (byteOffsetToSampleOffset (startSample), byteOffsetToSampleOffset (numBytes));
    loopPointsView.repaint ();
}

void ChannelEditorComponent::appendCueSet ()
{
    const auto numCueSets { squidChannelProperties.getNumCueSets () };
    jassert (numCueSets < 64);
    if (numCueSets == 64)
        return;
    const auto newCueSetIndex { numCueSets };
    squidChannelProperties.setCueSetPoints (newCueSetIndex, squidChannelProperties.getStartCueSet (numCueSets - 1), squidChannelProperties.getLoopCueSet (numCueSets - 1), squidChannelProperties.getEndCueSet (numCueSets - 1));
    squidChannelProperties.setCurCueSet (newCueSetIndex, true);
}

void ChannelEditorComponent::deleteCueSet (int cueSetIndex)
{
    squidChannelProperties.removeCueSet (cueSetIndex);
}

// Data Changed functions
void ChannelEditorComponent::attackDataChanged (int attack)
{
    attackTextEditor.setText (juce::String (getUiValue (attack)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::bitsDataChanged (int bits)
{
    bitsTextEditor.setText (juce::String (bits == 0 ? 16 : bits), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::channelSourceDataChanged (uint8_t channelSourceIndex)
{
    configFileSelectorFromChannelSource ();
    channelSourceComboBox.setSelectedItemIndex (channelSourceIndex, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::channelFlagsDataChanged (uint16_t channelFlags)
{
    // handle cue random flag
    cueRandomButton.setToggleState (channelFlags & ChannelFlags::kCueRandom, juce::NotificationType::dontSendNotification);

    // handle cue step flag
    cueStepButton.setToggleState (channelFlags & ChannelFlags::kCueStepped, juce::NotificationType::dontSendNotification);

    // handle neighbor out flag
    const auto useAltOut { channelFlags & ChannelFlags::kNeighborOutput };
    const auto channelIndex { squidChannelProperties.getChannelIndex () };
    if (channelIndex < 2) // 1-2
    {
        if (useAltOut)
            outputComboBox.setSelectedItemIndex (1, juce::NotificationType::dontSendNotification);
        else
            outputComboBox.setSelectedItemIndex (0, juce::NotificationType::dontSendNotification);
    }
    else if (channelIndex < 4) // 3-4
    {
        if (useAltOut)
            outputComboBox.setSelectedItemIndex (0, juce::NotificationType::dontSendNotification);
        else
            outputComboBox.setSelectedItemIndex (1, juce::NotificationType::dontSendNotification);
    }
    else if (channelIndex < 6) // 5-6
    {
        if (useAltOut)
            outputComboBox.setSelectedItemIndex (1, juce::NotificationType::dontSendNotification);
        else
            outputComboBox.setSelectedItemIndex (0, juce::NotificationType::dontSendNotification);
    }
    else // 7-8
    {
        if (useAltOut)
            outputComboBox.setSelectedItemIndex (0, juce::NotificationType::dontSendNotification);
        else
            outputComboBox.setSelectedItemIndex (1, juce::NotificationType::dontSendNotification);
    }
}

void ChannelEditorComponent::chokeDataChanged (int choke)
{
    chokeComboBox.setSelectedItemIndex (choke, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::decayDataChanged (int decay)
{
    decayTextEditor.setText (juce::String (getUiValue (decay)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::endCueDataChanged (juce::int32 endCueByteOffset)
{
    const auto endCueSampleOffset { byteOffsetToSampleOffset(endCueByteOffset) };
    endCueTextEditor.setText (juce::String (endCueSampleOffset), juce::NotificationType::dontSendNotification);
    waveformDisplay.setCueEndPoint (endCueSampleOffset);
    updateLoopPointsView ();
}

void ChannelEditorComponent::eTrigDataChanged (int eTrig)
{
    eTrigComboBox.setSelectedItemIndex (eTrig, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::fileNameDataChanged (juce::String fileName)
{
    fileNameSelectLabel.setText (fileName, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::filterTypeDataChanged (int filterType)
{
    filterTypeComboBox.setSelectedItemIndex (filterType, juce::NotificationType::dontSendNotification);
    setFilterEnableState ();
}

void ChannelEditorComponent::filterFrequencyDataChanged (int filterFrequency)
{
    auto getFilterFrequencyUiValue = [] (int internalValue)
    {
        return (internalValue == 0 ? 99 : 98 - ((internalValue - 55) / 40));
    };
    filterFrequencyTextEditor.setText (juce::String (getFilterFrequencyUiValue(filterFrequency)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::filterResonanceDataChanged (int filterResonance)
{
    filterResonanceTextEditor.setText (juce::String (getUiValue (filterResonance)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::levelDataChanged (int level)
{
    levelTextEditor.setText (juce::String (getUiValue (level)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::loopCueDataChanged (juce::int32 loopCueByteOffset)
{
    const auto loopCueSampleOffset { byteOffsetToSampleOffset (loopCueByteOffset) };
    loopCueTextEditor.setText (juce::String (loopCueSampleOffset), false);
    waveformDisplay.setCueLoopPoint (loopCueSampleOffset);
    updateLoopPointsView ();
}

void ChannelEditorComponent::loopModeDataChanged (int loopMode)
{
    loopModeComboBox.setSelectedItemIndex (loopMode, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::quantDataChanged (int quant)
{
    quantComboBox.setSelectedItemIndex (quant, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::rateDataChanged (int rate)
{
    rateComboBox.setSelectedId (rate + 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::reverseDataChanged (int reverse)
{
    reverseButton.setToggleState (reverse == 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::speedDataChanged (int speed)
{
    speedTextEditor.setText (juce::String (getUiValue (speed)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::startCueDataChanged (juce::int32 startCueByteOffset)
{
    const auto startCueSampleOffset { byteOffsetToSampleOffset (startCueByteOffset) };
    startCueTextEditor.setText (juce::String (startCueSampleOffset), juce::NotificationType::dontSendNotification);
    waveformDisplay.setCueStartPoint (startCueSampleOffset);
    updateLoopPointsView ();
}

void ChannelEditorComponent::stepsDataChanged (int steps)
{
    stepsComboBox.setSelectedItemIndex (steps, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::xfadeDataChanged (int xfade)
{
    xfadeTextEditor.setText (juce::String (xfade), juce::NotificationType::dontSendNotification);
}

// UI Changed functions
void ChannelEditorComponent::attackUiChanged (int attack)
{
    const auto newAttackValue { getInternalValue (attack) };
    squidChannelProperties.setAttack (newAttackValue, false);
}

void ChannelEditorComponent::bitsUiChanged (int bits)
{
    squidChannelProperties.setBits (bits, false);
}

void ChannelEditorComponent::configFileSelectorFromChannelSource ()
{
    fileNameSelectLabel.setEnabled (squidChannelProperties.getChannelSource () == squidChannelProperties.getChannelIndex ());
}

void ChannelEditorComponent::channelSourceUiChanged (uint8_t channelSourceIndex)
{
    squidChannelProperties.setChannelSource (channelSourceIndex, false);
    configFileSelectorFromChannelSource ();
}

void ChannelEditorComponent::chokeUiChanged (int choke)
{
    squidChannelProperties.setChoke (choke, false);
}

void ChannelEditorComponent::decayUiChanged (int decay)
{
    const auto newDecayValue { getInternalValue (decay) };
    squidChannelProperties.setDecay (newDecayValue, false);
}

void ChannelEditorComponent::endCueUiChanged (juce::int32 endCueSampleOffset)
{
    const auto endCueByteOffset { sampleOffsetToByteOffset (endCueSampleOffset) };
    squidChannelProperties.setEndCue (endCueByteOffset, false);
    squidChannelProperties.setCueSetEndPoint (curCueSetIndex, endCueByteOffset);

    waveformDisplay.setCueEndPoint (endCueSampleOffset);
    updateLoopPointsView ();
}

void ChannelEditorComponent::eTrigUiChanged (int eTrig)
{
    squidChannelProperties.setETrig (eTrig, false);
}

void ChannelEditorComponent::filterTypeUiChanged (int filter)
{
    squidChannelProperties.setFilterType (filter, false);
    setFilterEnableState ();
}

void ChannelEditorComponent::filterFrequencyUiChanged (int filterFrequency)
{
    auto getFileterFrequencyInternalValue = [] (int uiValue)
    {
        const auto invertedValue = 99 - uiValue;
        return (invertedValue == 0 ? 0 : 55 + ((invertedValue - 1) * 40));
    };
    squidChannelProperties.setFilterFrequency (getFileterFrequencyInternalValue (filterFrequency), false);
}

void ChannelEditorComponent::filterResonanceUiChanged (int filterResonance)
{
    const auto newResonanceValue { getInternalValue (filterResonance) };
    squidChannelProperties.setFilterResonance (newResonanceValue, false);
}

void ChannelEditorComponent::levelUiChanged (int level)
{
    const auto newLevelValue { getInternalValue (level) };
    squidChannelProperties.setLevel (newLevelValue, false);
}

void ChannelEditorComponent::loopCueUiChanged (juce::int32 loopCueSampleOffset)
{
    const auto loopCueByteOffset { sampleOffsetToByteOffset (loopCueSampleOffset) };
    squidChannelProperties.setLoopCue (loopCueByteOffset, false);
    squidChannelProperties.setCueSetLoopPoint (curCueSetIndex, loopCueByteOffset);

    waveformDisplay.setCueLoopPoint (loopCueSampleOffset);
    updateLoopPointsView ();
}

void ChannelEditorComponent::loopModeUiChanged (int loopMode)
{
    squidChannelProperties.setLoopMode (loopMode, false);
}

void ChannelEditorComponent::quantUiChanged (int quant)
{
    squidChannelProperties.setQuant (quant, false);
}

void ChannelEditorComponent::rateUiChanged (int rate)
{
    squidChannelProperties.setRate (rate, false);
}

void ChannelEditorComponent::reverseUiChanged (int reverse)
{
    squidChannelProperties.setReverse (reverse, false);
}

void ChannelEditorComponent::speedUiChanged (int speed)
{
    const auto newSpeedValue { getInternalValue (speed) };
    squidChannelProperties.setSpeed (newSpeedValue, false);
}

void ChannelEditorComponent::startCueUiChanged (juce::int32 startCueSampleOffset)
{
    const auto startCueByteOffset { sampleOffsetToByteOffset (startCueSampleOffset) };
    squidChannelProperties.setStartCue (startCueByteOffset, false);
    squidChannelProperties.setCueSetStartPoint (curCueSetIndex, startCueByteOffset);

    waveformDisplay.setCueStartPoint (startCueSampleOffset);
    updateLoopPointsView ();
}

void ChannelEditorComponent::stepsUiChanged (int steps)
{
    squidChannelProperties.setSteps (steps, false);
}

void ChannelEditorComponent::xfadeUiChanged (int xfade)
{
    squidChannelProperties.setXfade (xfade, false);
}

bool ChannelEditorComponent::handleSampleAssignment (juce::String fileName)
{
    juce::Logger::outputDebugString ("sample to load: " + fileName);
    auto srcFile { juce::File (fileName) };
    const auto channelDirectory { juce::File(appProperties.getRecentlyUsedFile (0)).getChildFile(juce::String(squidChannelProperties.getChannelIndex () + 1)) };
    auto destFile { channelDirectory.getChildFile (srcFile.getFileName ()) };
    if (srcFile.getParentDirectory () != channelDirectory)
    {
        // TODO handle case where file of same name already exists
        // TODO should copy be moved to a thread?
        srcFile.copyFileTo (destFile);
        // TODO handle failure
    }
    // TODO - we should probably handle the case of the file missing. it shouldn't happen, as the file was selected through the file manager or a drag/drop
    //        but it's possible that the file gets deleted somehow after selection
    jassert (destFile.exists ());
    editManager->loadChannel (squidChannelProperties.getValueTree (), squidChannelProperties.getChannelIndex (), destFile);
    return true;
}

bool ChannelEditorComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    if (files.size () != 1 || ! editManager->isSupportedAudioFile (files[0]))
        return false;

    return true;
}

void ChannelEditorComponent::filesDropped (const juce::StringArray& files, int /*x*/, int /*y*/)
{
//    draggingFiles = false;
//    repaint ();
    if (! handleSampleAssignment (files[0]))
    {
//        // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
    }
}

//void ChannelEditorComponent::fileDragEnter (const juce::StringArray& files, int x, int y)
//{
//    setDropIndex (files, x, y);
//    draggingFiles = true;
//    repaint ();
//}

//void ChannelEditorComponent::fileDragMove (const juce::StringArray& files, int x, int y)
//{
//    setDropIndex (files, x, y);
//    repaint ();
//}

//void ChannelEditorComponent::fileDragExit (const juce::StringArray&)
//{
//    draggingFiles = false;
//    repaint ();
//}

void ChannelEditorComponent::resized ()
{
    const auto columnWidth { 160 };
    const auto spaceBetweenColumns { 10 };

    const auto fieldWidth { columnWidth / 2 - 2 };
    auto xInitialOffSet { 15 };

    auto xOffset { xInitialOffSet };
    auto yOffset { kInitialYOffset };

    // FILENAME
    fileNameLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    fileNameSelectLabel.setBounds (fileNameLabel.getRight () + 3, yOffset, fieldWidth * 3, kParameterLineHeight);

    channelSourceLabel.setBounds (fileNameSelectLabel.getRight () + 10, yOffset, fieldWidth + 20, kMediumLabelIntSize);
    channelSourceComboBox.setBounds (channelSourceLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = fileNameSelectLabel.getBottom () + 3;

    // column one
    bitsLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    bitsTextEditor.setBounds (bitsLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = bitsTextEditor.getBottom () + 3;
    rateLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    rateComboBox.setBounds (rateLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    // only one of these is visible for a given channel
    // Speed for Channels 1-5
    yOffset = rateComboBox.getBottom () + 3;
    speedLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    speedTextEditor.setBounds (speedLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    // Quantize for Channels 6-8
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
    yOffset = levelTextEditor.getBottom () + 3;
    reverseLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    reverseButton.setBounds (reverseLabel.getRight () + 3, yOffset, fieldWidth, kMediumLabelIntSize + 2);
    xOffset += columnWidth + spaceBetweenColumns;
    yOffset = fileNameSelectLabel.getBottom () + 3;
    //yOffset = kInitialYOffset;
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
    startCueLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    startCueTextEditor.setBounds (startCueLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = startCueTextEditor.getBottom () + 3;
    loopCueLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    loopCueTextEditor.setBounds (loopCueLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = loopCueTextEditor.getBottom () + 3;
    endCueLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    endCueTextEditor.setBounds (endCueLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);

    // column three
    xOffset += columnWidth + spaceBetweenColumns;
    yOffset = fileNameSelectLabel.getBottom () + 3;
    chokeLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    chokeComboBox.setBounds (chokeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = chokeComboBox.getBottom () + 3;
    eTrigLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    eTrigComboBox.setBounds (eTrigLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = eTrigComboBox.getBottom () + 3;
    stepsLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    stepsComboBox.setBounds (stepsLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = stepsComboBox.getBottom () + 3;
    outputLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    outputComboBox.setBounds (outputLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = outputComboBox.getBottom () + 3;
    cueRandomLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    cueRandomButton.setBounds (cueRandomLabel.getRight () + 3, yOffset, fieldWidth, kMediumLabelIntSize + 2);
    yOffset = cueRandomButton.getBottom () + 3;
    cueStepLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    cueStepButton.setBounds (cueStepLabel.getRight () + 3, yOffset, fieldWidth, kMediumLabelIntSize + 2);

    loopPointsView.setBounds (outputComboBox.getRight () + spaceBetweenColumns, outputComboBox.getY (), columnWidth * 2, reverseButton.getY () - outputComboBox.getY () - 5);
    oneShotPlayButton.setBounds (loopPointsView.getX () + 5, loopPointsView.getY () - 3 - kMediumLabelIntSize, 35, kMediumLabelIntSize);
    loopPlayButton.setBounds (oneShotPlayButton.getRight () + 3, oneShotPlayButton.getY (), 35, kMediumLabelIntSize);

    // column four
//     xOffset += columnWidth + spaceBetweenColumns;
//     yOffset = kInitialYOffset;

    // LOWER PANE VIEW BUTTONS
    cueSetViewButton.setBounds (getWidth () - 40 - 60 - 20 - 60, reverseButton.getY (), fieldWidth, kParameterLineHeight);
    cvAssignViewButton.setBounds (cueSetViewButton.getRight () + 10, reverseButton.getY (), fieldWidth, kParameterLineHeight);

    // CUE SET BUTTONS
    addCueSetButton.setBounds (endCueTextEditor.getRight () + 60, reverseButton.getY (), fieldWidth, kParameterLineHeight);
    deleteCueSetButton.setBounds (addCueSetButton.getRight () + 10, addCueSetButton.getY (), fieldWidth, kParameterLineHeight);
    const auto kHeightOfCueSetButton { 20 };
    const auto kWidthOfCueSetButton { 30 };
    const auto kWidthOfWaveformEditor { 962 };
    xOffset = xInitialOffSet;
    yOffset = reverseButton.getBottom () + 10 + kHeightOfCueSetButton;
    // WAVEFORM
    waveformDisplay.setBounds (xOffset, yOffset, kWidthOfWaveformEditor, getHeight () - yOffset - (kHeightOfCueSetButton * 2));
    // CUE SET TABS
    for (auto cueSetIndex { 0 }; cueSetIndex < cueSetButtons.size () / 2; ++cueSetIndex)
    {
        const auto buttonX { xOffset + cueSetIndex * kWidthOfCueSetButton };
        cueSetButtons [cueSetIndex].setBounds (buttonX, waveformDisplay.getY () - kHeightOfCueSetButton, kWidthOfCueSetButton, kHeightOfCueSetButton);
        cueSetButtons [cueSetIndex + 32].setBounds (buttonX, waveformDisplay.getBottom (), kWidthOfCueSetButton, kHeightOfCueSetButton);
    }

    cvAssignEditor.setBounds (cueSetButtons [0].getX (), cueSetButtons [0].getY (), cueSetButtons [63].getRight () - cueSetButtons [0].getX (), cueSetButtons [63].getBottom () - cueSetButtons [0].getY ());
}

void ChannelEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void ChannelEditorComponent::initOutputComboBox ()
{
    outputComboBox.clear (juce::NotificationType::dontSendNotification);
    if (squidChannelProperties.getChannelIndex () < 4)
    {
        outputComboBox.addItem ("1-2", 1);
        outputComboBox.addItem ("3-4", 2);
    }
    else
    {
        outputComboBox.addItem ("5-6", 1);
        outputComboBox.addItem ("7-8", 2);
    }
}

void ChannelEditorComponent::setLowerPaneView (LowerPaneView whichView)
{
    auto setCuetSetVisibility = [this] (bool isVisible)
    {
        waveformDisplay.setVisible (isVisible);
        for (auto cueSetIndex { 0 }; cueSetIndex < cueSetButtons.size (); ++cueSetIndex)
            cueSetButtons [cueSetIndex].setVisible (isVisible);
    };
    if (whichView == LowerPaneView::cueSets)
    {
        setCuetSetVisibility (true);
        cvAssignEditor.setVisible (false);
    }
    else
    {
        setCuetSetVisibility (false);
        cvAssignEditor.setVisible (true);
    }
}
