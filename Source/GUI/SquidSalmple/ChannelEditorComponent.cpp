#include "ChannelEditorComponent.h"
#include "../Theme/SquidColourIds.h"
#include "../../SystemServices.h"
#include "../../SquidSalmple/Metadata/SquidSalmpleDefs.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

constexpr auto kMaxSampleLength { 524287 };

const auto kLargeLabelSize { 20.0f };
const auto kMediumLabelSize { 12.0f };
const auto kSmallLabelSize { 12.0f };
const auto kLargeLabelIntSize { static_cast<int> (kLargeLabelSize) };
const auto kMediumLabelIntSize { static_cast<int> (kMediumLabelSize) };
const auto kSmallLabelIntSize { static_cast<int> (kSmallLabelSize) };

const auto kParameterLineHeight { 20 };
const auto kInterControlYOffset { 2 };
const auto kInitialYOffset { 5 };

static const auto kScaleMax { 65535. };
static const auto kScaleStep { kScaleMax / 100 };

const auto kDialogTextEditorName { "samplename" };

ChannelEditorComponent::ChannelEditorComponent ()
{
    setOpaque (true);
    toolsButton.setTooltip ("Channel Tools");
    toolsButton.onClick = [this] ()
    {
        juce::PopupMenu editMenu;
        editMenu.addSectionHeader ("Channel " + juce::String (squidChannelProperties.getChannelIndex () + 1));
        editMenu.addSeparator ();
        {
            // Clone
            juce::PopupMenu cloneMenu;
            {
                auto cloneSettingsMenu { editManager->createChannelInteractionMenu (squidChannelProperties.getChannelIndex (), "To",
                    [this] (SquidChannelProperties& destChannelProperties)
                    {
                        destChannelProperties.copyFrom (squidChannelProperties.getValueTree (), SquidChannelProperties::mainSettings, SquidChannelProperties::CheckIndex::yes);
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
                            auto destFile { juce::File (destChannelProperties.getSampleFileName ()).withFileExtension ("._wav") };
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
            ) };

            editMenu.addSubMenu ("Swap", swapMenu);
        }
        editMenu.addItem ("Rename Sample", true, false, [this, channelIndex = squidChannelProperties.getChannelIndex ()] ()
        {
            const auto currentFileName { juce::File (squidChannelProperties.getSampleFileName ()).getFileNameWithoutExtension () };
            renameAlertWindow = std::make_unique<juce::AlertWindow> ("RENAME SAMPLE", "Enter the new name for '" + currentFileName + "'", juce::MessageBoxIconType::NoIcon);
            renameAlertWindow->addTextEditor (kDialogTextEditorName, currentFileName, {});
            renameAlertWindow->addButton ("RENAME", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
            renameAlertWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
            auto* textEdtitor { renameAlertWindow->getTextEditor (kDialogTextEditorName) };
            auto* createButton { renameAlertWindow->getButton ("RENAME") };
            auto* cancelButton { renameAlertWindow->getButton ("CANCEL") };
            textEdtitor->setExplicitFocusOrder (1);
            createButton->setExplicitFocusOrder (2);
            cancelButton->setExplicitFocusOrder (3);
            renameAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this, channelIndex] (int option)
            {
                renameAlertWindow->exitModalState (option);
                renameAlertWindow->setVisible (false);
                if (option == 1) // ok
                {
                    auto newSampleName { renameAlertWindow->getTextEditorContents (kDialogTextEditorName) };
                    editManager->renameSample (channelIndex, newSampleName);
                }
                renameAlertWindow.reset ();
            }));
        });
        {
            auto clearCueSets = [this] ()
            {
                squidChannelProperties.setCurCueSet (0, false);
                for (auto cueSetCount { squidChannelProperties.getNumCueSets () }; cueSetCount > 1; --cueSetCount)
                    squidChannelProperties.removeCueSet (cueSetCount - 1);
                const auto endOffset { SquidChannelProperties::sampleOffsetToByteOffset (squidChannelProperties.getSampleDataNumSamples ()) };
                squidChannelProperties.setStartCue (0, true);
                squidChannelProperties.setLoopCue (0, true);
                squidChannelProperties.setEndCue (endOffset, true);
                squidChannelProperties.setCueSetPoints (0, 0, 0, SquidChannelProperties::sampleOffsetToByteOffset (squidChannelProperties.getSampleDataNumSamples ()));
            };
            juce::PopupMenu cueSetsMenu;
            cueSetsMenu.addItem ("Clear Cue Sets", true, false, [this, clearCueSets] () { clearCueSets (); });
            {
                juce::PopupMenu chopMenu;
                for (auto numberOfPieces { 2 }; numberOfPieces < 17; ++numberOfPieces)
                {
                    chopMenu.addItem (juce::String (numberOfPieces), true, false, [this, clearCueSets, numberOfPieces] ()
                    {
                        clearCueSets ();
                        for (auto curCueSetIndex { 0 }; curCueSetIndex < numberOfPieces; ++curCueSetIndex)
                        {
                            const auto startOffset { SquidChannelProperties::sampleOffsetToByteOffset (squidChannelProperties.getSampleDataNumSamples () / numberOfPieces * curCueSetIndex) };
                            const auto endOffset { SquidChannelProperties::sampleOffsetToByteOffset (squidChannelProperties.getSampleDataNumSamples () / numberOfPieces * (curCueSetIndex + 1)) };
                            squidChannelProperties.setCueSetPoints (curCueSetIndex, startOffset, startOffset, endOffset);
                        }
                        squidChannelProperties.setCurCueSet (0, true);
                        startCueDataChanged (0);
                        loopCueDataChanged (0);
                        endCueDataChanged (squidChannelProperties.getEndCue ());
                        waveformDisplay.setCuePoints (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCueSet (0)),
                                                      SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCueSet (0)),
                                                      SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCueSet (0)));
                    });
                }
                cueSetsMenu.addSubMenu ("Chop", chopMenu);
            }
            editMenu.addSubMenu ("Cue Sets", cueSetsMenu);
        }
        editMenu.addItem ("Clear", true, false, [this, channelIndex = squidChannelProperties.getChannelIndex ()] ()
        {
            editManager->clearChannel (squidChannelProperties.getChannelIndex ());
        });
        editMenu.addItem ("Default", true, false, [this, channelIndex = squidChannelProperties.getChannelIndex ()] ()
        {
            editManager->setChannelDefaults (squidChannelProperties.getChannelIndex ());
        });
        editMenu.addItem ("Revert", true, false, [this, channelIndex = squidChannelProperties.getChannelIndex ()] ()
        {
            editManager->setChannelUnedited (squidChannelProperties.getChannelIndex ());
        });
        editMenu.showMenuAsync ({});

    };
    addAndMakeVisible (toolsButton);
    setupComponents ();
}

void ChannelEditorComponent::setupComponents ()
{
    auto setupLabel = [this] (juce::Label& label, juce::String text)
    {
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (juce::Justification::centredLeft);
        label.setFont (SquidType::parameterLabel ());
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    auto setupTextEditor = [this] (juce::TextEditor& textEditor, juce::Justification justification, int maxLen, juce::String validInputCharacters)
    {
        textEditor.setJustification (justification);
        textEditor.setFont (SquidType::value ());
        textEditor.setBorder ({ 0, 7, 0, 7 });
        textEditor.setIndents (0, 0);
        HoverHighlight::attach (textEditor);
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        addAndMakeVisible (textEditor);
    };
    auto setupComboBox = [this] (juce::ComboBox& comboBox, std::function<void ()> onChangeCallback)
    {
        jassert (onChangeCallback != nullptr);
        comboBox.onChange = onChangeCallback;
        HoverHighlight::attach (comboBox);
        addAndMakeVisible (comboBox);
    };
    auto setupHeaderLabel = [this] (juce::Label& label, juce::String text)
    {
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (juce::Justification::centredLeft);
        label.setFont (SquidType::sectionHeader ());
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    setupHeaderLabel (levelEnvHeaderLabel, "LEVEL & ENV");
    setupHeaderLabel (filterHeaderLabel, "FILTER");
    setupHeaderLabel (qualityHeaderLabel, "QUALITY");
    setupHeaderLabel (loopHeaderLabel, "LOOP");
    setupHeaderLabel (triggerHeaderLabel, "TRIGGER");
    setupHeaderLabel (cueTriggerHeaderLabel, "CUE TRIGGER");
    setupHeaderLabel (cuePointsHeaderLabel, "CUE POINTS");
    setupHeaderLabel (loopTunerHeaderLabel, "LOOP TUNER");

    addAndMakeVisible (startCueSwatch);
    addAndMakeVisible (loopCueSwatch);
    addAndMakeVisible (endCueSwatch);

    // FILENAME
    setupHeaderLabel (sampleFileNameLabel, "SAMPLE");
    sampleFileNameSelectLabel.setTooltip ("Sample File Name. Click to open file browser, or drag a file onto the editor. If the file name is dimmed out this channel is using a sample from the Channel specified in the SOURCE parameter.");
    applyExplicitColours ();
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
    addAndMakeVisible (sampleFileNameSelectLabel);

    setupLabel (channelSourceLabel, "SOURCE");
    {
        for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
        {
            // TODO - the channel index is not known until ::init is called
            //const auto channelString { juce::String ("C") + juce::String (curChannelIndex + 1) + (squidChannelProperties.getChannelIndex () == curChannelIndex ? "(self)" : "") };
            const auto channelString { juce::String ("C") + juce::String (curChannelIndex + 1) };
            channelSourceComboBox.addItem (channelString, curChannelIndex + 1);
        }
    }
    channelSourceComboBox.setTooltip ("Channel Reference. Select the channel for which this channel will get it's sample from. Can be changed on the module by holding the Chan button and turning the program knob.");
    channelSourceComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        squidChannelProperties.setChannelSource (static_cast<uint8_t> (std::clamp (channelSourceComboBox.getSelectedItemIndex () + scrollAmount, 0, channelSourceComboBox.getNumItems () - 1)), true);
    };
    channelSourceComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupComboBox (channelSourceComboBox, [this] () { channelSourceUiChanged (static_cast<uint8_t> (channelSourceComboBox.getSelectedItemIndex ())); });

    // BITS
    setupLabel (bitsLabel, "BITS");
    bitsTextEditor.setTooltip ("Bits. Adjust the bit depth of playback. Can be from 1 to 16. Can be changed on the module in the Quality settings.");
    bitsTextEditor.getMinValueCallback = [this] () { return 1; };
    bitsTextEditor.getMaxValueCallback = [this] () { return 16; };
    bitsTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    bitsTextEditor.updateDataCallback = [this] (int value) { bitsUiChanged (value == 16 ? 0 : value); };
    bitsTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { squidChannelProperties.getBits () + static_cast<int> (valueDelta) };
        bitsTextEditor.setValue (newValue);
    };
    bitsTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupTextEditor (bitsTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 1-16
    // RATE
    setupLabel (rateLabel, "RATE");
    rateComboBox.getProperties ().set (SquidLnFProperties::valueUnit, "kHz");
    rateComboBox.setTooltip ("Rate. Adjusts the playback rate of the sample in khz. Values are 4, 6, 7, 9, 11, 14, 22, 44. Can be changed on the module in the Quality settings.");
    rateComboBox.addItem ("4", 8);
    rateComboBox.addItem ("6", 7);
    rateComboBox.addItem ("7", 6);
    rateComboBox.addItem ("9", 5);
    rateComboBox.addItem ("11", 4);
    rateComboBox.addItem ("14", 3);
    rateComboBox.addItem ("22", 2);
    rateComboBox.addItem ("44", 1);
    rateComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        squidChannelProperties.setRate (rateComboBox.getItemId (std::clamp (rateComboBox.getSelectedItemIndex () + scrollAmount, 0, rateComboBox.getNumItems () - 1)) - 1, true);
    };
    rateComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupComboBox (rateComboBox, [this] () { rateUiChanged (rateComboBox.getSelectedId () - 1); }); // 4,6,7,9,11,14,22,44
    // SPEED
    setupLabel (speedLabel, "SPEED");
    speedTextEditor.setTooltip ("Speed. Linear playback speed control. From 1 to 100, where 50 is normal speed. Available for Channels 1-5. Can be changed on the module in the Quality settings.");
    speedTextEditor.getMinValueCallback = [this] () { return 1; };
    speedTextEditor.getMaxValueCallback = [this] () { return 99; };
    speedTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    speedTextEditor.updateDataCallback = [this] (int value) { speedUiChanged (value); };
    speedTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { getUiValue (squidChannelProperties.getSpeed ()) + static_cast<int> (valueDelta) };
        speedTextEditor.setValue (newValue);
    };
    speedTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupTextEditor (speedTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 1 - 99 (50 is normal, below that is negative speed? above is positive?)
    // QUANTIZE
    setupLabel (quantLabel, "QUANT");
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
    quantComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        squidChannelProperties.setQuant (std::clamp (quantComboBox.getSelectedItemIndex () + scrollAmount, 0, quantComboBox.getNumItems () - 1), true);
    };
    quantComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupComboBox (quantComboBox, [this] () { quantUiChanged (quantComboBox.getSelectedId () - 1); }); // 0-14 (Off, 12, OT, MA, mi, Hm, PM, Pm, Ly, Ph, Jp, P5, C1, C4, C5)

    // PITCH SHIFT
    setupLabel (pitchShiftLabel, "PITCH");
    pitchShiftTextEditor.setTooltip ("Pitch Shift. Adjusts how much the audio is shifted in pitch.");
    pitchShiftTextEditor.getMinValueCallback = [this] () { return 0.f; };
    pitchShiftTextEditor.getMaxValueCallback = [this] () { return 4.0f; };
    pitchShiftTextEditor.getIncrementCallback = [this] () { return 0.01; };
    pitchShiftTextEditor.toStringCallback = [this] (double value) { return juce::String (static_cast<float> (value), 2); };
    pitchShiftTextEditor.updateDataCallback = [this] (double value) { pitchShiftUiChanged (static_cast<float> (value)); };
    pitchShiftTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { (squidChannelProperties.getPitchShift () / 1000.) + valueDelta };
        pitchShiftTextEditor.setValue (newValue);
    };
    pitchShiftTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
            [this] (SquidChannelProperties& destChannelProperties)
            {
                destChannelProperties.setPitchShift (squidChannelProperties.getPitchShift (), false);
            },
            [this] ()
            {
                SquidChannelProperties defaultChannelProperties (editManager->getDefaultChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                 SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setPitchShift (defaultChannelProperties.getPitchShift (), true);
            },
            [this] ()
            {
                SquidChannelProperties uneditedChannelProperties (editManager->getUneditedChannelProperties (squidChannelProperties.getChannelIndex ()),
                                                                  SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                squidChannelProperties.setPitchShift (uneditedChannelProperties.getPitchShift (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (pitchShiftTextEditor, juce::Justification::centredRight, 0, "0123456789.");

    // FILTER TYPE
    setupLabel (filterTypeLabel, "TYPE");
    filterTypeComboBox.setTooltip ("Filer Type. Enables the resonant multimode filter and the corresponding Frequency and Resonance parameters. Filer Types: Off, Low Pass, Band Pass, Notch, and High Pass.");
    {
        auto filterId { 1 };
        filterTypeComboBox.addItem ("Off", filterId++);
        filterTypeComboBox.addItem ("Low Pass", filterId++);
        filterTypeComboBox.addItem ("Band Pass", filterId++);
        filterTypeComboBox.addItem ("Notch", filterId++);
        filterTypeComboBox.addItem ("High Pass", filterId++);
    }
    filterTypeComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        squidChannelProperties.setFilterType (std::clamp (filterTypeComboBox.getSelectedItemIndex () + scrollAmount, 0, filterTypeComboBox.getNumItems () - 1), true);
    };
    filterTypeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupComboBox (filterTypeComboBox, [this] () { filterTypeUiChanged (filterTypeComboBox.getSelectedId () - 1); }); // Off, LP, BP, NT, HP (0-4)
    // FILTER FREQUENCY
    setupLabel (filterFrequencyLabel, "FREQ");
    filterFrequencyTextEditor.setTooltip ("Filter Frequency. Adjusts the cut off frequency of the filter, only appears when a filter type is selected.");
    filterFrequencyTextEditor.getMinValueCallback = [this] () { return 0; };
    filterFrequencyTextEditor.getMaxValueCallback = [this] () { return 99; };
    filterFrequencyTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    filterFrequencyTextEditor.updateDataCallback = [this] (int value) { filterFrequencyUiChanged (value); };
    filterFrequencyTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { filterFrequencyTextEditor.getText ().getIntValue () + static_cast<int> (valueDelta) };
        filterFrequencyTextEditor.setValue (newValue);
    };
    filterFrequencyTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
                squidChannelProperties.setFilterFrequency (uneditedChannelProperties.getFilterFrequency (), true);
            }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (filterFrequencyTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 1-99?

    // FILTER RESONANCE
    setupLabel (filterResonanceLabel, "RESO");
    filterResonanceTextEditor.setTooltip ("Filter Resonance. Adjust the resonant peak of the filter, only appears when a filter type is selected.");
    filterResonanceTextEditor.getMinValueCallback = [this] () { return 0; };
    filterResonanceTextEditor.getMaxValueCallback = [this] () { return 99; };
    filterResonanceTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    filterResonanceTextEditor.updateDataCallback = [this] (int value) { filterResonanceUiChanged (value); };
    filterResonanceTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { getUiValue (squidChannelProperties.getFilterResonance ()) + static_cast<int> (valueDelta) };
        filterResonanceTextEditor.setValue (newValue);
    };
    filterResonanceTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupTextEditor (filterResonanceTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 1-99?
    // LEVEL
    setupLabel (levelLabel, "LEVEL");
    levelTextEditor.setTooltip ("Level. Adjust the playback volume of a sample. At 50 is unity gain. below will attenuate, above increase. Use this setting to avoid digital clipping when mixed. Value defaults to 30 as to conservatively avoid digital clipping.");
    levelTextEditor.getMinValueCallback = [this] () { return 1; };
    levelTextEditor.getMaxValueCallback = [this] () { return 99; };
    levelTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    levelTextEditor.updateDataCallback = [this] (int value) { levelUiChanged (value); };
    levelTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { getUiValue (squidChannelProperties.getLevel ()) + static_cast<int> (valueDelta) };
        levelTextEditor.setValue (newValue);
    };
    levelTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupTextEditor (levelTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 1-99
    // ATTACK
    setupLabel (attackLabel, "ATTACK");
    attackTextEditor.setTooltip ("Add a simple attack envelope to control volume at the beginning of sample playback. Behaves similar to Decay if the sample is set to loop.");
    attackTextEditor.getMinValueCallback = [this] () { return 0; };
    attackTextEditor.getMaxValueCallback = [this] () { return 99; };
    attackTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    attackTextEditor.updateDataCallback = [this] (int value) { attackUiChanged (value); };
    attackTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { getUiValue (squidChannelProperties.getAttack ()) + static_cast<int> (valueDelta) };
        attackTextEditor.setValue (newValue);
    };
    attackTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupTextEditor (attackTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 0-99
    // DECAY
    setupLabel (decayLabel, "DECAY");
    decayTextEditor.setTooltip ("Add a simple decay envelope to fade out the volume of the sample. Decay time shortens as value rises. If a sample is set to loop the envelope will effect the loop as a whole (rather than single sample) with max decay time being 10 seconds.");
    decayTextEditor.getMinValueCallback = [this] () { return 0; };
    decayTextEditor.getMaxValueCallback = [this] () { return 99; };
    decayTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    decayTextEditor.updateDataCallback = [this] (int value) { decayUiChanged (value); };
    decayTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { getUiValue (squidChannelProperties.getDecay ()) + static_cast<int> (valueDelta) };
        decayTextEditor.setValue (newValue);
    };
    decayTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupTextEditor (decayTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 0-99
    // LOOP MODE
    setupLabel (loopModeLabel, "MODE");
    {
        auto loopId { 1 };
        loopModeComboBox.addItem ("None", loopId++);
        loopModeComboBox.addItem ("Normal", loopId++);
        loopModeComboBox.addItem ("ZigZag", loopId++);
        loopModeComboBox.addItem ("Normal Gate", loopId++);
        loopModeComboBox.addItem ("ZigZag Gate", loopId++);
    }
    loopModeComboBox.setTooltip ("Loop Mode. Configures how looping will operate. Normal for forward playing. ZigZag plays alternatively forwards then backwards between loop and end points. Gate options indicate sample will only play & loop whilst the associated channels trigger input is held high (like a sustain). If Decay is set however, playback will move to this stage when the trigger goes low.");
    loopModeComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        squidChannelProperties.setLoopMode (static_cast<uint8_t> (std::clamp (loopModeComboBox.getSelectedItemIndex () + scrollAmount, 0, loopModeComboBox.getNumItems () - 1)), true);
    };
    loopModeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupComboBox (loopModeComboBox, [this] () { loopModeUiChanged (loopModeComboBox.getSelectedItemIndex ()); }); // none, normal, zigZag, gate, zigZagGate (0-4)
    // XFADE
    setupLabel (xfadeLabel, "XFADE");
    xfadeTextEditor.setTooltip ("Loop Crossfade. Adds a simple cross fade between the sample end and loop points as to smooth out loops.");
    xfadeTextEditor.getMinValueCallback = [this] () { return 0; };
    xfadeTextEditor.getMaxValueCallback = [this] () { return 99; };
    xfadeTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    xfadeTextEditor.updateDataCallback = [this] (int value) { xfadeUiChanged (value); };
    xfadeTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { squidChannelProperties.getXfade () + static_cast<int> (valueDelta) };
        xfadeTextEditor.setValue (newValue);
    };
    xfadeTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupTextEditor (xfadeTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 0 -99
    // REVERSE
    setupLabel (reverseLabel, "REVERSE");
    reverseButton.setTooltip ("Reverse. Reverses the sample");
    reverseButton.onClick = [this] () { reverseUiChanged (reverseButton.getToggleState ()); };
    reverseButton.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupLabel (startCueLabel, "START");
    startCueLabel.setFont (SquidType::cuePointLabel ());
    startCueTextEditor.setTooltip ("Cue Start. Sets the starting sample for playback.");
    startCueTextEditor.getMinValueCallback = [this] () { return 0; };
    startCueTextEditor.getMaxValueCallback = [this] () { return SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()); };
    startCueTextEditor.toStringCallback = [this] (juce::int32 value) { return juce::String (value); };
    startCueTextEditor.updateDataCallback = [this] (juce::int32 value) { startCueUiChanged (value); };
    startCueTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto valueOffset { static_cast<int> (valueDelta) };
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
        juce::PopupMenu adjustMenu;
        {
            juce::PopupMenu zeroCrossingMenuOptions;
            zeroCrossingMenuOptions.addItem ("Left  <<", true, false, [this] ()
            {
                auto newStartCue { editManager->findPreviousZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()),
                                                                          0,
                                                                          *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newStartCue != -1)
                    startCueTextEditor.setValue (newStartCue);
            });
            zeroCrossingMenuOptions.addItem ("Right >>", true, false, [this] ()
            {
                auto newStartCue { editManager->findNextZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()),
                                                                      SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()),
                                                                      *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newStartCue != -1)
                {
                    if (newStartCue > static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
                        loopCueTextEditor.setValue (newStartCue);
                    startCueTextEditor.setValue (newStartCue);
                }
            });
            adjustMenu.addSubMenu ("Zero Crossing", zeroCrossingMenuOptions);
        }
        auto editMenu { editManager->createChannelEditMenu (adjustMenu, squidChannelProperties.getChannelIndex (),
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
    setupTextEditor (startCueTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 0 - sample length?
    // LOOP
    setupLabel (loopCueLabel, "LOOP");
    loopCueLabel.setFont (SquidType::cuePointLabel ());
    loopCueTextEditor.setTooltip ("Cue Loop. Sets the loop starting sample for playback.");
    loopCueTextEditor.getMinValueCallback = [this] () { return SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()); };
    loopCueTextEditor.getMaxValueCallback = [this] () { return SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()); };
    loopCueTextEditor.toStringCallback = [this] (juce::int32 value) { return juce::String (value); };
    loopCueTextEditor.updateDataCallback = [this] (juce::int32 value) { loopCueUiChanged (value); };
    loopCueTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto valueOffset { static_cast<int> (valueDelta) };
        auto newValue { 0 };
        if (valueOffset < 0 && std::abs (valueOffset) > static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
            newValue = 0;
        else
            newValue = SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ()) + valueOffset;
        loopCueTextEditor.setValue (newValue);
    };
    loopCueTextEditor.onPopupMenuCallback = [this] ()
    {
        juce::PopupMenu adjustMenu;
        {
            juce::PopupMenu zeroCrossingMenuOptions;
            zeroCrossingMenuOptions.addItem ("Left  <<", true, false, [this] ()
            {
                auto newLoopCue { editManager->findPreviousZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ()),
                                                                         SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()),
                                                                         *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newLoopCue != -1)
                    loopCueTextEditor.setValue (newLoopCue);
            });
            zeroCrossingMenuOptions.addItem ("Right >>", true, false, [this] ()
            {
                auto newLoopCue { editManager->findNextZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ()),
                                                                     SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()),
                                                                     *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newLoopCue != -1)
                    loopCueTextEditor.setValue (newLoopCue);
            });
            adjustMenu.addSubMenu ("Zero Crossing", zeroCrossingMenuOptions);
        }
        auto editMenu { editManager->createChannelEditMenu (adjustMenu, squidChannelProperties.getChannelIndex (),
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
            squidChannelProperties.setLoopCue (uneditedChannelProperties.getLoopCue (), true);
        }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (loopCueTextEditor, juce::Justification::centredRight, 0, "0123456789"); // 0 - sample length?, or sampleStart - sampleEnd
    // END
    setupLabel (endCueLabel, "END");
    endCueLabel.setFont (SquidType::cuePointLabel ());
    endCueTextEditor.setTooltip ("Cue End. Sets the end sample for playback and looping.");
    endCueTextEditor.getMinValueCallback = [this] () { return SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()); };
    endCueTextEditor.getMaxValueCallback = [this] () { return squidChannelProperties.getSampleDataNumSamples (); };
    endCueTextEditor.toStringCallback = [this] (juce::int32 value) { return juce::String (value); };
    endCueTextEditor.updateDataCallback = [this] (juce::int32 value) { endCueUiChanged (value); };
    endCueTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto valueOffset { static_cast<int> (valueDelta) };
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
        juce::PopupMenu adjustMenu;
        {
            juce::PopupMenu zeroCrossingMenuOptions;
            zeroCrossingMenuOptions.addItem ("Left  <<", true, false, [this] ()
            {
                auto newEndCue { editManager->findPreviousZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()),
                                                                        SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()),
                                                                        *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newEndCue != -1)
                {
                    if (newEndCue < static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
                        loopCueTextEditor.setValue (newEndCue);
                    endCueTextEditor.setValue (newEndCue);
                }
                                             });
            zeroCrossingMenuOptions.addItem ("Right >>", true, false, [this] ()
            {
                auto newEndCue { editManager->findNextZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()),
                                                                    squidChannelProperties.getSampleDataNumSamples (),
                                                                    *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newEndCue != -1)
                    endCueTextEditor.setValue (newEndCue);
            });
            adjustMenu.addSubMenu ("Zero Crossing", zeroCrossingMenuOptions);
        }
        auto editMenu { editManager->createChannelEditMenu (adjustMenu, squidChannelProperties.getChannelIndex (),
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
    setupTextEditor (endCueTextEditor, juce::Justification::centredRight, 0, "0123456789"); // sampleStart - sample length
    // CHOKE
    setupLabel (chokeLabel, "CHOKE");
    chokeComboBox.setTooltip ("Choke. Select a channel that will stop playing when this channel plays.");
    chokeComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        const auto newItemIndex { static_cast<uint8_t> (std::clamp (chokeComboBox.getSelectedItemIndex () + scrollAmount, 0, chokeComboBox.getNumItems () - 1)) };
        squidChannelProperties.setChoke (chokeComboBox.getItemId (newItemIndex) - 1, true);
    };
    chokeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupComboBox (chokeComboBox, [this] () { chokeUiChanged (chokeComboBox.getSelectedId () - 1); }); // Off, C1, C2, C3, C4, C5, C6, C7, C8
    // ETrig
    setupLabel (eTrigLabel, "EOS TRIG");
    eTrigComboBox.setTooltip ("EOS Trigger. Selecting a channel will start playback of that channel when this one ends. Selecting On will cause a trigger to happen on the Trigger Output.");
    eTrigComboBox.addItem ("Off", 1);
    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
        eTrigComboBox.addItem ("> " + juce::String (curChannelIndex + 1), curChannelIndex + 2);
    eTrigComboBox.addItem ("On", 10);
    eTrigComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        squidChannelProperties.setETrig (static_cast<uint8_t> (std::clamp (eTrigComboBox.getSelectedItemIndex () + scrollAmount, 0, eTrigComboBox.getNumItems () - 1)), true);
    };
    eTrigComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupComboBox (eTrigComboBox, [this] () { eTrigUiChanged (eTrigComboBox.getSelectedItemIndex ()); }); // Off, > 1, > 2, > 3, > 4, > 5, > 6, > 7, > 8, On
    // Steps
    setupLabel (stepsLabel, "STEPS");
    stepsComboBox.setTooltip ("Steps. Cycles incoming triggers across specified adjacent channels in a stepped round robin fashion. This allows for polyphonic type triggering of samples.");
    stepsComboBox.addItem ("Off", 1);
    for (auto curNumSteps { 0 }; curNumSteps < 7; ++curNumSteps)
        stepsComboBox.addItem ("- " + juce::String (curNumSteps + 2), curNumSteps + 2);
    stepsComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        squidChannelProperties.setSteps (static_cast<uint8_t> (std::clamp (stepsComboBox.getSelectedItemIndex () + scrollAmount, 0, stepsComboBox.getNumItems () - 1)), true);
    };
    stepsComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupComboBox (stepsComboBox, [this] () { stepsUiChanged (stepsComboBox.getSelectedItemIndex ()); }); // 0-7 (Off, - 2, - 3, - 4, - 5, - 6, - 7, - 8)
    // Output
    setupLabel (outputLabel, "OUTPUT");
    outputComboBox.setTooltip ("Neighbour Output. Select with the original channel output, or assign it to it's neighbor. Chans 1-4 to the 1+2 output or 3+4 output and channels 5-8 to the 5+6 output or 7+8 output.");
    outputComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (-valueDelta) };
        const auto selectedIndex { static_cast<uint8_t> (std::clamp (outputComboBox.getSelectedItemIndex () + scrollAmount, 0, outputComboBox.getNumItems () - 1)) };
        outputComboBox.setSelectedItemIndex (selectedIndex);
        outputUiChanged (selectedIndex);
    };
    outputComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupComboBox (outputComboBox, [this] () { outputUiChanged (outputComboBox.getSelectedItemIndex ()); });
    // CUE RANDOM
    setupLabel (cueRandomLabel, "RANDOM");
    cueRandomButton.setTooltip ("Random Cue Selection. Enabling will cause a random Cue Set to be selected each time the channel is triggered.");
    cueRandomButton.onClick = [this] ()
    {
        editManager->setCueRandom (squidChannelProperties.getChannelIndex (), cueRandomButton.getToggleState ());
    };
    cueRandomButton.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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
    setupLabel (cueStepLabel, "STEP");
    cueStepButton.setTooltip ("Step Cue Selection. Enabling will cause the next Cue Set to be selected each time the channel is triggered. Once it reaches the end, it will wrap around to the first.");
    cueStepButton.onClick = [this] ()
    {
        editManager->setCueStep (squidChannelProperties.getChannelIndex (), cueStepButton.getToggleState ());
    };
    cueStepButton.onPopupMenuCallback = [this] ()
    {
        auto editMenu { editManager->createChannelEditMenu ({}, squidChannelProperties.getChannelIndex (),
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

    for (auto* slideSwitch : { &reverseButton, &cueRandomButton, &cueStepButton })
        slideSwitch->setThumbShape (RoundedSlideSwitch::ThumbShape::circle);

    // LOOP POINTS VIEW
    addAndMakeVisible (loopPointsView);

    // PLAY BUTTONS
    auto setupPlayButton = [this] (TransportButton& playButton, AudioPlayerProperties::PlayMode playMode)
    {
        playButton.setEnabled (false);
        playButton.onClick = [this, &playButton, playMode] ()
        {
            if (playButton.isPlaying ())
            {
                audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
                playButton.setPlaying (false);
            }
            else
            {
                audioPlayerProperties.setSampleSource (squidChannelProperties.getChannelIndex (), false);
                audioPlayerProperties.setPlayMode (playMode, false);
                audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::play, false);
                // only one of the pair can be playing
                oneShotPlayButton.setPlaying (&playButton == &oneShotPlayButton);
                loopPlayButton.setPlaying (&playButton == &loopPlayButton);
            }
        };
        addAndMakeVisible (playButton);
    };
    loopPlayButton.setTooltip ("Continuous looping playback back the sample, using the loop and end cue points. No DSP is applied.");
    setupPlayButton (loopPlayButton, AudioPlayerProperties::PlayMode::loop);
    oneShotPlayButton.setTooltip ("Play back the sample once, using the start and end cue points. No DSP is applied.");
    setupPlayButton (oneShotPlayButton, AudioPlayerProperties::PlayMode::once);

    // WAVEFORM TOOLS
    waveformToolsButton.setTooltip ("WAVEFORM TOOLS");
    waveformToolsButton.onClick = [this] () { showWaveformToolsMenu (); };
    addAndMakeVisible (waveformToolsButton);

    // CUE SET ADD/DELETE BUTTONS
    addCueSetButton.setTooltip ("Add Cue Set. Will append a new Cue Set to the end.");
    addCueSetButton.onClick = [this] () { appendCueSet (); };
    addCueSetButton.setEnabled (false);
    addAndMakeVisible (addCueSetButton);
    deleteCueSetButton.setTooltip ("Delete Cue Set. Will delete the currently selected Cue Set.");
    deleteCueSetButton.onClick = [this] () { deleteCueSet (squidChannelProperties.getCurCueSet ()); };
    deleteCueSetButton.setEnabled (false);
    addAndMakeVisible (deleteCueSetButton);

    // CV ASSIGN EDITOR
    addAndMakeVisible (cvAssignEditor);

    // WAVEFORM DISPLAY
    waveformDisplay.isInterestedInFiles = [this] (const juce::StringArray& /*files*/)
    {
        return true;
    };
    waveformDisplay.onFilesDropped = [this] (const juce::StringArray& files, WaveformDisplay::DropType dropType)
    {
        switch (dropType)
        {
            case WaveformDisplay::DropType::replace:
            {
                if (files.size () == 1)
                    handleSampleAssignment (files [0]);
                else
                    filesDroppedOnCueSetEditor (files, "new._wav", {});
            }
            break;
            case WaveformDisplay::DropType::append:
            {
                juce::StringArray concatenateList;
                auto cueSetListVT { squidChannelProperties.getValueTree ().getChildWithName (SquidChannelProperties::CueSetListTypeId).createCopy () };

                if (auto currentSampleFile { juce::File (squidChannelProperties.getSampleFileName ()) }; currentSampleFile.getFileExtension () == "._wav")
                {
                    auto tempFile { currentSampleFile.getParentDirectory ().getChildFile (currentSampleFile.getFileNameWithoutExtension () + "_").withFileExtension ("_wav") };
                    currentSampleFile.moveFileTo (tempFile);
                    concatenateList.add (tempFile.getFullPathName ());
                    concatenateList.addArray (files);
                    filesDroppedOnCueSetEditor (concatenateList, currentSampleFile.getFileName (), cueSetListVT);
                    tempFile.deleteFile ();
                }
                else
                {
                    concatenateList.add (squidChannelProperties.getSampleFileName ());
                    concatenateList.addArray (files);
                    filesDroppedOnCueSetEditor (concatenateList, juce::File (squidChannelProperties.getSampleFileName ()).withFileExtension ("._wav").getFileName (), cueSetListVT);
                }
            }
            break;
            default:
                jassertfalse;
            break;
        }
    };
    waveformDisplay.onStartPointChange = [this] (juce::int64 startPoint)
    {
        const auto startCueByteOffset { static_cast<int> (startPoint * 2) };
        squidChannelProperties.setCueSetStartPoint (curCueSetIndex, startCueByteOffset);
        squidChannelProperties.setStartCue (startCueByteOffset, true);
    };
    waveformDisplay.onLoopPointChange = [this] (juce::int64 loopPoint)
    {
        const auto loopCueByteOffset { static_cast<int> (loopPoint * 2) };
        squidChannelProperties.setCueSetLoopPoint (curCueSetIndex, loopCueByteOffset);
        squidChannelProperties.setLoopCue (loopCueByteOffset, true);
    };
    waveformDisplay.onEndPointChange = [this] (juce::int64 endPoint)
    {
        const auto endCueByteOffset { static_cast<int> (endPoint * 2) };
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

void ChannelEditorComponent::showWaveformToolsMenu ()
{
    juce::PopupMenu menu;
    menu.addSectionHeader ("WAVEFORM");
    menu.addSeparator ();
    menu.addItem ("Fit To View", [this] () { waveformDisplay.fitToView (); });
    menu.addItem ("Reset Vertical Zoom", [this] () { waveformDisplay.resetVerticalZoom (); });
    menu.showMenuAsync (juce::PopupMenu::Options ().withTargetComponent (&waveformToolsButton));
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
    applyExplicitColours ();
    cvAssignEditor.setEnableState (CvParameterIndex::FiltFreq, filterEnabled);
    cvAssignEditor.setEnableState (CvParameterIndex::FiltRes, filterEnabled);
    cvAssignEditor.repaint ();
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
    jassert (editManager != nullptr);

    sampleFileNameSelectLabel.setFileFilter (editManager->getFileTypesList ());
    // FileSelectLabel defaults to a generic prompt, since it is shared across applications
    sampleFileNameSelectLabel.setDialogTitle ("Please select the Squid Salmple file you want to load...");

    loopPointsView.init (squidChannelPropertiesVT, rootPropertiesVT);

    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::client, AudioPlayerProperties::EnableCallbacks::yes);
    audioPlayerProperties.onPlayStateChange = [this] (AudioPlayerProperties::PlayState playState)
    {
        if (playState == AudioPlayerProperties::PlayState::stop)
        {
            juce::MessageManager::callAsync ([this] ()
            {
                oneShotPlayButton.setPlaying (false);
                loopPlayButton.setPlaying (false);
            });
        }
        else if (playState == AudioPlayerProperties::PlayState::play)
        {
            if (audioPlayerProperties.getPlayMode () == AudioPlayerProperties::PlayMode::once)
            {
                juce::MessageManager::callAsync ([this] ()
                {
                    oneShotPlayButton.setPlaying (true);
                    loopPlayButton.setPlaying (false);
                });
            }
            else
            {
                juce::MessageManager::callAsync ([this] ()
                {
                    oneShotPlayButton.setPlaying (false);
                    loopPlayButton.setPlaying (true);
                });
            }
        }
        else
        {
            jassertfalse;
        }
    };

    squidChannelProperties.wrap (squidChannelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);
    waveformDisplay.init (rootPropertiesVT);
    waveformDisplay.setChannelIndex (squidChannelProperties.getChannelIndex ());
    cvAssignEditor.init (rootPropertiesVT, squidChannelPropertiesVT);
    cvAssignEditor.setEnableState (CvParameterIndex::Speed, squidChannelProperties.getChannelIndex () < 5);
    cvAssignEditor.setEnableState (CvParameterIndex::PitchShift, squidChannelProperties.getChannelIndex () < 5);

    chokeComboBox.addItem ("Off", squidChannelProperties.getChannelIndex () + 1);
    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
    {
        if (curChannelIndex != squidChannelProperties.getChannelIndex ())
        {
            const auto channelString { juce::String ("C") + juce::String (curChannelIndex + 1) };
            chokeComboBox.addItem (channelString, curChannelIndex + 1);
        }
    }

    // TODO - we need to call this when the sample changes
    updateLoopPointsView ();
    updateWaveformDisplay ();

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
    pitchShiftDataChanged (squidChannelProperties.getPitchShift ());
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
        pitchShiftLabel.setVisible (false);
        pitchShiftTextEditor.setVisible (false);
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
    squidChannelProperties.onPitchShiftChange = [this] (int pitchShift) { pitchShiftDataChanged (pitchShift); };
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

    squidChannelProperties.onSampleDataAudioBufferChange = [this] ([[maybe_unused]] AudioBufferRefCounted::RefCountedPtr audioBufferPtr)
    {
        updateLoopPointsView ();
        if (squidChannelProperties.getSampleDataAudioBuffer () != nullptr)
        {
            waveformDisplay.setAudioBuffer (squidChannelProperties.getSampleDataAudioBuffer ()->getAudioBuffer ());
            const auto numSamples { squidChannelProperties.getSampleDataNumSamples () };
            sampleSecondsText = juce::String (numSamples / squidChannelProperties.getSampleDataSampleRate (), 2);
            // grouped, as the mockup writes it: 13,125
            auto digits { juce::String (numSamples) };
            for (auto insertAt { digits.length () - 3 }; insertAt > 0; insertAt -= 3)
                digits = digits.substring (0, insertAt) + "," + digits.substring (insertAt);
            sampleCountText = digits;
            repaint (sampleMetaBounds);
        }
        else
        {
            waveformDisplay.setAudioBuffer (nullptr);
            sampleSecondsText = {};
            sampleCountText = {};
            repaint (sampleMetaBounds);
        }
        oneShotPlayButton.setEnabled (squidChannelProperties.getSampleDataAudioBuffer () != nullptr);
        loopPlayButton.setEnabled (squidChannelProperties.getSampleDataAudioBuffer () != nullptr);
    };
}

void ChannelEditorComponent::updateWaveformDisplay ()
{
    // Only ever call this when the sample itself changes. The cue markers are
    // pushed separately by the individual cue change handlers, and handing the
    // waveform display a buffer makes it re-fit the view to the whole sample.
    if (auto sampleData { squidChannelProperties.getSampleDataAudioBuffer () }; sampleData != nullptr)
        waveformDisplay.setAudioBuffer (sampleData->getAudioBuffer ());
    else
        waveformDisplay.setAudioBuffer (nullptr);
}

void ChannelEditorComponent::updateLoopPointsView ()
{
    uint32_t startSample { 0 };
    uint32_t numBytes { 0 };
    if (squidChannelProperties.getSampleDataAudioBuffer () != nullptr)
    {
        startSample = squidChannelProperties.getLoopCue ();
        numBytes = squidChannelProperties.getEndCue () - startSample;
        loopPointsView.setAudioBuffer (squidChannelProperties.getSampleDataAudioBuffer ()->getAudioBuffer ());
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
    chokeComboBox.setSelectedId (choke + 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::decayDataChanged (int decay)
{
    decayTextEditor.setText (juce::String (getUiValue (decay)), juce::NotificationType::dontSendNotification);
}

void ChannelEditorComponent::endCueDataChanged (juce::int32 endCueByteOffset)
{
    const auto endCueSampleOffset { SquidChannelProperties::byteOffsetToSampleOffset (endCueByteOffset) };
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
    sampleFileNameSelectLabel.setFileName (sampleFileName);
    // the chip is sized to the name it shows
    resized ();
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

void ChannelEditorComponent::pitchShiftDataChanged (int pitchShift)
{
    pitchShiftTextEditor.setText (juce::String (static_cast<float> (pitchShift) / 1000.0, 2), juce::NotificationType::dontSendNotification);
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
    const auto usesOwnSample { squidChannelProperties.getChannelSource () == squidChannelProperties.getChannelIndex () };
    sampleFileNameSelectLabel.setEnabled (usesOwnSample);
    sampleFileNameSelectLabel.setSourceChannel (usesOwnSample ? -1 : static_cast<int> (squidChannelProperties.getChannelSource ()));
    // the chip is sized to what it shows, which now may include the source channel
    resized ();
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

void ChannelEditorComponent::pitchShiftUiChanged (float pitchShift)
{
    squidChannelProperties.setPitchShift (static_cast<int> (pitchShift * 1000.f), false);
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

bool ChannelEditorComponent::handleSampleAssignment (const juce::StringArray& fileNames)
{
    const auto baseChannelIndex { squidChannelProperties.getChannelIndex () };
    for (auto channelOffset { 0 }; channelOffset < std::min (fileNames.size (), 8); ++channelOffset)
    {
        const auto currentChannelIndex { baseChannelIndex + channelOffset };
        //DebugLog ("ChannelEditorComponent", "handleSampleAssignment - channel " + juce::String (currentChannelIndex) + " sample to load: " + fileNames[channelOffset]);
        auto srcFile { juce::File (fileNames [channelOffset]) };
        const auto channelDirectory { juce::File (appProperties.getRecentlyUsedFile (0)).getChildFile (juce::String (currentChannelIndex + 1)) };
        if (! channelDirectory.exists ())
            channelDirectory.createDirectory ();
        auto destFile { channelDirectory.getChildFile (srcFile.withFileExtension ("_wav").getFileName ()) };
        SquidChannelProperties destChannelProperties { editManager->getChannelPropertiesVT (currentChannelIndex), SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no };
        auto currentSampleFile { juce::File (destChannelProperties.getSampleFileName ()) };
        // if the currently assigned sample is a "temp" file, we will delete it
        if (currentSampleFile.getFileExtension () == "._wav")
            currentSampleFile.deleteFile ();
        if (srcFile.getParentDirectory () != channelDirectory)
        {
            if (! editManager->copySampleToChannel (srcFile, destFile))
            {
                // TODO - indicate an error?
                return false;
            }
        }
        // TODO - we should probably handle the case of the file missing. it shouldn't happen, as the file was selected through the file manager or a drag/drop
        //        but it's possible that the file gets deleted somehow after selection
        jassert (destFile.exists ());
        editManager->loadChannel (destChannelProperties.getValueTree (), static_cast<uint8_t> (currentChannelIndex), destFile);
    }
    return true;
}

bool ChannelEditorComponent::isInterestedInFileDrag (const juce::StringArray& /*files*/)
{
    return true;
}

void ChannelEditorComponent::filesDropped (const juce::StringArray& files, int x, int y)
{
    draggingFiles = false;
    repaint ();
    if (! supportedFile)
        return;
    const auto dropBounds { juce::Rectangle<int> { 0, 0, getWidth (), cueSetButtons [0].getY () } };
    if (! dropBounds.contains (x, y))
        return;
    if (! handleSampleAssignment (files))
    {
        // TODO - indicate an error?
    }
}

void ChannelEditorComponent::fileDragEnter (const juce::StringArray& files, int /*x*/, int /*y*/)
{
    dropDetails = {};
    supportedFile = true;
    const auto baseChannelIndex { squidChannelProperties.getChannelIndex () };
    const auto numberOfAssignments { std::min (files.size (), 8) };
    for (auto channelOffset { 0 }; channelOffset < numberOfAssignments; ++channelOffset)
    {
        const auto currentChannelIndex { baseChannelIndex + channelOffset };
        auto draggedFile { juce::File (files [channelOffset]) };
        if (editManager->isSquidManagerSupportedAudioFile (draggedFile))
        {
            // The extension only says what a file claims to be. One that is damaged,
            // empty, not really audio, or a cloud placeholder that has not been
            // downloaded yet has no reader, and cannot be assigned.
            if (auto reader { editManager->getReaderFor (draggedFile) }; reader != nullptr)
            {
                const double ratio { 44100. / reader->sampleRate };
                const int actualNumSamples { static_cast<int> (reader->lengthInSamples * ratio) };
                if (actualNumSamples > kMaxSampleLength)
                    dropDetails += juce::String (dropDetails.length () > 0 ? ". " : "") + "Channel " + juce::String (currentChannelIndex + 1) + " will be truncated to 11 seconds";
            }
            else
            {
                dropMsg = "Cannot read file(s)";
                dropDetails += juce::String (dropDetails.length () > 0 ? ". " : "") + "Cannot read " + draggedFile.getFileName ();
                supportedFile = false;
            }
        }
        else
        {
            dropMsg = "Unsupported file type(s)";
            supportedFile = false;
        }
    }
    if (supportedFile)
    {
        if (files.size () == 1)
            dropMsg = "Assign sample to Channel " + juce::String (baseChannelIndex + 1);
        else
            dropMsg = "Assign samples to Channels " + juce::String (baseChannelIndex + 1) + " - " + juce::String (baseChannelIndex + numberOfAssignments);
    }

    draggingFiles = true;
    repaint ();
}

void ChannelEditorComponent::fileDragExit (const juce::StringArray&)
{
   draggingFiles = false;
   repaint ();
}

namespace
{
    // Measurements from the mockup, in one place, so the layout reads as intent.
    constexpr auto kEditorPadding { 7 };
    constexpr auto kSectionGap { 7 };
    constexpr auto kSectionHeaderHeight { 14 };
    constexpr auto kFieldHeight { 21 };
    constexpr auto kFieldWidth { 62 };
    constexpr auto kParameterRowGap { 5 };
    constexpr auto kSampleCardHeight { 37 };
    constexpr auto kSampleChipHeight { 25 };
    constexpr auto kCueHeaderHeight { 82 };
    constexpr auto kCuePointColumnWidth { 118 };
    constexpr auto kCuePointColumnGap { 8 };
    constexpr auto kCueChipRowHeight { 24 };
    constexpr auto kCueToolWidth { 22 };
    constexpr auto kSwitchWidth { 34 };
    constexpr auto kSwitchHeight { 17 };
}

void ChannelEditorComponent::resized ()
{
    auto localBounds { getLocalBounds ().reduced (kEditorPadding) };

    // ---------------- SAMPLE ----------------
    {
        constexpr auto kGap { 9 };
        sampleCardBounds = localBounds.removeFromTop (kSampleCardHeight);
        auto sampleRow { sampleCardBounds.reduced (1).reduced (8, 0) };
        sampleFileNameLabel.setBounds (sampleRow.removeFromLeft (SquidPaint::textWidth (SquidType::sectionHeader (), sampleFileNameLabel.getText ()) + 2));
        sampleRow.removeFromLeft (kGap);

        const auto toolsWidth { toolsButton.getIdealWidth () };
        toolsButton.setBounds (sampleRow.removeFromRight (toolsWidth).withSizeKeepingCentre (toolsWidth, ActionButton::kSmallHeight));
        sampleRow.removeFromRight (kGap);

        const auto chipWidth { juce::jlimit (120, 360, sampleFileNameSelectLabel.getIdealWidth ()) };
        sampleFileNameSelectLabel.setBounds (sampleRow.removeFromLeft (chipWidth).withSizeKeepingCentre (chipWidth, kSampleChipHeight));
        sampleRow.removeFromLeft (kGap);
        sampleMetaBounds = sampleRow;
    }
    localBounds.removeFromTop (kSectionGap);

    // ---------------- PARAMETERS ----------------
    // One panel divided into six named groups, each a label column and a narrow
    // value column, so the numbers line up down the group.
    {
        static constexpr auto kParameterRows { 4 };
        static constexpr auto kGroupTopPadding { 7 };
        static constexpr auto kGroupBottomPadding { 9 };
        parameterPanelBounds = localBounds.removeFromTop (1 + kGroupTopPadding + kSectionHeaderHeight + 2 + kParameterRowGap
                                                          + (kParameterRows * kFieldHeight) + ((kParameterRows - 1) * kParameterRowGap)
                                                          + kGroupBottomPadding + 1);
        const auto inner { parameterPanelBounds.reduced (1) };
        const auto columnWidth { static_cast<float> (inner.getWidth ()) / 6.0f };

        auto placeGroup = [this, inner, columnWidth] (int columnIndex, juce::Label& header,
                                                      const std::vector<std::pair<juce::Component*, juce::Component*>>& rows)
        {
            const auto left { inner.getX () + juce::roundToInt (static_cast<float> (columnIndex) * columnWidth) };
            const auto right { inner.getX () + juce::roundToInt (static_cast<float> (columnIndex + 1) * columnWidth) };
            const auto isLastColumn { columnIndex == 5 };
            if (! isLastColumn)
                parameterDividerX [static_cast<size_t> (columnIndex)] = right - 1;

            auto content { juce::Rectangle<int> { left, inner.getY (), right - left, inner.getHeight () }
                               .withTrimmedRight (isLastColumn ? 0 : 1)
                               .reduced (9, 0)
                               .withTrimmedTop (kGroupTopPadding) };
            header.setBounds (content.removeFromTop (kSectionHeaderHeight));
            content.removeFromTop (2 + kParameterRowGap);
            for (const auto& [label, field] : rows)
            {
                auto row { content.removeFromTop (kFieldHeight) };
                content.removeFromTop (kParameterRowGap);
                if (field != nullptr)
                    field->setBounds (row.removeFromRight (kFieldWidth));
                row.removeFromRight (7);
                if (label != nullptr)
                    label->setBounds (row);
            }
        };

        placeGroup (0, levelEnvHeaderLabel, { { &levelLabel,  &levelTextEditor },
                                              { &attackLabel, &attackTextEditor },
                                              { &decayLabel,  &decayTextEditor },
                                              { &outputLabel, &outputComboBox } });
        placeGroup (1, filterHeaderLabel,   { { &filterTypeLabel,      &filterTypeComboBox },
                                              { &filterFrequencyLabel, &filterFrequencyTextEditor },
                                              { &filterResonanceLabel, &filterResonanceTextEditor } });
        // Speed (channels 1-5) and Quant (channels 6-8) share the top slot; only one
        // of the pair is ever visible, so they are given the same bounds.
        placeGroup (2, qualityHeaderLabel,  { { &speedLabel,      &speedTextEditor },
                                              { &bitsLabel,       &bitsTextEditor },
                                              { &rateLabel,       &rateComboBox },
                                              { &pitchShiftLabel, &pitchShiftTextEditor } });
        quantLabel.setBounds (speedLabel.getBounds ());
        quantComboBox.setBounds (speedTextEditor.getBounds ());

        placeGroup (3, loopHeaderLabel,     { { &loopModeLabel, &loopModeComboBox },
                                              { &xfadeLabel,    &xfadeTextEditor },
                                              { &reverseLabel,  &reverseButton } });
        placeGroup (4, triggerHeaderLabel,  { { &chokeLabel,         &chokeComboBox },
                                              { &eTrigLabel,         &eTrigComboBox },
                                              { &stepsLabel,         &stepsComboBox },
                                              { &channelSourceLabel, &channelSourceComboBox } });
        placeGroup (5, cueTriggerHeaderLabel, { { &cueRandomLabel, &cueRandomButton },
                                                { &cueStepLabel,   &cueStepButton } });

        // A slide switch reads as a switch at its own size; stretched to the width of
        // a value field it just looks like a broken text box. Right aligned, so the
        // switches line up with the value fields above them.
        for (auto* slideSwitch : { &reverseButton, &cueRandomButton, &cueStepButton })
            slideSwitch->setBounds (slideSwitch->getBounds ().removeFromRight (kSwitchWidth).withSizeKeepingCentre (kSwitchWidth, kSwitchHeight));
    }
    localBounds.removeFromTop (kSectionGap);

    // ---------------- CV ASSIGN, along the bottom ----------------
    // the editor draws its own header band, with the CV tabs in it
    cvAssignCardBounds = localBounds.removeFromBottom (1 + CvAssignEditor::kHeaderHeight + CvAssignEditor::kMatrixHeight + 1);
    cvAssignEditor.setBounds (cvAssignCardBounds.reduced (1));
    localBounds.removeFromBottom (kSectionGap);

    // ---------------- CUE POINTS, CUE SETS AND WAVEFORM fill what is left ----------------
    cueSetsCardBounds = localBounds;
    auto cueCard { localBounds.reduced (1) };

    // The cue point numbers sit directly above the waveform they move, colour
    // matched to their markers, with the loop tuner filling the rest of the band.
    {
        cueHeaderBounds = cueCard.removeFromTop (kCueHeaderHeight);
        auto header { cueHeaderBounds.withTrimmedBottom (1).reduced (8, 6) };

        auto cuePointsColumn { header.removeFromLeft ((kCuePointColumnWidth * 3) + (kCuePointColumnGap * 2)) };
        header.removeFromLeft (16);

        cuePointsHeaderLabel.setBounds (cuePointsColumn.removeFromTop (kSectionHeaderHeight));
        cuePointsColumn.removeFromTop (6);
        auto cueFieldsRow { cuePointsColumn.removeFromTop (kFieldHeight) };
        auto placeCuePoint = [&cueFieldsRow] (MarkerSwatch& swatch, juce::Label& label, juce::Component& field)
        {
            auto group { cueFieldsRow.removeFromLeft (kCuePointColumnWidth) };
            cueFieldsRow.removeFromLeft (kCuePointColumnGap);
            // never taller than its row, so it disappears with the row when the card is squeezed shut
            swatch.setBounds (group.removeFromLeft (3).withSizeKeepingCentre (3, std::min (15, group.getHeight ())));
            group.removeFromLeft (6);
            label.setBounds (group.removeFromLeft (32));
            group.removeFromLeft (6);
            field.setBounds (group);
        };
        placeCuePoint (startCueSwatch, startCueLabel, startCueTextEditor);
        placeCuePoint (loopCueSwatch,  loopCueLabel,  loopCueTextEditor);
        placeCuePoint (endCueSwatch,   endCueLabel,   endCueTextEditor);

        cuePointsColumn.removeFromTop (6);
        auto transportRow { cuePointsColumn.removeFromTop (TransportButton::kHeight) };
        const auto transportWidth { (transportRow.getWidth () - kCuePointColumnGap) / 2 };
        oneShotPlayButton.setBounds (transportRow.removeFromLeft (transportWidth));
        loopPlayButton.setBounds (transportRow.removeFromRight (transportWidth));

        loopTunerHeaderLabel.setBounds (header.removeFromTop (kSectionHeaderHeight));
        header.removeFromTop (6);
        loopPointsView.setBounds (header);
    }

    // 32 chips above the waveform and 32 below, spanning the card
    topCueChipBounds = cueCard.removeFromTop (kCueChipRowHeight + 1);
    bottomCueChipBounds = cueCard.removeFromBottom (kCueChipRowHeight + 1);
    auto placeChips = [this] (juce::Rectangle<int> rowBounds, size_t firstChip)
    {
        static constexpr auto kChipsPerRow { 32 };
        static constexpr auto kChipGap { 1.0f };
        const auto row { rowBounds.reduced (3) };
        const auto chipWidth { (static_cast<float> (row.getWidth ()) - (kChipGap * (kChipsPerRow - 1))) / kChipsPerRow };
        for (auto chipIndex { 0 }; chipIndex < kChipsPerRow; ++chipIndex)
        {
            const auto chipLeft { row.getX () + juce::roundToInt (static_cast<float> (chipIndex) * (chipWidth + kChipGap)) };
            const auto chipRight { row.getX () + juce::roundToInt ((static_cast<float> (chipIndex) * (chipWidth + kChipGap)) + chipWidth) };
            cueSetButtons [firstChip + static_cast<size_t> (chipIndex)].setBounds (chipLeft, row.getY (), chipRight - chipLeft, row.getHeight ());
        }
    };
    placeChips (topCueChipBounds.withTrimmedBottom (1), 0);
    placeChips (bottomCueChipBounds.withTrimmedTop (1), 32);

    // the waveform menu level with the ruler, then add and delete below it, split by hairlines
    cueToolBounds = cueCard.removeFromLeft (kCueToolWidth + 1);
    auto cueTools { cueToolBounds.withTrimmedRight (1) };
    waveformToolsButton.setBounds (cueTools.removeFromTop (WaveformDisplay::kTimelineHeight).withTrimmedBottom (1));
    addCueSetButton.setBounds (cueTools.removeFromTop (cueTools.getHeight () / 2).withTrimmedBottom (1));
    deleteCueSetButton.setBounds (cueTools);
    waveformDisplay.setBounds (cueCard);
}

void ChannelEditorComponent::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    applyExplicitColours ();
}

void ChannelEditorComponent::applyExplicitColours ()
{
    // Labels keep per-instance colours, so they have to be refreshed by hand when
    // the palette changes.
    auto tint = [this] (juce::Label& label, int colourId)
    {
        label.setColour (juce::Label::ColourIds::textColourId, findColour (colourId));
    };

    for (auto* header : { &levelEnvHeaderLabel, &filterHeaderLabel, &qualityHeaderLabel,
                          &loopHeaderLabel, &triggerHeaderLabel, &cueTriggerHeaderLabel,
                          &cuePointsHeaderLabel, &loopTunerHeaderLabel, &sampleFileNameLabel })
        tint (*header, SquidColours::accentText);

    for (auto* label : { &levelLabel, &attackLabel, &decayLabel, &outputLabel, &filterTypeLabel,
                         &speedLabel, &quantLabel, &bitsLabel, &rateLabel, &pitchShiftLabel,
                         &loopModeLabel, &xfadeLabel, &reverseLabel, &chokeLabel, &eTrigLabel,
                         &stepsLabel, &channelSourceLabel, &cueRandomLabel, &cueStepLabel,
                         &startCueLabel, &loopCueLabel, &endCueLabel })
        tint (*label, SquidColours::textDim);

    // a parameter that does nothing in the current filter mode says so in its label too
    const auto filterEnabled { filterFrequencyTextEditor.isEnabled () };
    tint (filterFrequencyLabel, filterEnabled ? SquidColours::textDim : SquidColours::textGhost);
    tint (filterResonanceLabel, filterEnabled ? SquidColours::textDim : SquidColours::textGhost);
}

void ChannelEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (SquidColours::windowBackground));

    // Each section is a panel lifted off the background, so the editor reads as a few
    // blocks rather than one field of controls.
    const std::array<juce::Rectangle<int>, 4> cards { sampleCardBounds, parameterPanelBounds, cueSetsCardBounds, cvAssignCardBounds };
    for (const auto& card : cards)
        if (! card.isEmpty ())
            SquidPaint::card (g, *this, card);

    const auto outline { findColour (SquidColours::outline) };

    // the parameter groups are separated by hairlines running the panel's height
    g.setColour (outline);
    for (const auto dividerX : parameterDividerX)
        if (dividerX > 0)
            g.fillRect (dividerX, parameterPanelBounds.getY () + 1, 1, parameterPanelBounds.getHeight () - 2);

    // the bands inside the cue card sit a step above the card, each with its hairline
    const auto band { findColour (SquidColours::panelHeader) };
    g.setColour (band);
    g.fillRect (cueHeaderBounds.withTrimmedBottom (1));
    g.fillRect (topCueChipBounds.withTrimmedBottom (1));
    g.fillRect (bottomCueChipBounds.withTrimmedTop (1));
    g.setColour (outline);
    g.fillRect (cueHeaderBounds.withTop (cueHeaderBounds.getBottom () - 1));
    g.fillRect (topCueChipBounds.withTop (topCueChipBounds.getBottom () - 1));
    g.fillRect (bottomCueChipBounds.withHeight (1));
    // behind the cue tools, so the gap between them and their right edge read as hairlines
    g.fillRect (cueToolBounds);

    // the bands cover the card's rounded corners, so its outline goes back on top
    for (const auto& card : cards)
        if (! card.isEmpty ())
        {
            g.setColour (outline);
            g.drawRoundedRectangle (card.toFloat ().reduced (0.5f), 3.0f, 1.0f);
        }

    // "0.30 s  .  13,125 samples" - the numbers in the dim ink, their units muted
    if (sampleSecondsText.isNotEmpty ())
    {
        const auto font { SquidType::meta () };
        const auto separator { juce::String (juce::CharPointer_UTF8 (" s \xc2\xb7 ")) };
        auto metaBounds { sampleMetaBounds };
        g.setFont (font);
        auto drawPiece = [&g, &metaBounds, &font] (const juce::String& text, juce::Colour colour)
        {
            const auto width { SquidPaint::textWidth (font, text) };
            g.setColour (colour);
            g.drawText (text, metaBounds.removeFromLeft (width), juce::Justification::centredLeft, false);
        };
        const auto numberColour { findColour (SquidColours::textDim) };
        const auto unitColour { findColour (SquidColours::menuHeaderText) };
        drawPiece (sampleSecondsText, numberColour);
        drawPiece (separator, unitColour);
        drawPiece (sampleCountText, numberColour);
        drawPiece (" samples", unitColour);
    }
}

void ChannelEditorComponent::paintOverChildren (juce::Graphics& g)
{
    constexpr auto dropMsgFontSizeDouble { 30.f };
    constexpr auto dropDetailsFontSize { 20.f };
    constexpr auto sectionSpacing { 5.f };
    if (draggingFiles)
    {
        if (dropDetails.isEmpty ())
        {
            constexpr auto fontHeight { 30.f };
            const auto dropBounds { juce::Rectangle<int> { 0, 0, getWidth (), cueSetButtons [0].getY () } };
            g.setColour (findColour (SquidColours::dropOverlay));
            g.fillRect (dropBounds);
            g.setFont (fontHeight);

            auto stringWidthPixels { juce::GlyphArrangement::getStringWidth (g.getCurrentFont (), dropMsg) + 10.f };
            auto center { dropBounds.getCentre () };
            SquidPaint::messagePlate (g, *this, { static_cast<float> (center.getX ()) - (stringWidthPixels / 2.f), static_cast<float> (center.getY ()) - (fontHeight / 2.f), stringWidthPixels, fontHeight + 5.f }, 10.f);
            g.setColour (SquidPaint::messageInk (*this, ! supportedFile));
            g.drawText (dropMsg, dropBounds, juce::Justification::centred, false);
        }
        else
        {
            juce::StringArray words;
            words.addTokens (dropDetails, false);
            juce::StringArray lines;
            g.setFont (dropDetailsFontSize);
            for (auto& word : words)
            {
                if (lines.isEmpty ())
                    lines.add (word);
                else
                {
                    auto lastLine { lines [lines.size () - 1] };
                    if (juce::GlyphArrangement::getStringWidth (g.getCurrentFont (), lastLine + " " + word) < getWidth () - 40)
                        lines.set (lines.size () - 1, lastLine + " " + word);
                    else
                        lines.add (word);
                }
            }
            auto longestLine = [&g, lines] ()
            {
                auto lineLength { 0.f };
                auto maxLineLength { 0.f };
                for (auto& line : lines)
                {
                    lineLength = std::max (lineLength, juce::GlyphArrangement::getStringWidth (g.getCurrentFont (), line));
                    if (lineLength > maxLineLength)
                        maxLineLength = lineLength;
                }
                return maxLineLength;
            } ();
            //
            // dropMsg
            //
            // dropDetails * numLines
            //
            const auto dropBounds { juce::Rectangle<int> { 0, 0, getWidth (), cueSetButtons [0].getY () } };
            auto dropAreaBounds { dropBounds };

            const auto detailsHeight { std::max (5, lines.size ()) * dropDetailsFontSize + (sectionSpacing * 2) };
            const auto detailsBounds { dropAreaBounds.removeFromBottom (static_cast<int> (detailsHeight)).reduced (0, static_cast<int> (sectionSpacing)) };
            const auto dropMsgBounds { juce::Rectangle<int> (0, 0, dropAreaBounds.getWidth (), static_cast<int> (dropMsgFontSizeDouble)).withCentre (dropAreaBounds.getCentre ()) };

            // fill entire drop area with transparent
            g.setColour (findColour (SquidColours::dropOverlay));
            g.fillRect (dropBounds);

            // display main drop message background
            // draw main drop message
            g.setFont (dropMsgFontSizeDouble);
            SquidPaint::messagePlate (g, *this, dropMsgBounds.toFloat ().withWidth (juce::GlyphArrangement::getStringWidth (g.getCurrentFont (), dropMsg) + 10.f).withCentre (dropMsgBounds.getCentre ().toFloat ()).withY (dropMsgBounds.getY () + 2.f), 10.f);
            g.setColour (SquidPaint::messageInk (*this));
            g.drawText (dropMsg, dropMsgBounds, juce::Justification::centred, false);

            // calculate background rectangle size from longest line and number of lines, using ellipsis if necessary
            g.setFont (dropDetailsFontSize);
            auto dropDetailsDisplayBounds { juce::Rectangle<int> { 0, 0, static_cast<int> (longestLine + 20), lines.size () * static_cast<int> (dropDetailsFontSize) }.withCentre (detailsBounds.getCentre ()) };
            //dropDetailsDisplayBounds.setY (detailsBounds.getHeight () / 2.f - dropDetailsDisplayBounds.getHeight () / 2.f );
            SquidPaint::messagePlate (g, *this, dropDetailsDisplayBounds.toFloat (), 10.f);
            g.setColour (SquidPaint::messageInk (*this));
            for (auto lineIndex { 0 }; lineIndex < lines.size (); ++lineIndex)
            {
                g.drawText (lines [lineIndex], dropDetailsDisplayBounds.removeFromTop (static_cast<int> (dropDetailsFontSize)), juce::Justification::centred, false);
            }
        }
    }
}

void ChannelEditorComponent::filesDroppedOnCueSetEditor (const juce::StringArray& files, juce::String outputFileName, juce::ValueTree cueSets)
{
    editManager->concatenateAndBuildCueSets (files, squidChannelProperties.getChannelIndex (), outputFileName, cueSets);
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
            editManager->setAltOutput (channelIndex, false);
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
