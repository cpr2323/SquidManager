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

ChannelEditorComponent::ChannelEditorComponent ()
{
    setOpaque (true);
    toolsButton.setButtonText ("TOOLS");
    toolsButton.setTooltip ("Channel Tools");
    toolsButton.onClick = [this] ()
    {
        auto* popupMenuLnF { new juce::LookAndFeel_V4 };
        popupMenuLnF->setColour (juce::PopupMenu::ColourIds::headerTextColourId, juce::Colours::white.withAlpha (0.3f));
        juce::PopupMenu editMenu;
        editMenu.setLookAndFeel (popupMenuLnF);
        editMenu.addSectionHeader ("Channel " + juce::String (squidChannelProperties.getChannelIndex () + 1));
        editMenu.addSeparator ();
        {
            // Clone
            juce::PopupMenu cloneMenu;
            {
                auto cloneSettingsMenu { editManager->createChannelInteractionMenu (squidChannelProperties.getChannelIndex (), "To",
                    [this] (SquidChannelProperties& destChannelProperties)
                    {
                        destChannelProperties.copyFrom (squidChannelProperties.getValueTree(), SquidChannelProperties::mainSettings, SquidChannelProperties::CheckIndex::yes);
                    },
                    [this] (SquidChannelProperties&) { return true; },
                    [this] (SquidChannelProperties&) { return true; }
                ) };
                cloneMenu.addSubMenu ("Main Settings", cloneSettingsMenu);
            }
            {
                auto cloneSampleAndSettingsMenu { editManager->createChannelInteractionMenu (squidChannelProperties.getChannelIndex (), "To",
                    [this] (SquidChannelProperties& destChannelProperties)
                    {
                        destChannelProperties.copyFrom (squidChannelProperties.getValueTree (), SquidChannelProperties::all, SquidChannelProperties::CheckIndex::yes);
                        if (auto currentSrcFile { juce::File (squidChannelProperties.getSampleFileName ()) }; currentSrcFile != juce::File ())
                        {
                            auto destFile { juce::File (destChannelProperties.getSampleFileName()).withFileExtension ("._wav") };
                            auto destChannelDirectory { destFile.getParentDirectory () };
                            if (! destChannelDirectory.exists ())
                                destChannelDirectory.createDirectory ();
                            if (destFile.exists ())
                                destFile.deleteFile ();
                            currentSrcFile.copyFileTo (destFile);
                            destChannelProperties.setSampleFileName (destFile.getFullPathName (), false);
                        }
                    },
                    [this] (SquidChannelProperties&) { return true; },
                    [this] (SquidChannelProperties&) { return true; }
                ) };
                cloneMenu.addSubMenu ("Entire Channel", cloneSampleAndSettingsMenu);
            }
            {
                juce::PopupMenu cloneCvAssignsMenu;
                {
                    for (auto cvSrcIndex { 0 }; cvSrcIndex < 8; ++cvSrcIndex)
                    {
                        juce::PopupMenu cloneCvAssignsDestCatagoryMenu;
                        juce::PopupMenu cloneCvAssignsDestCvMenu;
                        for (auto cvDestIndex { 0 }; cvDestIndex < 8; ++cvDestIndex)
                        {
                            if (cvDestIndex != cvSrcIndex)
                                cloneCvAssignsDestCvMenu.addItem ("CV " + juce::String (cvDestIndex + 1), true, false, [this, cvSrcIndex, cvDestIndex] ()
                                {
                                    editManager->cloneCvAssigns (squidChannelProperties.getChannelIndex (), cvSrcIndex, squidChannelProperties.getChannelIndex (), cvDestIndex);
                                });
                        }
                        cloneCvAssignsDestCatagoryMenu.addSubMenu ("CV", cloneCvAssignsDestCvMenu);

                        juce::PopupMenu cloneCvAssignsChannelDestMenu;
                        for (auto channelDestIndex { 0 }; channelDestIndex < 8; ++channelDestIndex)
                        {
                            if (channelDestIndex != squidChannelProperties.getChannelIndex ())
                            {
                                juce::PopupMenu cloneCvAssignsChannelDestCvMenu;
                                for (auto cvDestIndex { 0 }; cvDestIndex < 8; ++cvDestIndex)
                                    cloneCvAssignsChannelDestCvMenu.addItem ("CV " + juce::String (cvDestIndex + 1), true, false, [this, cvSrcIndex, channelDestIndex, cvDestIndex] ()
                                    {
                                        editManager->cloneCvAssigns (squidChannelProperties.getChannelIndex (), cvSrcIndex, channelDestIndex, cvDestIndex);
                                    });
                                cloneCvAssignsChannelDestMenu.addSubMenu ("Channel " + juce::String (channelDestIndex + 1), cloneCvAssignsChannelDestCvMenu);
                            }
                        }
                        cloneCvAssignsDestCatagoryMenu.addSubMenu ("Channel", cloneCvAssignsChannelDestMenu);

                        cloneCvAssignsMenu.addSubMenu ("CV " + juce::String (cvSrcIndex + 1), cloneCvAssignsDestCatagoryMenu);
                    }
                }
                cloneMenu.addSubMenu ("CV Assign", cloneCvAssignsMenu);
            }
            editMenu.addSubMenu ("Clone", cloneMenu);
        }
        {
            auto swapMenu { editManager->createChannelInteractionMenu (squidChannelProperties.getChannelIndex (), "With", 
                [this] (SquidChannelProperties& destChannelProperties)
                {
                    editManager->swapChannels (squidChannelProperties.getChannelIndex (), destChannelProperties.getChannelIndex ());
                },
                [this] (SquidChannelProperties&) { return true; },
                [this] (SquidChannelProperties&) { return false; } 
            )};

            editMenu.addSubMenu("Swap", swapMenu);
        }
        editMenu.addItem ("Clear Cue Sets", true, false, [this, channelIndex = squidChannelProperties.getChannelIndex ()] ()
        {
            squidChannelProperties.setCurCueSet (0, false);
            for (auto cueSetCount { squidChannelProperties.getNumCueSets () }; cueSetCount > 1; --cueSetCount)
                squidChannelProperties.removeCueSet (cueSetCount - 1);
            const auto endOffset { SquidChannelProperties::sampleOffsetToByteOffset (squidChannelProperties.getSampleDataNumSamples ()) };
            squidChannelProperties.setStartCue (0, true);
            squidChannelProperties.setLoopCue (0, true);
            squidChannelProperties.setEndCue (endOffset, true);
            squidChannelProperties.setCueSetPoints (0, 0, 0, SquidChannelProperties::sampleOffsetToByteOffset (squidChannelProperties.getSampleDataNumSamples ()));
        });
        editMenu.addItem ("Default", true, false, [this, channelIndex = squidChannelProperties.getChannelIndex ()] ()
        {
            editManager->setChannelDefaults (squidChannelProperties.getChannelIndex ());
        });
        editMenu.addItem ("Revert", true, false, [this, channelIndex = squidChannelProperties.getChannelIndex ()] ()
        {
            editManager->setChannelUnedited (squidChannelProperties.getChannelIndex ());
        });
        editMenu.showMenuAsync ({}, [this, popupMenuLnF] (int) { delete popupMenuLnF; });

    };
    addAndMakeVisible (toolsButton);
    addAndMakeVisible (sampleLengthLabel);
    setupComponents ();
}

ChannelEditorComponent::~ChannelEditorComponent ()
{
    addCueSetButton.setLookAndFeel (nullptr);
    deleteCueSetButton.setLookAndFeel (nullptr);
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
    setupLabel (sampleFileNameLabel, "FILE", kMediumLabelSize, juce::Justification::centred);
    sampleFileNameSelectLabel.setTooltip ("Sample File Name. Click to open file browser, or drag a file onto the editor. If the file name is dimmed out this channel is using a sample from the Channel specified in the SOURCE parameter.");
    sampleFileNameSelectLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
    sampleFileNameSelectLabel.setColour (juce::Label::ColourIds::backgroundColourId, juce::Colours::black);
    sampleFileNameSelectLabel.setOutline (juce::Colours::white);
    sampleFileNameSelectLabel.onFilesSelected = [this] (const juce::StringArray& files)
    {
        if (! handleSampleAssignment (files[0]))
        {
            // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
        }
    };
    sampleFileNameSelectLabel.onPopupMenuCallback = [this] ()
    {
        // Clone
        // Revert
    };
    setupLabel (sampleFileNameSelectLabel, "", 15.0, juce::Justification::centredLeft);

    setupLabel (channelSourceLabel, "SOURCE", kMediumLabelSize, juce::Justification::centred);
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
    channelSourceComboBox.setTooltip ("Channel Reference. Select the channel for which this channel will get it's sample from. Can be changed on the module by holding the Chan button and turning the program knob.");
    channelSourceComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        squidChannelProperties.setChannelSource (static_cast<uint8_t>(std::clamp (channelSourceComboBox.getSelectedItemIndex () + scrollAmount, 0, channelSourceComboBox.getNumItems () - 1)), true);
    };
    channelSourceComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex(),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setChannelSource (squidChannelProperties.getChannelSource (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setChannelSource (defaultChannelProperties.getChannelSource (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setChannelSource (uneditedChannelProperties.getChannelSource (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (channelSourceComboBox, "SampleChannel", [this] () { channelSourceUiChanged (static_cast<uint8_t>(channelSourceComboBox.getSelectedItemIndex ())); });

    // BITS
    setupLabel (bitsLabel, "BITS", kMediumLabelSize, juce::Justification::centred);
    bitsTextEditor.setTooltip ("Bits. Adjust the bit depth of playback. Can be from 1 to 16. Can be changed on the module in the Quality settings.");
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
    bitsTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setBits (squidChannelProperties.getBits (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setBits (defaultChannelProperties.getBits (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setBits (uneditedChannelProperties.getBits (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (bitsTextEditor, juce::Justification::centred, 0, "0123456789", "Bits"); // 1-16
    // RATE
    setupLabel (rateLabel, "RATE", kMediumLabelSize, juce::Justification::centred);
    rateComboBox.setTooltip ("Rate. Adjusts the playback rate of the sample in khz. Values are 4, 6, 7, 9, 11, 14, 22, 44. Can be changed on the module in the Quality settings.");
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
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction};
        squidChannelProperties.setRate (rateComboBox.getItemId (std::clamp (rateComboBox.getSelectedItemIndex () + scrollAmount, 0, rateComboBox.getNumItems () - 1)) - 1, true);
    };
    rateComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setRate (squidChannelProperties.getRate (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setRate (defaultChannelProperties.getRate (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setRate (uneditedChannelProperties.getRate (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (rateComboBox, "Rate", [this] () { rateUiChanged (rateComboBox.getSelectedId () - 1); }); // 4,6,7,9,11,14,22,44
    // SPEED
    setupLabel (speedLabel, "SPEED", kMediumLabelSize, juce::Justification::centred);
    speedTextEditor.setTooltip ("Speed. Linear playback speed control. From 1 to 100, where 50 is normal speed. Available for Channels 1-5. Can be changed on the module in the Quality settings.");
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
    speedTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setSpeed (squidChannelProperties.getSpeed (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setSpeed (defaultChannelProperties.getSpeed (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setSpeed (uneditedChannelProperties.getSpeed (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (speedTextEditor, juce::Justification::centred, 0, "0123456789", "Speed"); // 1 - 99 (50 is normal, below that is negative speed? above is positive?)
    // QUANTIZE
    setupLabel (quantLabel, "QUANT", kMediumLabelSize, juce::Justification::centred);
    quantComboBox.setTooltip ("Quantization. This applies pitch quantisation to the sample playback (speed). Note the original pitch of the sample file serves as the scale's root note. Available for Channels 6-8.");
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
    quantComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setQuant (squidChannelProperties.getQuant (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setQuant (defaultChannelProperties.getQuant (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setQuant (uneditedChannelProperties.getQuant (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (quantComboBox, "Quantize", [this] () { quantUiChanged (quantComboBox.getSelectedId () - 1); }); // 0-14 (Off, 12, OT, MA, mi, Hm, PM, Pm, Ly, Ph, Jp, P5, C1, C4, C5)
    // FILTER TYPE
    setupLabel (filterTypeLabel, "FILTER", kMediumLabelSize, juce::Justification::centred);
    filterTypeComboBox.setTooltip ("Filer Type. Enables the resonant multimode filter and the corresponding Frequency and Resonance parameters. Filer Types: Off, Low Pass, Band Pass, Notch, and High Pass.");
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
    filterTypeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setFilterType (squidChannelProperties.getFilterType (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setFilterType (defaultChannelProperties.getFilterType (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setFilterType (uneditedChannelProperties.getFilterType (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (filterTypeComboBox, "Filter", [this] () { filterTypeUiChanged (filterTypeComboBox.getSelectedId () - 1); }); // Off, LP, BP, NT, HP (0-4)
    // FILTER FREQUENCY
    setupLabel (filterFrequencyLabel, "FREQ", kMediumLabelSize, juce::Justification::centred);
    filterFrequencyTextEditor.setTooltip ("Filter Frequency. Adjusts the cut off frequency of the filter, only appears when a filter type is selected.");
    filterFrequencyTextEditor.getMinValueCallback = [this] () { return 0; };
    filterFrequencyTextEditor.getMaxValueCallback = [this] () { return 99; };
    filterFrequencyTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    filterFrequencyTextEditor.updateDataCallback = [this] (int value) { filterFrequencyUiChanged (value); };
    filterFrequencyTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
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
        const auto newValue { filterFrequencyTextEditor.getText().getIntValue() + (multiplier * direction) };
        filterFrequencyTextEditor.setValue (newValue);
    };
    filterFrequencyTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setFilterFrequency (squidChannelProperties.getFilterFrequency (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setFilterFrequency (defaultChannelProperties.getFilterFrequency (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setFilterFrequency (uneditedChannelProperties.getFilterFrequency(), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (filterFrequencyTextEditor, juce::Justification::centred, 0, "0123456789", "Frequency"); // 1-99?

    // FILTER RESONANCE
    setupLabel (filterResonanceLabel, "RESO", kMediumLabelSize, juce::Justification::centred);
    filterResonanceTextEditor.setTooltip ("Filter Resonance. Adjust the resonant peak of the filter, only appears when a filter type is selected.");
    filterResonanceTextEditor.getMinValueCallback = [this] () { return 0; };
    filterResonanceTextEditor.getMaxValueCallback = [this] () { return 99; };
    filterResonanceTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    filterResonanceTextEditor.updateDataCallback = [this] (int value) { filterResonanceUiChanged (value); };
    filterResonanceTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
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
        const auto newValue { getUiValue (squidChannelProperties.getFilterResonance ()) + (multiplier * direction) };
        filterResonanceTextEditor.setValue (newValue);
    };
    filterResonanceTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setFilterResonance (squidChannelProperties.getFilterResonance (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setFilterResonance (defaultChannelProperties.getFilterResonance (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setFilterResonance (uneditedChannelProperties.getFilterResonance (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (filterResonanceTextEditor, juce::Justification::centred, 0, "0123456789", "Resonance"); // 1-99?
    // LEVEL
    setupLabel (levelLabel, "LEVEL", kMediumLabelSize, juce::Justification::centred);
    levelTextEditor.setTooltip ("Level. Adjust the playback volume of a sample. At 50 is unity gain. below will attenuate, above increase. Use this setting to avoid digital clipping when mixed. Value defaults to 30 as to conservatively avoid digital clipping.");
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
    levelTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setLevel (squidChannelProperties.getLevel (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setLevel (defaultChannelProperties.getLevel (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setLevel (uneditedChannelProperties.getLevel (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (levelTextEditor, juce::Justification::centred, 0, "0123456789", "Level"); // 1-99
    // ATTACK
    setupLabel (attackLabel, "ATTACK", kMediumLabelSize, juce::Justification::centred);
    attackTextEditor.setTooltip ("Add a simple attack envelope to control volume at the beginning of sample playback. Behaves similar to Decay if the sample is set to loop.");
    attackTextEditor.getMinValueCallback = [this] () { return 0; };
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
    attackTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setAttack (squidChannelProperties.getAttack (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setAttack (defaultChannelProperties.getAttack (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setAttack (uneditedChannelProperties.getAttack (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (attackTextEditor, juce::Justification::centred, 0, "0123456789", "Attack"); // 0-99
    // DECAY
    setupLabel (decayLabel, "DECAY", kMediumLabelSize, juce::Justification::centred);
    decayTextEditor.setTooltip ("Add a simple decay envelope to fade out the volume of the sample. Decay time shortens as value rises. If a sample is set to loop the envelope will effect the loop as a whole (rather than single sample) with max decay time being 10 seconds.");
    decayTextEditor.getMinValueCallback = [this] () { return 0; };
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
    decayTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setDecay (squidChannelProperties.getDecay (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setDecay (defaultChannelProperties.getDecay (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setDecay (uneditedChannelProperties.getDecay (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (decayTextEditor, juce::Justification::centred, 0, "0123456789", "Decay"); // 0-99
    // LOOP MODE
    setupLabel (loopModeLabel, "LOOP MODE", kMediumLabelSize, juce::Justification::centred);
    {
        auto loopId { 1 };
        loopModeComboBox.addItem ("None", loopId++);
        loopModeComboBox.addItem ("Normal", loopId++);
        loopModeComboBox.addItem ("ZigZag", loopId++);
        loopModeComboBox.addItem ("Normal Gate", loopId++);
        loopModeComboBox.addItem ("ZigZag Gate", loopId++);
    }
    loopModeComboBox.setTooltip ("Loop Mode. Configures how looping will operate. Normal for forward playing. ZigZag plays alternatively forwards then backwards between loop and end points. Gate options indicate sample will only play & loop whilst the associated channels trigger input is held high (like a sustain). If Decay is set however, playback will move to this stage when the trigger goes low.");
    loopModeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    loopModeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        squidChannelProperties.setLoopMode (static_cast<uint8_t>(std::clamp (loopModeComboBox.getSelectedItemIndex () + scrollAmount, 0, loopModeComboBox.getNumItems () - 1)), true);
    };
    loopModeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setLoopMode (squidChannelProperties.getLoopMode (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setLoopMode (defaultChannelProperties.getLoopMode (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setLoopMode (uneditedChannelProperties.getLoopMode (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (loopModeComboBox, "LoopMode", [this] () { loopModeUiChanged (loopModeComboBox.getSelectedItemIndex ()); }); // none, normal, zigZag, gate, zigZagGate (0-4)
    // XFADE
    setupLabel (xfadeLabel, "XFADE", kMediumLabelSize, juce::Justification::centred);
    xfadeTextEditor.setTooltip ("Loop Crossfade. Adds a simple cross fade between the sample end and loop points as to smooth out loops.");
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
    xfadeTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setXfade (squidChannelProperties.getXfade (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setXfade (defaultChannelProperties.getXfade (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setXfade (uneditedChannelProperties.getXfade (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (xfadeTextEditor, juce::Justification::centred, 0, "0123456789", "XFade"); // 0 -99
    // REVERSE
    setupLabel (reverseLabel, "REVERSE", kMediumLabelSize, juce::Justification::centred);
    reverseButton.setTooltip ("Reverse. Reverses the sample");
    reverseButton.onClick = [this] () { reverseUiChanged (reverseButton.getToggleState ()); };
    reverseButton.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setReverse (squidChannelProperties.getReverse (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setReverse (defaultChannelProperties.getReverse (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setReverse (uneditedChannelProperties.getReverse (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (reverseButton);
    // START
    setupLabel (startCueLabel, "START", kMediumLabelSize, juce::Justification::centred);
    startCueTextEditor.setTooltip ("Cue Start. Sets the starting sample for playback.");
    startCueTextEditor.getMinValueCallback = [this] () { return 0; };
    startCueTextEditor.getMaxValueCallback = [this] () { return SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()); };
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
        if (valueOffset < 0 && std::abs (valueOffset) > static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ())))
            newValue = 0;
        else
            newValue = SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()) + valueOffset;
        if (newValue > static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
            loopCueTextEditor.setValue (newValue);
        startCueTextEditor.setValue (newValue);
    };
    startCueTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setStartCue (squidChannelProperties.getStartCue (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setStartCue (defaultChannelProperties.getStartCue (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setStartCue (uneditedChannelProperties.getStartCue (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (startCueTextEditor, juce::Justification::centred, 0, "0123456789", "Start"); // 0 - sample length?
    // LOOP
    setupLabel (loopCueLabel, "LOOP", kMediumLabelSize, juce::Justification::centred);
    loopCueTextEditor.setTooltip ("Cue Loop. Sets the loop starting sample for playback.");
    loopCueTextEditor.getMinValueCallback = [this] () { return SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()); };
    loopCueTextEditor.getMaxValueCallback = [this] () { return SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()); };
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
        if (valueOffset < 0 && std::abs (valueOffset) > static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
            newValue = 0;
        else
            newValue = SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ()) + valueOffset;
        loopCueTextEditor.setValue (newValue);
    };
    loopCueTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setLoopCue (squidChannelProperties.getLoopCue (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setLoopCue (defaultChannelProperties.getLoopCue (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setLoopCue(uneditedChannelProperties.getLoopCue (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (loopCueTextEditor, juce::Justification::centred, 0, "0123456789", "Loop"); // 0 - sample length?, or sampleStart - sampleEnd
    // END
    setupLabel (endCueLabel, "END", kMediumLabelSize, juce::Justification::centred);
    endCueTextEditor.setTooltip ("Cue End. Sets the end sample for playback and looping.");
    endCueTextEditor.getMinValueCallback = [this] () { return SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()); };
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
        if (valueOffset < 0 && std::abs (valueOffset) > static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ())))
            newValue = 0;
        else
            newValue = SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()) + valueOffset;
        if (newValue < static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
            loopCueTextEditor.setValue (newValue);
        endCueTextEditor.setValue (newValue);
    };
    endCueTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setEndCue (squidChannelProperties.getEndCue (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setEndCue (defaultChannelProperties.getEndCue (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setEndCue (uneditedChannelProperties.getEndCue (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
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
    chokeComboBox.setTooltip ("Choke. Select a channel that will stop playing when this channel plays.");
    chokeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    chokeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        squidChannelProperties.setChoke (static_cast<uint8_t>(std::clamp (chokeComboBox.getSelectedItemIndex () + scrollAmount, 0, chokeComboBox.getNumItems () - 1)), true);
    };
    chokeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setChoke (squidChannelProperties.getChoke (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setChoke (defaultChannelProperties.getChoke (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setChoke (uneditedChannelProperties.getChoke (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (chokeComboBox, "Choke", [this] () { chokeUiChanged (chokeComboBox.getSelectedItemIndex ()); }); // C1, C2, C3, C4, C5, C6, C7, C8
    // ETrig
    setupLabel (eTrigLabel, "EOS TRIG", kMediumLabelSize, juce::Justification::centred);
    eTrigComboBox.setTooltip ("EOS Trigger. Selecting a channel will start playback of that channel when this one ends. Selecting On will cause a trigger to happen on the Trigger Output.");
    eTrigComboBox.addItem ("Off", 1);
    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
        eTrigComboBox.addItem ("> " + juce::String (curChannelIndex + 1), curChannelIndex + 2);
    eTrigComboBox.addItem ("On", 10);
    eTrigComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    eTrigComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        squidChannelProperties.setETrig (static_cast<uint8_t>(std::clamp (eTrigComboBox.getSelectedItemIndex () + scrollAmount, 0, eTrigComboBox.getNumItems () - 1)), true);
    };
    eTrigComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setETrig (squidChannelProperties.getETrig (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setETrig (defaultChannelProperties.getETrig (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setETrig (uneditedChannelProperties.getETrig (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (eTrigComboBox, "EOS Trig", [this] () { eTrigUiChanged (eTrigComboBox.getSelectedItemIndex ()); }); // Off, > 1, > 2, > 3, > 4, > 5, > 6, > 7, > 8, On
    // Steps 
    setupLabel (stepsLabel, "STEPS", kMediumLabelSize, juce::Justification::centred);
    stepsComboBox.setTooltip ("Steps. Cycles incoming triggers across specified adjacent channels in a stepped round robin fashion. This allows for polyphonic type triggering of samples.");
    stepsComboBox.addItem ("Off", 1);
    for (auto curNumSteps { 0 }; curNumSteps < 7; ++curNumSteps)
        stepsComboBox.addItem ("- " + juce::String (curNumSteps + 2), curNumSteps + 2);
    stepsComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    stepsComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        squidChannelProperties.setSteps (static_cast<uint8_t>(std::clamp (stepsComboBox.getSelectedItemIndex () + scrollAmount, 0, stepsComboBox.getNumItems () - 1)), true);
    };
    stepsComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setSteps (squidChannelProperties.getSteps (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setSteps (defaultChannelProperties.getSteps (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setSteps (uneditedChannelProperties.getSteps (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (stepsComboBox, "Steps", [this] () {stepsUiChanged (stepsComboBox.getSelectedItemIndex ()); }); // 0-7 (Off, - 2, - 3, - 4, - 5, - 6, - 7, - 8)
    // Output
    setupLabel (outputLabel, "OUTPUT", kMediumLabelSize, juce::Justification::centred);
    outputComboBox.setTooltip ("Neighbour Output. Select with the original channel output, or assign it to it's neighbor. Chans 1-4 to the 1+2 output or 3+4 output and channels 5-8 to the 5+6 output or 7+8 output.");
    outputComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    outputComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * (-1 * direction) };
        const auto selectedIndex { static_cast<uint8_t>(std::clamp (outputComboBox.getSelectedItemIndex () + scrollAmount, 0, outputComboBox.getNumItems () - 1)) };
        outputComboBox.setSelectedItemIndex (selectedIndex);
        outputUiChanged (selectedIndex);
    };
    outputComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                editManager->setAltOutput (destChannelProperties.getChannelIndex (), editManager->isAltOutput (squidChannelProperties.getChannelIndex ()));
            },
            [this] ()
            {
                 editManager->setAltOutput (squidChannelProperties.getChannelIndex (), editManager->isAltOutput (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ())));
            },
            [this] ()
            {
                 editManager->setAltOutput (squidChannelProperties.getChannelIndex (), editManager->isAltOutput (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ())));
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (outputComboBox, "Output", [this] () { outputUiChanged (outputComboBox.getSelectedItemIndex ()); });
    // CUE RANDOM
    setupLabel (cueRandomLabel, "CUE RANDOM", kMediumLabelSize, juce::Justification::centred);
    cueRandomButton.setTooltip ("Random Cue Selection. Enabling will cause a random Cue Set to be selected each time the channel is triggered.");
    cueRandomButton.onClick = [this] ()
    {
        editManager->setCueRandom (squidChannelProperties.getChannelIndex (), cueRandomButton.getToggleState ());
    };
    cueRandomButton.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                editManager->setCueRandom (destChannelProperties.getChannelIndex (), editManager->isCueRandomOn (squidChannelProperties.getChannelIndex ()));
            },
            [this] ()
            {
                 editManager->setCueRandom (squidChannelProperties.getChannelIndex (), editManager->isCueRandomOn (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ())));
            },
            [this] ()
            {
                editManager->setCueRandom (squidChannelProperties.getChannelIndex (), editManager->isCueRandomOn (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ())));
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (cueRandomButton);
    // CUE STEP
    setupLabel (cueStepLabel, "CUE STEP", kMediumLabelSize, juce::Justification::centred);
    cueStepButton.setTooltip ("Step Cue Selection. Enabling will cause the next Cue Set to be selected each time the channel is triggered. Once it reaches the end, it will wrap around to the first.");
    cueStepButton.onClick = [this] ()
    {
        editManager->setCueStep (squidChannelProperties.getChannelIndex (), cueStepButton.getToggleState ());
    };
    cueStepButton.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu (squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                editManager->setCueStep (destChannelProperties.getChannelIndex (), editManager->isCueStepOn (squidChannelProperties.getChannelIndex ()));
            },
            [this] ()
            {
                 editManager->setCueStep (squidChannelProperties.getChannelIndex (), editManager->isCueStepOn (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ())));
            },
            [this] ()
            {
                editManager->setCueStep (squidChannelProperties.getChannelIndex (), editManager->isCueStepOn (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ())));
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (cueStepButton);

    // LOOP POINTS VIEW
    addAndMakeVisible (loopPointsView);

    // PLAY BUTTONS
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
    loopPlayButton.setTooltip ("Continuous looping playback back the sample, using the loop and end cue points. No DSP is applied.");
    setupPlayButton (loopPlayButton, "LOOP", false, "ONCE", AudioPlayerProperties::PlayMode::loop);
    oneShotPlayButton.setTooltip ("Play back the sample once, using the start and end cue points. No DSP is applied.");
    setupPlayButton (oneShotPlayButton, "ONCE", false, "LOOP", AudioPlayerProperties::PlayMode::once);

    // CUE SET ADD/DELETE BUTTONS
    addCueSetButton.setTooltip ("Add Cue Set. Will append a new Cue Set to the end.");
    addCueSetButton.setLookAndFeel (&cueEditButtonLnF);
    addCueSetButton.setButtonText ("+");
    addCueSetButton.onClick = [this] () { appendCueSet (); };
    addCueSetButton.setEnabled (false);
    addAndMakeVisible (addCueSetButton);
    deleteCueSetButton.setTooltip ("Delete Cue Set. Will delete the currently selected Cue Set.");
    deleteCueSetButton.setLookAndFeel (&cueEditButtonLnF);
    deleteCueSetButton.setButtonText ("-");
    deleteCueSetButton.onClick = [this] () { deleteCueSet (squidChannelProperties.getCurCueSet ()); };
    deleteCueSetButton.setEnabled (false);
    addAndMakeVisible (deleteCueSetButton);

    // CV ASSIGN EDITOR
    addAndMakeVisible (cvAssignEditor);

    // WAVEFORM DISPLAY
    waveformDisplay.isInterestedInFiles = [this] (const juce::StringArray& files)
    {
        for (auto& file : files)
            if (! editManager->isSupportedAudioFile (file))
                return false;
        return true;
    };
    waveformDisplay.onFilesDropped = [this] (const juce::StringArray& files)
    {
        if (files.size () == 1)
            handleSampleAssignment (files [0]);
        else
            filesDroppedOnCueSetEditor (files);
    };
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
        cueSetButtons [cueSetIndex].onClick = [this, cueSetIndex] () { setCurCue (cueSetIndex); };
        addAndMakeVisible (cueSetButtons [cueSetIndex]);
    }
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
    if (filterEnabled)
    {
        filterFrequencyTextEditor.setText (juce::String (getFilterFrequencyUiValue (squidChannelProperties.getFilterFrequency ())), juce::NotificationType::dontSendNotification);
        filterResonanceTextEditor.setText (juce::String (getUiValue (squidChannelProperties.getFilterResonance ())), juce::NotificationType::dontSendNotification);
    }
    else
    {
        filterFrequencyTextEditor.setText ("--", juce::NotificationType::dontSendNotification);
        filterResonanceTextEditor.setText ("--", juce::NotificationType::dontSendNotification);
    }
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
    waveformDisplay.setCuePoints (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCueSet (cueSetIndex)),
                                  SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCueSet (cueSetIndex)),
                                  SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCueSet (cueSetIndex)));
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

bool ChannelEditorComponent::loadFile (juce::String sampleFileName)
{
    return handleSampleAssignment (sampleFileName);
}

juce::ValueTree ChannelEditorComponent::getChannelPropertiesVT ()
{
    return squidChannelProperties.getValueTree ();
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
    waveformDisplay.setChannelIndex (squidChannelProperties.getChannelIndex ());
    cvAssignEditor.init (rootPropertiesVT, squidChannelPropertiesVT);

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
    filterTypeDataChanged (squidChannelProperties.getFilterType ());
    filterFrequencyDataChanged (squidChannelProperties.getFilterFrequency ());
    filterResonanceDataChanged (squidChannelProperties.getFilterResonance ());
    levelDataChanged (squidChannelProperties.getLevel ());
    loopCueDataChanged (squidChannelProperties.getLoopCue ());
    loopModeDataChanged (squidChannelProperties.getLoopMode ());
    quantDataChanged (squidChannelProperties.getQuant ());
    rateDataChanged (squidChannelProperties.getRate ());
    reverseDataChanged (squidChannelProperties.getReverse ());
    sampleFileNameDataChanged (squidChannelProperties.getSampleFileName ());
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
    squidChannelProperties.onSampleFileNameChange = [this] (juce::String sampleFileName) { sampleFileNameDataChanged (sampleFileName); };
    squidChannelProperties.onEndCueChange = [this] (int endCue) { endCueDataChanged (endCue); };
    squidChannelProperties.onEndCueSetChange = [this] (int cueIndex, int endCue)
    {
        if (cueIndex == curCueSetIndex)
            waveformDisplay.setCueEndPoint (SquidChannelProperties::byteOffsetToSampleOffset (endCue));
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
        setCurCue (squidChannelProperties.getCurCueSet ());
    };
    squidChannelProperties.onLoopCueChange = [this] (int loopCue) { loopCueDataChanged (loopCue); };
    squidChannelProperties.onLoopCueSetChange = [this] (int cueIndex, int loopCue)
    {
        if (cueIndex == curCueSetIndex)
            waveformDisplay.setCueLoopPoint (SquidChannelProperties::byteOffsetToSampleOffset (loopCue));
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
            waveformDisplay.setCueStartPoint (SquidChannelProperties::byteOffsetToSampleOffset (startCue));
    };
    squidChannelProperties.onStepsChange = [this] (int steps) { stepsDataChanged (steps); };
    squidChannelProperties.onXfadeChange = [this] (int xfade) { xfadeDataChanged (xfade); };

    squidChannelProperties.onSampleDataAudioBufferChange = [this] (AudioBufferRefCounted::RefCountedPtr audioBufferPtr)
    {
        updateLoopPointsView ();
        if (squidChannelProperties.getSampleDataAudioBuffer () != nullptr)
        {
            waveformDisplay.setAudioBuffer (squidChannelProperties.getSampleDataAudioBuffer ()->getAudioBuffer ());
            sampleLengthLabel.setText ("(" + juce::String (squidChannelProperties.getSampleDataNumSamples () / squidChannelProperties.getSampleDataSampleRate (), 2) + " seconds/" +
                                       juce::String (squidChannelProperties.getSampleDataNumSamples ()) + " samples)", juce::NotificationType::dontSendNotification);
        }
        else
        {
            waveformDisplay.setAudioBuffer (nullptr);
            sampleLengthLabel.setText ("", juce::NotificationType::dontSendNotification);
        }
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
        waveformDisplay.setAudioBuffer (squidChannelProperties.getSampleDataAudioBuffer ()->getAudioBuffer ());
    }
    else
    {
        loopPointsView.setAudioBuffer (nullptr);
    }
    oneShotPlayButton.setEnabled (squidChannelProperties.getSampleDataAudioBuffer () != nullptr);
    loopPlayButton.setEnabled (squidChannelProperties.getSampleDataAudioBuffer () != nullptr);
    loopPointsView.setLoopPoints (SquidChannelProperties::byteOffsetToSampleOffset (startSample), SquidChannelProperties::byteOffsetToSampleOffset (numBytes));
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

int ChannelEditorComponent::getFilterFrequencyUiValue (int internalValue)
{
    return (internalValue == 0 ? 99 : 98 - ((internalValue - 55) / 40));
}

int ChannelEditorComponent::getFilterFrequencyInternalValue (int uiValue)
{
    const auto invertedValue = 99 - uiValue;
    return (invertedValue == 0 ? 0 : 55 + ((invertedValue - 1) * 40));
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

void ChannelEditorComponent::channelFlagsDataChanged ([[maybe_unused]] uint16_t channelFlags)
{
    // handle cue random flag
    cueRandomButton.setToggleState (editManager->isCueRandomOn (squidChannelProperties.getChannelIndex ()), juce::NotificationType::dontSendNotification);

    // handle cue step flag
    cueStepButton.setToggleState (editManager->isCueStepOn (squidChannelProperties.getChannelIndex ()), juce::NotificationType::dontSendNotification);

    // handle neighbor out flag
    const auto channelIndex { squidChannelProperties.getChannelIndex () };
    const auto useAltOut { editManager->isAltOutput (channelIndex) };
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
    const auto endCueSampleOffset { SquidChannelProperties::byteOffsetToSampleOffset(endCueByteOffset) };
    endCueTextEditor.setText (juce::String (endCueSampleOffset), juce::NotificationType::dontSendNotification);
    waveformDisplay.setCueEndPoint (endCueSampleOffset);
    updateLoopPointsView ();
}

void ChannelEditorComponent::eTrigDataChanged (int eTrig)
{
    eTrigComboBox.setSelectedItemIndex (eTrig, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::sampleFileNameDataChanged (juce::String sampleFileName)
{
    const auto justTheFileName { juce::File (sampleFileName).getFileNameWithoutExtension() };
    sampleFileNameSelectLabel.setText (justTheFileName, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::filterTypeDataChanged (int filterType)
{
    filterTypeComboBox.setSelectedItemIndex (filterType, juce::NotificationType::dontSendNotification);
    setFilterEnableState ();
}

void ChannelEditorComponent::filterFrequencyDataChanged (int filterFrequency)
{
    filterFrequencyTextEditor.setText (juce::String (getFilterFrequencyUiValue (filterFrequency)), juce::NotificationType::dontSendNotification);
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
    const auto loopCueSampleOffset { SquidChannelProperties::byteOffsetToSampleOffset (loopCueByteOffset) };
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
    const auto startCueSampleOffset { SquidChannelProperties::byteOffsetToSampleOffset (startCueByteOffset) };
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
    sampleFileNameSelectLabel.setEnabled (squidChannelProperties.getChannelSource () == squidChannelProperties.getChannelIndex ());
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
    const auto endCueByteOffset { SquidChannelProperties::sampleOffsetToByteOffset (endCueSampleOffset) };
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
    squidChannelProperties.setFilterFrequency (getFilterFrequencyInternalValue (filterFrequency), false);
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
    const auto loopCueByteOffset { SquidChannelProperties::sampleOffsetToByteOffset (loopCueSampleOffset) };
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
    const auto startCueByteOffset { SquidChannelProperties::sampleOffsetToByteOffset (startCueSampleOffset) };
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

bool ChannelEditorComponent::handleSampleAssignment (juce::String sampleFileName)
{
    DebugLog ("ChannelEditorComponent", "handleSampleAssignment - sample to load: " + sampleFileName);
    auto srcFile { juce::File (sampleFileName) };
    const auto channelDirectory { juce::File(appProperties.getRecentlyUsedFile (0)).getChildFile(juce::String(squidChannelProperties.getChannelIndex () + 1)) };
    if (! channelDirectory.exists ())
        channelDirectory.createDirectory ();
    auto destFile { channelDirectory.getChildFile (srcFile.withFileExtension ("_wav").getFileName ()) };
    auto currentSampleFile { juce::File (squidChannelProperties.getSampleFileName ()) };
    // if the currently assigned sample is a "temp" file, we will delete it
    if (currentSampleFile.getFileExtension () == "._wav")
        currentSampleFile.deleteFile ();
    if (srcFile.getParentDirectory () != channelDirectory)
    {
        // TODO handle case where file of same name already exists
        // TODO should copy be moved to a thread?
        // since we are copying the file from elsewhere, we will save it to a file with a "magic" extension
        // this is so we can undo things if the Bank is not saved, and we don't use the normal 'wav' extension
        // in case the app crashes, or something, and the extra file would confuse the module
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

void ChannelEditorComponent::filesDropped (const juce::StringArray& files, int x, int y)
{
    draggingFiles = false;
    repaint ();
    const auto dropBounds { juce::Rectangle<int>{0, 0, getWidth (), cueSetButtons [0].getY ()} };
    if (!dropBounds.contains (x, y))
        return;
    if (! handleSampleAssignment (files[0]))
    {
        // TODO - indicate an error?
    }
}

void ChannelEditorComponent::fileDragEnter (const juce::StringArray& /*files*/, int /*x*/, int /*y*/)
{
    draggingFiles = true;
    repaint ();
}

void ChannelEditorComponent::fileDragExit (const juce::StringArray&)
{
   draggingFiles = false;
   repaint ();
}

void ChannelEditorComponent::resized ()
{
    const auto columnWidth { 160 };
    const auto spaceBetweenColumns { 10 };

    const auto fieldWidth { columnWidth / 2 - 2 };
    auto xInitialOffSet { 15 };

    auto xOffset { xInitialOffSet };
    auto yOffset { kInitialYOffset };

    // FILENAME
    sampleFileNameLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    sampleFileNameSelectLabel.setBounds (sampleFileNameLabel.getRight () + 3, yOffset, fieldWidth * 3, kParameterLineHeight);
    toolsButton.setBounds (getWidth () - 43, sampleFileNameSelectLabel.getY (), 40, 20);
    sampleLengthLabel.setBounds (sampleFileNameSelectLabel.getRight () + 3, yOffset, fieldWidth * 3, kParameterLineHeight);
    yOffset = sampleFileNameSelectLabel.getBottom () + 3;

    // column one
    levelLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    levelTextEditor.setBounds (levelLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = levelTextEditor.getBottom () + 3;
    attackLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    attackTextEditor.setBounds (attackLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = attackTextEditor.getBottom () + 3;
    decayLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    decayTextEditor.setBounds (decayLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = decayTextEditor.getBottom () + 3;
    outputLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    outputComboBox.setBounds (outputLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = outputComboBox.getBottom () + 3;
    startCueLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    startCueTextEditor.setBounds (startCueLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = startCueTextEditor.getBottom () + 3;
    loopCueLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    loopCueTextEditor.setBounds (loopCueLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = loopCueTextEditor.getBottom () + 3;
    endCueLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    endCueTextEditor.setBounds (endCueLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);

    loopPointsView.setBounds (startCueTextEditor.getRight () + spaceBetweenColumns, startCueTextEditor.getY (), columnWidth * 2, endCueTextEditor.getBottom () - startCueTextEditor.getY ());
    oneShotPlayButton.setBounds (loopPointsView.getX () + 2, loopPointsView.getY () + 2, 35, kMediumLabelIntSize);
    loopPlayButton.setBounds (loopPointsView.getX () + 2, loopPointsView.getBottom () - 2 - kMediumLabelIntSize, 35, kMediumLabelIntSize);

    // column two
    xOffset += columnWidth + spaceBetweenColumns;
    yOffset = sampleFileNameSelectLabel.getBottom () + 3;
    filterTypeLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    filterTypeComboBox.setBounds (filterTypeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = filterTypeComboBox.getBottom () + 3;
    filterFrequencyLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    filterFrequencyTextEditor.setBounds (filterFrequencyLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = filterFrequencyTextEditor.getBottom () + 3;
    filterResonanceLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    filterResonanceTextEditor.setBounds (filterResonanceLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = filterResonanceTextEditor.getBottom () + 3;
    reverseLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    reverseButton.setBounds (reverseLabel.getRight () + 3, yOffset, fieldWidth, kMediumLabelIntSize + 2);

    // column three
    xOffset += columnWidth + spaceBetweenColumns;
    yOffset = sampleFileNameSelectLabel.getBottom () + 3;
    // only one of these is visible for a given channel
    // Speed for Channels 1-5
    speedLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    speedTextEditor.setBounds (speedLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    // Quantize for Channels 6-8
    quantLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    quantComboBox.setBounds (quantLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = quantComboBox.getBottom () + 3;
    bitsLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    bitsTextEditor.setBounds (bitsLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = bitsTextEditor.getBottom () + 3;
    rateLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    rateComboBox.setBounds (rateLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = rateComboBox.getBottom () + 3;

    // column four
    xOffset += columnWidth + spaceBetweenColumns;
    yOffset = sampleFileNameSelectLabel.getBottom () + 3;
    loopModeLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    loopModeComboBox.setBounds (loopModeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = loopModeComboBox.getBottom () + 3;
    xfadeLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    xfadeTextEditor.setBounds (xfadeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = xfadeTextEditor.getBottom () + 3;
    chokeLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    chokeComboBox.setBounds (chokeLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = chokeComboBox.getBottom () + 3;
    eTrigLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    eTrigComboBox.setBounds (eTrigLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = eTrigComboBox.getBottom () + 3;
    stepsLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    stepsComboBox.setBounds (stepsLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);
    yOffset = stepsComboBox.getBottom () + 3;
    channelSourceLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    channelSourceComboBox.setBounds (channelSourceLabel.getRight () + 3, yOffset, fieldWidth, kParameterLineHeight);

    // column five
    xOffset += columnWidth + spaceBetweenColumns;
    yOffset = sampleFileNameSelectLabel.getBottom () + 3;
    cueRandomLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    cueRandomButton.setBounds (cueRandomLabel.getRight () + 3, yOffset, fieldWidth, kMediumLabelIntSize + 2);
    yOffset = cueRandomButton.getBottom () + 3;
    cueStepLabel.setBounds (xOffset, yOffset, fieldWidth, kMediumLabelIntSize);
    cueStepButton.setBounds (cueStepLabel.getRight () + 3, yOffset, fieldWidth, kMediumLabelIntSize + 2);

    const auto kWidthOfWaveformEditor { 962 };

    // CV ASSIGNS EDITOR AT BOTTOM
    constexpr auto kCvAssignHeight { 100 };
    cvAssignEditor.setBounds (xInitialOffSet, getHeight() - 5 - kCvAssignHeight, kWidthOfWaveformEditor, kCvAssignHeight);

    // CUE SETS STUFF BETWEEN CHANNEL PARAMETERS AND CV ASSIGNS EDITOR
    // CUE SET BUTTONS
    const auto kHeightOfCueSetButton { 20 };
    const auto kWidthOfCueSetButton { 30 };
    xOffset = xInitialOffSet;
    yOffset = endCueTextEditor.getBottom () + 5 + kHeightOfCueSetButton;
    const auto kHeightOfWaveformDisplay { cvAssignEditor.getY () - yOffset - 5 - kHeightOfCueSetButton };
    const auto kWidthOfCueEditButtons { 15 };
    addCueSetButton.setBounds (xInitialOffSet, yOffset, kWidthOfCueEditButtons, (kHeightOfWaveformDisplay / 2) - 2);
    deleteCueSetButton.setBounds (xInitialOffSet, yOffset + (kHeightOfWaveformDisplay / 2) + 2, kWidthOfCueEditButtons, (kHeightOfWaveformDisplay / 2) - 2);
    // WAVEFORM
    const auto kWaveformXOffset { kWidthOfCueEditButtons + 2};
    waveformDisplay.setBounds (xOffset + kWaveformXOffset, yOffset, kWidthOfWaveformEditor - kWaveformXOffset, kHeightOfWaveformDisplay);
    // CUE SET TABS
    for (auto cueSetIndex { 0 }; cueSetIndex < cueSetButtons.size () / 2; ++cueSetIndex)
    {
        const auto buttonX { xOffset + cueSetIndex * kWidthOfCueSetButton };
        cueSetButtons [cueSetIndex].setBounds (buttonX, waveformDisplay.getY () - kHeightOfCueSetButton, kWidthOfCueSetButton, kHeightOfCueSetButton);
        cueSetButtons [cueSetIndex + 32].setBounds (buttonX, waveformDisplay.getBottom (), kWidthOfCueSetButton, kHeightOfCueSetButton);
    }
}

void ChannelEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void ChannelEditorComponent::paintOverChildren (juce::Graphics& g)
{
    if (draggingFiles)
    {
        const auto dropBounds { juce::Rectangle<int>{0, 0, getWidth (), cueSetButtons [0].getY ()} };
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.fillRect (dropBounds);
        g.setFont (30.0f);
        g.setColour (juce::Colours::black);
        g.drawText ("Assign sample to Channel " + juce::String(squidChannelProperties.getChannelIndex () + 1), dropBounds, juce::Justification::centred, false);
    }
}

void ChannelEditorComponent::filesDroppedOnCueSetEditor (const juce::StringArray& files)
{
    editManager->concatenateAndBuildCueSets (files, squidChannelProperties.getChannelIndex ());
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

void ChannelEditorComponent::outputUiChanged (int selectedIndex)
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
    if (channelIndex < 2) // 1-2
    {
        if (selectedIndex == 0)
            editManager->setAltOutput(channelIndex, false);
        else
            editManager->setAltOutput (channelIndex, true);
    }
    else if (channelIndex < 4) // 3-4
    {
        if (selectedIndex == 1)
            editManager->setAltOutput (channelIndex, false);
        else
            editManager->setAltOutput (channelIndex, true);
    }
    else if (channelIndex < 6) // 5-6
    {
        if (selectedIndex == 0)
            editManager->setAltOutput (channelIndex, false);
        else
            editManager->setAltOutput (channelIndex, true);
    }
    else // 7-8
    {
        if (selectedIndex == 1)
            editManager->setAltOutput (channelIndex, false);
        else
            editManager->setAltOutput (channelIndex, true);
    }
}
