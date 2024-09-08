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
    bankNameEditor.setTooltip ("Bank Name. Maximum of 11 characters long. Stored in the info.txt file in the bank folder");
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

    // TOOLS BUTTON
    toolsButton.setButtonText ("TOOLS");
    toolsButton.onClick = [this] ()
    {
        auto* popupMenuLnF { new juce::LookAndFeel_V4 };
        popupMenuLnF->setColour (juce::PopupMenu::ColourIds::headerTextColourId, juce::Colours::white.withAlpha (0.3f));
        juce::PopupMenu pm;
        pm.setLookAndFeel (popupMenuLnF);
        pm.addSectionHeader ("Bank");
        pm.addSeparator ();

        //pm.addItem ("Import", false, false, [this] () { /*importBank ();*/ });
        //pm.addItem ("Export", false, false, [this] () { /*exportBank ();*/ });
        pm.addItem ("Default", true, false, [this] ()
        {
            editManager->setBankDefaults ();
        });
        pm.addItem ("Revert", true, false, [this] ()
        {
            editManager->setBankUnedited ();
        });

        pm.showMenuAsync ({}, [this, popupMenuLnF] (int) { delete popupMenuLnF; });
    };
    addAndMakeVisible (toolsButton);

    // CHANNEL TABS
    channelTabs.isSupportedFile = [this] (juce::String fileName) { return editManager->getFileInfo (fileName).supported; };
    channelTabs.loadFile = [this] (juce::String fileName, int channelIndex)
    {
        channelTabs.setCurrentTabIndex (channelIndex, true);
        return channelEditorComponents [channelIndex].loadFile (fileName);
    };
    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
    {
        channelTabs.addTab ("CH " + juce::String::charToString ('1' + curChannelIndex), juce::Colours::darkgrey, &channelEditorComponents [curChannelIndex], false);
    }
    addAndMakeVisible (channelTabs);
    channelTabs.onSelectedTabChanged = [this] (int)
    {
        audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
    };

    // the Channel tabs are overlaying the Tools button, move the load button to front
    toolsButton.toFront (false);

    // timer to set enabled state of the save button based on if the bank data has changed
    startTimer (250);
}

void SquidEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);
    runtimeRootProperties.onSystemRequestedQuit = [this] ()
    {
        audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
        runtimeRootProperties.setPreferredQuitState (RuntimeRootProperties::QuitState::idle, false);
        bankLoseEditWarning ("Exiting SquidManager", [this] ()
        {
            editManager->cleanUpTempFiles (appProperties.getRecentlyUsedFile (0));
            juce::MessageManager::callAsync ([this] () { runtimeRootProperties.setQuitState (RuntimeRootProperties::QuitState::now, false); });
        }, [this] ()
        {
            // do nothing
        });
    };
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();

    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::client, AudioPlayerProperties::EnableCallbacks::yes);

    BankManagerProperties bankManagerProperties (runtimeRootProperties.getValueTree (), BankManagerProperties::WrapperType::owner, BankManagerProperties::EnableCallbacks::no);
    unEditedSquidBankProperties.wrap (bankManagerProperties.getBank("unedited"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.wrap (bankManagerProperties.getBank ("edit"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.forEachChannel ([this, &rootPropertiesVT] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        channelEditorComponents [channelIndex].init (channelPropertiesVT, rootPropertiesVT);
        return true;
    });
    squidBankProperties.onNameChange = [this] (juce::String name) { nameDataChanged (name); };
    squidBankProperties.onLoadBegin = [this] ()
    {
        audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
    };
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

void SquidEditorComponent::bankLoseEditWarning (juce::String title, std::function<void ()> overwriteFunction, std::function<void ()> cancelFunction)
{
    jassert (overwriteFunction != nullptr);
    jassert (cancelFunction != nullptr);

    if (BankHelpers::areEntireBanksEqual (unEditedSquidBankProperties.getValueTree (), squidBankProperties.getValueTree ()))
    {
        overwriteFunction ();
    }
    else
    {
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, title,
            "You have not saved a Bank that you have edited.\n  Select Continue to lose your changes.\n  Select Cancel to go back and save.", "Continue (lose changes)", "Cancel", nullptr,
            juce::ModalCallbackFunction::create ([this, overwriteFunction, cancelFunction] (int option)
            {
                juce::MessageManager::callAsync ([this, option, overwriteFunction, cancelFunction] ()
                {
                    if (option == 1) // Continue
                        overwriteFunction ();
                    else // cancel
                        cancelFunction ();
                });
            }));
    }
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
    toolsButton.setBounds (saveButton.getBounds ().withY (saveButton.getBottom () + 3));

    const auto channelSectionY { saveButton.getBottom () + 3 };
    const auto kWidthOfWaveformEditor { 962 };
    channelTabs.setBounds (3, channelSectionY, kWidthOfWaveformEditor + 30, getHeight() - channelSectionY - 5);
}

void SquidEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}
