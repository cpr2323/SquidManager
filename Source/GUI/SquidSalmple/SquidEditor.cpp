#include "SquidEditor.h"
#include "../Theme/SquidColourIds.h"
#include "../../SquidSalmple/Bank/BankHelpers.h"
#include "../../SquidSalmple/Bank/BankManagerProperties.h"
#include "../../SquidSalmple/Metadata/SquidSalmpleDefs.h"
#include "../../SystemServices.h"
#include "oolib/Properties/PersistentRootProperties.h"

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
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setFont (label.getFont ().withPointHeight (fontSize));
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    auto setupTextEditor = [this] (juce::TextEditor& textEditor, juce::Justification justification, int maxLen, juce::String validInputCharacters)
    {
        textEditor.setJustification (justification);
        textEditor.setIndents (1, 0);
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        addAndMakeVisible (textEditor);
    };

    // NAME
    setupLabel (bankNameLabel, "BANK", kMediumLabelSize, juce::Justification::centredLeft);
    setupLabel (unsavedEditsLabel, "", 12.0f, juce::Justification::centredLeft);
    bankNameEditor.setTooltip ("Bank Name. Maximum of 11 characters long. Stored in the info.txt file in the bank folder");
    bankNameEditor.onFocusLost = [this] () { nameUiChanged (bankNameEditor.getText ()); };
    bankNameEditor.onReturnKey = [this] () { nameUiChanged (bankNameEditor.getText ()); };
    bankNameEditor.onTextChange = [this] () { nameUiChanged (bankNameEditor.getText ()); };
    // TODO - make sure I have the correct valid character set
    setupTextEditor (bankNameEditor, juce::Justification::centredLeft, 12, " !\"#$%^&'()#+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");

    // SAVE BUTTON
    saveButton.setButtonText ("SAVE BANK");
    saveButton.setEnabled (false);
    saveButton.onClick = [this] ()
    {
        // determine if any of the channels have metadata that is older than the current version, so we can warn the user
        auto needToWarnAboutVersionOverwrite { false };
        squidBankProperties.forEachChannel ([this, &needToWarnAboutVersionOverwrite] (juce::ValueTree channelPropertiesVT, [[maybe_unused]] int channelIndex)
        {
            SquidChannelProperties squidChannelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
            if (squidChannelProperties.getLoadedVersion () < static_cast<uint8_t> (kSignatureAndVersionCurrent & 0xFF))
            {
                needToWarnAboutVersionOverwrite = true;
                return false;
            }
            return true;
        });
        auto doSave = [this] ()
        {
            // TODO - if there is no sample loaded, we need to emulate what the module does, which is to create and empty wav file before saving the metadata
            editManager->saveBank ();
        };
        if (needToWarnAboutVersionOverwrite)
        {
            juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "OVERWRITE PREVIOUS METADATA VERSION", "One of more of the sample files contain Squid Salmple metadata that is a different format than the latest firmware. Saving will overwrite that with the newest format.\n\r Are you sure you want to do this?", "YES", "NO", nullptr,
                juce::ModalCallbackFunction::create ([this, doSave] (int option)
                {
                    if (option == 0) // no
                        return;
                    doSave ();
                }));
        }
        else
        {
            doSave ();
        }
    };
    addAndMakeVisible (saveButton);

    // TOOLS BUTTON
    toolsButton.onClick = [this] ()
    {
        juce::PopupMenu pm;
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

        pm.showMenuAsync ({});
    };
    addAndMakeVisible (toolsButton);

    // CHANNEL TABS
    channelTabs.isSupportedFile = [this] (juce::String fileName) { return editManager->isSquidManagerSupportedAudioFile (fileName); };
    channelTabs.loadFile = [this] (juce::String fileName, int channelIndex)
    {
        channelTabs.setCurrentTabIndex (channelIndex, true);
        return channelEditorComponents [channelIndex].loadFile (fileName);
    };
    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
    {
        channelTabs.addTab ("CH " + juce::String::charToString ('1' + curChannelIndex), findColour (SquidColours::tabBackground), &channelEditorComponents [curChannelIndex], false);
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
    unEditedSquidBankProperties.wrap (bankManagerProperties.getBank ("unedited"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
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
    const auto hasUnsavedEdits { ! BankHelpers::areEntireBanksEqual (unEditedSquidBankProperties.getValueTree (), squidBankProperties.getValueTree ()) };
    saveButton.setEnabled (hasUnsavedEdits);
    if (hasUnsavedEdits != bankHasUnsavedEdits)
    {
        bankHasUnsavedEdits = hasUnsavedEdits;
        // the app cannot say which value changed, only that something did
        unsavedEditsLabel.setText (hasUnsavedEdits ? "UNSAVED EDITS" : "", juce::NotificationType::dontSendNotification);
        applyExplicitColours ();
    }

    // a channel tab lights when that channel holds a sample
    squidBankProperties.forEachChannel ([this] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        SquidChannelProperties channelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
        channelTabs.setChannelHasContent (channelIndex, channelProperties.getSampleFileName ().isNotEmpty ());
        return true;
    });
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
    auto localBounds { getLocalBounds () };

    localBounds.removeFromTop (4);
    auto topRowBounds { localBounds.removeFromTop (24) };
    topRowBounds.removeFromLeft (10);
    bankNameLabel.setBounds (topRowBounds.removeFromLeft (40));
    topRowBounds.removeFromLeft (4);
    bankNameEditor.setBounds (topRowBounds.removeFromLeft (110));
    topRowBounds.removeFromLeft (12);
    unsavedEditsLabel.setBounds (topRowBounds.removeFromLeft (120));

    topRowBounds.removeFromRight (10);
    saveButton.setBounds (topRowBounds.removeFromRight (100));
    topRowBounds.removeFromRight (6);
    toolsButton.setBounds (topRowBounds.removeFromRight (100));

    const auto channelSectionY { saveButton.getBottom () + 4 };
    const auto kWidthOfWaveformEditor { 1082 };
    channelTabs.setBounds (3, channelSectionY, kWidthOfWaveformEditor + 30, getHeight () - channelSectionY - 5);
}

void SquidEditorComponent::applyExplicitColours ()
{
    bankNameLabel.setColour (juce::Label::ColourIds::textColourId, findColour (SquidColours::accentText));
    unsavedEditsLabel.setColour (juce::Label::ColourIds::textColourId, findColour (SquidColours::unsavedEdits));
    // Save is the one action in this header worth making obvious - but only when
    // there is something to save. JUCE does not dim a disabled button's fill, so
    // an always-accent Save would look ready to press with nothing to write.
    saveButton.setColour (juce::TextButton::ColourIds::buttonColourId,
                          findColour (bankHasUnsavedEdits ? SquidColours::accent : SquidColours::buttonBackground));
    saveButton.setColour (juce::TextButton::ColourIds::textColourOffId,
                          findColour (bankHasUnsavedEdits ? SquidColours::accentInk : SquidColours::textDim));
}

void SquidEditorComponent::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    applyExplicitColours ();
    // TabbedComponent stores a colour per tab rather than resolving one, so a
    // palette change has to be pushed into them
    for (auto tabIndex { 0 }; tabIndex < channelTabs.getNumTabs (); ++tabIndex)
        channelTabs.setTabBackgroundColour (tabIndex, findColour (SquidColours::tabBackground));

    // a plain TextEditor stores a colour with the text it already holds, so the
    // existing contents have to be re-tinted (oolib's CustomTextEditor does this
    // for itself, which is why the parameter fields do not need it here)
    bankNameEditor.applyColourToAllText (findColour (juce::TextEditor::textColourId), true);
}

void SquidEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (SquidColours::windowBackground));
}
