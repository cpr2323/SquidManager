#include "SquidEditor.h"
#include "../../SquidSalmple/Metadata/SquidMetaDataReader.h"
#include "../../Utility/PersistentRootProperties.h"

const auto kParameterLineHeight { 20 };
const auto kInterControlYOffset { 2 };
const auto kInitialYOffset { 5 };
static const auto kMediumLabelSize { 14.0f };

const auto kScaleMax { 65535. };
const auto kScaleStep { kScaleMax / 100 };

SquidEditorComponent::SquidEditorComponent ()
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
    auto setupTextEditor = [this] (juce::TextEditor& textEditor, juce::Justification justification, int maxLen, juce::String validInputCharacters)
    {
        textEditor.setJustification (justification);
        textEditor.setIndents (1, 0);
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        textEditor.setColour (juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::black);
        addAndMakeVisible (textEditor);
    };

    // NAME
    setupLabel (bankNameLabel, "NAME", kMediumLabelSize, juce::Justification::centred);
    bankNameEditor.onFocusLost = [this] () { nameUiChanged (bankNameEditor.getText ()); };
    bankNameEditor.onReturnKey = [this] () { nameUiChanged (bankNameEditor.getText ()); };
    // TODO - make sure I have the correct valid character set
    setupTextEditor (bankNameEditor, juce::Justification::centredLeft, 0, " !\"#$%^&'()#+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");

    // LOAD BUTTON
    loadButton.setButtonText ("LOAD");
    loadButton.onClick = [this] ()
    {
        fileChooser.reset (new juce::FileChooser ("Please select the bank directory to load...", {}, ""));
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            {
                auto bankDirectoryToLoad { fc.getURLResults () [0].getLocalFile () };
                appProperties.addRecentlyUsedFile (bankDirectoryToLoad.getFullPathName ());

                // check for info.txt
                auto infoTxtFile { bankDirectoryToLoad.getChildFile ("info.txt") };
                // read bank name if file exists
                if (infoTxtFile.exists ())
                {
                    auto infoTxtInputStream { infoTxtFile.createInputStream () };
                    auto firstLine { infoTxtInputStream->readNextLine () };
                    squidBankProperties.setName (firstLine.substring (0, 11), true);
                }
                // iterate over the channel folders and load sample 
                for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
                {
                    auto channelDirectory { bankDirectoryToLoad.getChildFile (juce::String (channelIndex + 1)) };
                    juce::File sampleFile;
                    if (channelDirectory.exists () && channelDirectory.isDirectory ())
                    {
                        for (const auto& entry : juce::RangedDirectoryIterator (channelDirectory.getFullPathName (), false, "*.wav", juce::File::findFiles))
                        {
                            sampleFile = entry.getFile ();
                            break;
                        }
                    }
                    else
                    {
                        // TODO - add support to convert old style bank files "chan-00X.wav"
                        // if file exits
                        //      create folder
                        //      copy file
                    }
                    if (sampleFile.exists ())
                    {
                        // TODO - check for import errors and handle accordingly
                        SquidMetaDataReader squidMetaDataReader;
                        SquidChannelProperties loadedSquidMetaDataProperties { squidMetaDataReader.read (sampleFile),
                                                                                SquidChannelProperties::WrapperType::owner,
                                                                                SquidChannelProperties::EnableCallbacks::no };
                        SquidChannelProperties squidChannelProperties { squidBankProperties.getChannelVT (channelIndex),
                                                                         SquidChannelProperties::WrapperType::client,
                                                                         SquidChannelProperties::EnableCallbacks::no };
                        squidChannelProperties.copyFrom (loadedSquidMetaDataProperties.getValueTree ());
                        channelEditorComponents [channelIndex].initCueSetTabs ();
                        channelEditorComponents [channelIndex].initWaveformDisplay (sampleFile, squidChannelProperties.getCurCueSet ());

                    }
                }
            }
        }, nullptr);
    };
    addAndMakeVisible (loadButton);

    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
    {
        channelTabs.addTab ("CH " + juce::String::charToString ('1' + curChannelIndex), juce::Colours::darkgrey, &channelEditorComponents [curChannelIndex], false);
    }
    addAndMakeVisible (channelTabs);

    startTimer (250);
}

void SquidEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    squidBankProperties.wrap (runtimeRootProperties.getValueTree (), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.forEachChannel ([this, &rootPropertiesVT] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        channelEditorComponents [channelIndex].init (channelPropertiesVT, rootPropertiesVT);
        return true;
    });
    squidBankProperties.onNameChange = [this] (juce::String name) { nameDataChanged (name); };
    nameDataChanged (squidBankProperties.getName ());
}

void SquidEditorComponent::nameUiChanged (juce::String name)
{
    squidBankProperties.setName (name, false);
}

void SquidEditorComponent::nameDataChanged (juce::String name)
{
    bankNameEditor.setText (name, juce::NotificationType::dontSendNotification);
}

void SquidEditorComponent::timerCallback ()
{
    // check if data has changed
}

void SquidEditorComponent::resized ()
{
    auto localBounds { getLocalBounds() };

    localBounds.removeFromTop (5);
    auto topRowBounds {localBounds.removeFromTop (kParameterLineHeight) };
    topRowBounds.removeFromLeft (5);
    bankNameLabel.setBounds (topRowBounds.removeFromLeft (45));
    topRowBounds.removeFromLeft (3);
    bankNameEditor.setBounds (topRowBounds.removeFromLeft (80));
    topRowBounds.removeFromRight (5);
    loadButton.setBounds (topRowBounds.removeFromRight (80));
    const auto channelSectionY { loadButton.getBottom () + 3 };
    const auto kWidthOfWaveformEditor { 962 };
    channelTabs.setBounds (3, channelSectionY, kWidthOfWaveformEditor + 30, getHeight() - channelSectionY - 5);
}

void SquidEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}
