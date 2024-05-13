#include "ChannelEditorComponent.h"
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

static const auto kScaleMax { 65535. };
static const auto kScaleStep { kScaleMax / 100 };

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
    stepsComboBox.setLookAndFeel (nullptr);
    eTrigComboBox.setLookAndFeel (nullptr);
    chokeComboBox.setLookAndFeel (nullptr);
    loopModeComboBox.setLookAndFeel (nullptr);
    filterTypeComboBox.setLookAndFeel (nullptr);
    quantComboBox.setLookAndFeel (nullptr);
    rateComboBox.setLookAndFeel (nullptr);
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
    // BITS
    setupLabel (bitsLabel, "BITS", kMediumLabelSize, juce::Justification::centred);
    bitsTextEditor.getMinValueCallback = [this] () { return 1; };
    bitsTextEditor.getMaxValueCallback = [this] () { return 16; };
    bitsTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    bitsTextEditor.updateDataCallback = [this] (int value) { bitsUiChanged (value); };
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
        squidChannelProperties.setRate (std::clamp (rateComboBox.getSelectedItemIndex () + scrollAmount, 0, rateComboBox.getNumItems () - 1), true);
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
    setupButton (reverseButton, "REVERSE", "Reverse", [this] () { reverseUiChanged (reverseButton.getToggleState ()); }); // 0-1
    // START
    setupLabel (startCueLabel, "START", kMediumLabelSize, juce::Justification::centred);
    startCueTextEditor.getMinValueCallback = [this] () { return 0; };
    startCueTextEditor.getMaxValueCallback = [this] () { return 1000; }; // TODO tie in the sample length here
    startCueTextEditor.toStringCallback = [this] (juce::int64 value) { return juce::String (value); };
    startCueTextEditor.updateDataCallback = [this] (juce::int64 value) { startCueUiChanged (value); };
    startCueTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
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
            const auto newValue { (squidChannelProperties.getStartCue () / 2) + (multiplier * direction) };
            startCueTextEditor.setValue (newValue);
    };
    setupTextEditor (startCueTextEditor, juce::Justification::centred, 0, "0123456789", "Start"); // 0 - sample length?
    // LOOP
    setupLabel (loopCueLabel, "LOOP", kMediumLabelSize, juce::Justification::centred);
    loopCueTextEditor.getMinValueCallback = [this] () { return 0; };
    loopCueTextEditor.getMaxValueCallback = [this] () { return 1000; }; // TODO tie in the sample length here
    loopCueTextEditor.toStringCallback = [this] (juce::int64 value) { return juce::String (value); };
    loopCueTextEditor.updateDataCallback = [this] (juce::int64 value) { loopCueUiChanged (value); };
    loopCueTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
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
            const auto newValue { (squidChannelProperties.getLoopCue () / 2) + (multiplier * direction) };
            loopCueTextEditor.setValue (newValue);
    };
    setupTextEditor (loopCueTextEditor, juce::Justification::centred, 0, "0123456789", "Loop"); // 0 - sample length?, or sampleStart - sampleEnd
    // END
    setupLabel (endCueLabel, "END", kMediumLabelSize, juce::Justification::centred);
    endCueTextEditor.getMinValueCallback = [this] () { return 0; };
    endCueTextEditor.getMaxValueCallback = [this] () { return 1000; }; // TODO tie in the sample length here
    endCueTextEditor.toStringCallback = [this] (juce::int64 value) { return juce::String (value); };
    endCueTextEditor.updateDataCallback = [this] (juce::int64 value) { endCueUiChanged (value); };
    endCueTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
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
            const auto newValue { (squidChannelProperties.getEndCue () / 2) + (multiplier * direction) };
            endCueTextEditor.setValue (newValue);
    };
    setupTextEditor (endCueTextEditor, juce::Justification::centred, 0, "0123456789", "End"); // sampleStart - sample length
    // CHOKE
    setupLabel (chokeLabel, "CHOKE", kMediumLabelSize, juce::Justification::centred);
    {
        for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
        {
            // TODO - only the 'other channels' should show in this list
            //        I think we want to display 'Off' for the current channel
            //if (channelIndex != curChannelIndex)
            chokeComboBox.addItem ("C" + juce::String (curChannelIndex + 1), curChannelIndex + 1);
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
    waveformDisplay.onStartPointChange = [this] (int startPoint)
    {
        const auto startCueByteOffset { startPoint * 2 };
        //const auto curCueSetIndex { squidChannelProperties.getCurCueSet () };
        squidChannelProperties.setCuePoints (curCueSetIndex, startCueByteOffset, squidChannelProperties.getLoopCueSet (curCueSetIndex), squidChannelProperties.getEndCueSet (curCueSetIndex));
        squidChannelProperties.setStartCue (startCueByteOffset, true);
    };
    waveformDisplay.onLoopPointChange = [this] (int loopPoint)
    {
        const auto loopCueByteOffset { loopPoint * 2 };
        //const auto curCueSetIndex { squidChannelProperties.getCurCueSet () };
        squidChannelProperties.setCuePoints (curCueSetIndex, squidChannelProperties.getStartCueSet (curCueSetIndex), loopCueByteOffset, squidChannelProperties.getEndCueSet (curCueSetIndex));
        squidChannelProperties.setLoopCue (loopCueByteOffset, true);
    };
    waveformDisplay.onEndPointChange = [this] (int endPoint)
    {
        const auto endCueByteOffset { endPoint * 2 };
        //const auto curCueSetIndex { squidChannelProperties.getCurCueSet () };
        squidChannelProperties.setCuePoints (curCueSetIndex, squidChannelProperties.getStartCueSet (curCueSetIndex), squidChannelProperties.getLoopCueSet (curCueSetIndex), endCueByteOffset);
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
    waveformDisplay.setCuePoints (squidChannelProperties.getStartCueSet (cueSetIndex) / 2, squidChannelProperties.getLoopCueSet (cueSetIndex) / 2, squidChannelProperties.getEndCueSet (cueSetIndex) / 2);
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

void ChannelEditorComponent::init (juce::ValueTree squidChannelPropertiesVT)
{
//     PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
//     runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);
//     runtimeRootProperties.onSystemRequestedQuit = [this] ()
//         {
            // TODO - add code to check if data needs to be saved before exiting
//         runtimeRootProperties.setPreferredQuitState (RuntimeRootProperties::QuitState::idle, false);
//         overwritePresetOrCancel ([this] ()
//         {
//             juce::MessageManager::callAsync ([this] () { runtimeRootProperties.setQuitState (RuntimeRootProperties::QuitState::now, false); });
//         }, [this] ()
//         {
//             // do nothing
//         });
//        };

//     appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
//     appProperties.onMostRecentFileChange = [this] (juce::String fileName)
//         {
//             //         //DebugLog ("Assimil8orEditorComponent", "Assimil8orEditorComponent::init/appProperties.onMostRecentFileChange: " + fileName);
//             //         //dumpStacktrace (-1, [this] (juce::String logLine) { DebugLog ("Assimil8orEditorComponent", logLine); });
//             waveformDisplay.init (fileName);
//         };

    //     PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
    //     unEditedPresetProperties.wrap (presetManagerProperties.getPreset ("unedited"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    //     presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);

    squidChannelProperties.wrap (squidChannelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);

    cvAssignEditor.init (squidChannelPropertiesVT);

    initCueSetTabs ();
    setCurCue (squidChannelProperties.getCurCueSet ());

    // put initial data into the UI
    attackDataChanged (squidChannelProperties.getAttack ());
    chokeDataChanged (squidChannelProperties.getChoke ());
    bitsDataChanged (squidChannelProperties.getBits ());
    decayDataChanged (squidChannelProperties.getDecay ());
    endCueDataChanged (squidChannelProperties.getEndCue ());
    eTrigDataChanged (squidChannelProperties.getETrig ());
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
    setFilterEnableState ();
    setCueEditButtonsEnableState ();
}

void ChannelEditorComponent::initializeCallbacks ()
{
    jassert (squidChannelProperties.isValid ());
    squidChannelProperties.onAttackChange = [this] (int attack) { attackDataChanged (attack); };
    squidChannelProperties.onBitsChange = [this] (int bits) { bitsDataChanged (bits); };
    squidChannelProperties.onChokeChange = [this] (int choke) { chokeDataChanged (choke); };
    squidChannelProperties.onCurCueSetChange = [this] (int cueSetIndex) { setCurCue (cueSetIndex); };
    squidChannelProperties.onDecayChange = [this] (int decay) { decayDataChanged (decay); };
    squidChannelProperties.onEndCueChange = [this] (int endCue) { endCueDataChanged (endCue); };
    squidChannelProperties.onEndCueSetChange = [this] (int cueIndex, int endCue)
    {
        if (cueIndex == curCueSetIndex)
            waveformDisplay.setCuePoints (squidChannelProperties.getStartCueSet (curCueSetIndex) / 2, squidChannelProperties.getLoopCueSet (curCueSetIndex) / 2, endCue / 2);
    };
    squidChannelProperties.onETrigChange = [this] (int eTrig) { eTrigDataChanged (eTrig); };
    squidChannelProperties.onFilterTypeChange = [this] (int filter) { filterTypeDataChanged (filter); };
    squidChannelProperties.onFilterFrequencyChange = [this] (int filterFrequency) { filterFrequencyDataChanged (filterFrequency); };
    squidChannelProperties.onFilterResonanceChange = [this] (int filterResonance) { filterResonanceDataChanged (filterResonance); };
    squidChannelProperties.onLevelChange = [this] (int level) { levelDataChanged (level); };
    squidChannelProperties.onLoopCueChange = [this] (int loopCue) { loopCueDataChanged (loopCue); };
    squidChannelProperties.onLoopCueSetChange = [this] (int cueIndex, int loopCue)
    {
        if (cueIndex == curCueSetIndex)
            waveformDisplay.setCuePoints (squidChannelProperties.getStartCueSet (curCueSetIndex) / 2, loopCue / 2, squidChannelProperties.getEndCueSet (curCueSetIndex) / 2);
    };
    squidChannelProperties.onLoopModeChange = [this] (int loopMode) { loopModeDataChanged (loopMode); };
    squidChannelProperties.onNumCueSetsChange = [this] (int numCueSets)
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
            waveformDisplay.setCuePoints (startCue / 2, squidChannelProperties.getLoopCueSet (curCueSetIndex) / 2, squidChannelProperties.getEndCueSet (curCueSetIndex) / 2);
    };
    squidChannelProperties.onStepsChange = [this] (int steps) { stepsDataChanged (steps); };
    squidChannelProperties.onXfadeChange = [this] (int xfade) { xfadeDataChanged (xfade); };
}

void ChannelEditorComponent::appendCueSet ()
{
    const auto numCueSets { squidChannelProperties.getNumCueSets () };
    jassert (numCueSets < 64);
    if (numCueSets == 64)
        return;
    const auto newCueSetIndex { numCueSets };
    squidChannelProperties.setCuePoints (newCueSetIndex, squidChannelProperties.getStartCueSet (numCueSets - 1), squidChannelProperties.getLoopCueSet (numCueSets - 1), squidChannelProperties.getEndCueSet (numCueSets - 1));
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
    bitsTextEditor.setText (juce::String (bits), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::chokeDataChanged (int choke)
{
    chokeComboBox.setSelectedItemIndex (choke, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::decayDataChanged (int decay)
{
    decayTextEditor.setText (juce::String (getUiValue (decay)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::endCueDataChanged (juce::int64 endCue)
{
    endCueTextEditor.setText (juce::String (endCue / 2), juce::NotificationType::dontSendNotification);
    waveformDisplay.setCuePoints (squidChannelProperties.getStartCueSet (curCueSetIndex) / 2, squidChannelProperties.getLoopCueSet (curCueSetIndex) / 2, endCue / 2);
}

void ChannelEditorComponent::eTrigDataChanged (int eTrig)
{
    eTrigComboBox.setSelectedItemIndex (eTrig, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::filterTypeDataChanged (int filterType)
{
    filterTypeComboBox.setSelectedItemIndex (filterType, juce::NotificationType::dontSendNotification);
    setFilterEnableState ();
}

void ChannelEditorComponent::filterFrequencyDataChanged (int filterFrequency)
{
    filterFrequencyTextEditor.setText (juce::String (filterFrequency), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::filterResonanceDataChanged (int filterResonance)
{
    filterResonanceTextEditor.setText (juce::String (getUiValue (filterResonance)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::levelDataChanged (int level)
{
    levelTextEditor.setText (juce::String (getUiValue (level)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::loopCueDataChanged (juce::int64 loopCue)
{
    loopCueTextEditor.setText (juce::String (loopCue / 2), false);
    waveformDisplay.setCuePoints (squidChannelProperties.getStartCueSet (curCueSetIndex) / 2, loopCue / 2, squidChannelProperties.getEndCueSet (curCueSetIndex) / 2);
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

void ChannelEditorComponent::startCueDataChanged (juce::int64 startCue)
{
    startCueTextEditor.setText (juce::String (startCue / 2), juce::NotificationType::dontSendNotification);
    waveformDisplay.setCuePoints (startCue / 2, squidChannelProperties.getLoopCueSet (curCueSetIndex) / 2, squidChannelProperties.getEndCueSet (curCueSetIndex) / 2);
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

void ChannelEditorComponent::chokeUiChanged (int choke)
{
    squidChannelProperties.setChoke (choke, false);
}

void ChannelEditorComponent::decayUiChanged (int decay)
{
    const auto newDecayValue { getInternalValue (decay) };
    squidChannelProperties.setDecay (newDecayValue, false);
}

void ChannelEditorComponent::endCueUiChanged (juce::int64 endCue)
{
    const auto endCueByteOffset { endCue * 2 };
    squidChannelProperties.setEndCue (endCueByteOffset, false);

    squidChannelProperties.setCuePoints (curCueSetIndex, squidChannelProperties.getStartCueSet (curCueSetIndex), squidChannelProperties.getLoopCueSet (curCueSetIndex), endCueByteOffset);
    waveformDisplay.setCuePoints (squidChannelProperties.getStartCueSet (curCueSetIndex) / 2, squidChannelProperties.getLoopCueSet (curCueSetIndex) / 2, endCueByteOffset / 2);
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
    squidChannelProperties.setFilterFrequency (filterFrequency, false);
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

void ChannelEditorComponent::loopCueUiChanged (juce::int64 loopCue)
{
    const auto loopCueByteOffset { loopCue * 2 };
    squidChannelProperties.setLoopCue (loopCueByteOffset, false);

    squidChannelProperties.setCuePoints (curCueSetIndex, squidChannelProperties.getStartCueSet (curCueSetIndex), loopCueByteOffset, squidChannelProperties.getEndCueSet (curCueSetIndex));
    waveformDisplay.setCuePoints (squidChannelProperties.getStartCueSet (curCueSetIndex) / 2, loopCueByteOffset / 2, squidChannelProperties.getEndCueSet (curCueSetIndex) / 2);
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

void ChannelEditorComponent::startCueUiChanged (juce::int64 startCue)
{
    const auto startCueByteOffset { startCue * 2 };
    squidChannelProperties.setStartCue (startCueByteOffset, false);

    squidChannelProperties.setCuePoints (curCueSetIndex, startCueByteOffset, squidChannelProperties.getLoopCueSet (curCueSetIndex), squidChannelProperties.getEndCueSet (curCueSetIndex));
    waveformDisplay.setCuePoints (startCueByteOffset / 2, squidChannelProperties.getLoopCueSet (curCueSetIndex) / 2, squidChannelProperties.getEndCueSet (curCueSetIndex) / 2);
}

void ChannelEditorComponent::stepsUiChanged (int steps)
{
    squidChannelProperties.setSteps (steps, false);
}

void ChannelEditorComponent::xfadeUiChanged (int xfade)
{
    squidChannelProperties.setXfade (xfade, false);
}

void ChannelEditorComponent::resized ()
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

    // column three
    xOffset += columnWidth + 10;
    yOffset = kInitialYOffset;
    chokeLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    chokeComboBox.setBounds (chokeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = chokeComboBox.getBottom () + 3;
    eTrigLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    eTrigComboBox.setBounds (eTrigLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = eTrigComboBox.getBottom () + 3;
    stepsLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    stepsComboBox.setBounds (stepsLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);

    // column four
    xOffset += columnWidth + 10;
    yOffset = kInitialYOffset;
    loadButton.setBounds (xOffset, yOffset, fieldWidth, kParameterLineHeight);

    // LOWER PANE VIEW BUTTONS
    cueSetViewButton.setBounds (getWidth () - 40 - 60 - 20 - 60, endCueTextEditor.getY (), fieldWidth, kParameterLineHeight);
    cvAssignViewButton.setBounds (cueSetViewButton.getRight () + 10, addCueSetButton.getY (), fieldWidth, kParameterLineHeight);

    // CUE SET BUTTONS
    addCueSetButton.setBounds (endCueTextEditor.getRight () + 60, endCueTextEditor.getY (), fieldWidth, kParameterLineHeight);
    deleteCueSetButton.setBounds (addCueSetButton.getRight () + 10, addCueSetButton.getY (), fieldWidth, kParameterLineHeight);
    const auto kHeightOfCueSetButton { 20 };
    const auto kWidthOfCueSetButton { 30 };
    const auto kWidthOfWaveformEditor { 962 };
    xOffset = xInitialOffSet;
    yOffset = levelTextEditor.getBottom () + 10 + kHeightOfCueSetButton;
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
