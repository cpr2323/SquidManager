#include "SquidEditor.h"
#include "../../SquidSalmple/Bank/BankHelpers.h"
#include "../../SquidSalmple/Bank/BankManagerProperties.h"
#include "../../SystemServices.h"
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
    setupTextEditor (bankNameEditor, juce::Justification::centredLeft, 12, " !\"#$%^&'()#+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");

    // SAVE BUTTON
    saveButton.setButtonText ("SAVE");
    saveButton.setEnabled (false);
    saveButton.onClick = [this] ()
    {
        // TODO - if there is no sample loaded, we need to emulate what the module does, which is to create and empty wav file before saving the metadata
        editManager->saveBank ();
    };
    addAndMakeVisible (saveButton);

    // TODO - the load button will eventually be removed, as loading will be done via a file system/bank browser
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
                jassert (editManager != nullptr);
                editManager->loadBank (bankDirectoryToLoad);
            }
        }, nullptr);
    };
    addAndMakeVisible (loadButton);

    // CHANNEL TABS
    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
    {
        channelTabs.addTab ("CH " + juce::String::charToString ('1' + curChannelIndex), juce::Colours::darkgrey, &channelEditorComponents [curChannelIndex], false);
    }
    addAndMakeVisible (channelTabs);

    // TODO - remove eventually
    // the Channel tabs are overlaying the Load button (which will be removed eventually, as loading will be done via a file system/bank browser
    // but for now we will just move the load button to front
    loadButton.toFront (false);

    // timer to set enabled state of the save button based on if the bank data has changed
    startTimer (250);
}

void SquidEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();

    BankManagerProperties bankManagerProperties (runtimeRootProperties.getValueTree (), BankManagerProperties::WrapperType::owner, BankManagerProperties::EnableCallbacks::no);
    unEditedSquidBankProperties.wrap (bankManagerProperties.getBank("unedited"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.wrap (bankManagerProperties.getBank ("edit"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
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
    saveButton.setEnabled (! BankHelpers::areEntireBanksEqual (unEditedSquidBankProperties.getValueTree (), squidBankProperties.getValueTree ()));
}

void SquidEditorComponent::resized ()
{
    auto localBounds { getLocalBounds() };

    localBounds.removeFromTop (5);
    // put bank name and save button on the top line
    auto topRowBounds {localBounds.removeFromTop (kParameterLineHeight) };
    topRowBounds.removeFromLeft (5);
    bankNameLabel.setBounds (topRowBounds.removeFromLeft (45));
    topRowBounds.removeFromLeft (3);
    bankNameEditor.setBounds (topRowBounds.removeFromLeft (80));
    topRowBounds.removeFromRight (5);
    saveButton.setBounds (topRowBounds.removeFromRight (80));
    loadButton.setBounds (saveButton.getBounds ().withY (saveButton.getBottom () + 3));

    const auto channelSectionY { saveButton.getBottom () + 3 };
    const auto kWidthOfWaveformEditor { 962 };
    channelTabs.setBounds (3, channelSectionY, kWidthOfWaveformEditor + 30, getHeight() - channelSectionY - 5);
}

void SquidEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}
