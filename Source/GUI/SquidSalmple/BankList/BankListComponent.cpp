#include "BankListComponent.h"
// #include "../../../Assimil8or/Assimil8orPreset.h"
// #include "../../../Assimil8or/FileTypeHelpers.h"
// #include "../../../Assimil8or/PresetManagerProperties.h"
// #include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../SystemServices.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/WatchDogTimer.h"

#define LOG_BANK_LIST 1
#if LOG_BANK_LIST
#define LogBankList(text) DebugLog ("BankListComponent", text);
#else
#define LogBankList(text) ;
#endif

BankListComponent::BankListComponent ()
{
    showAllBanks.setToggleState (true, juce::NotificationType::dontSendNotification);
    showAllBanks.setButtonText ("Show All");
    showAllBanks.setTooltip ("Show all Banks, Show only existing Banks");
    showAllBanks.onClick = [this] () { checkBanksThread.start (); };
    addAndMakeVisible (showAllBanks);
    addAndMakeVisible (bankListBox);

    checkBanksThread.onThreadLoop = [this] ()
    {
        checkBanks ();
        return false;
    };
}

void BankListComponent::init (juce::ValueTree rootPropertiesVT)
{
    LogBankList ("BankListComponent::init");
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();

    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    directoryDataProperties.onRootScanComplete = [this] ()
    {
        LogBankList ("BankListComponent::init - directoryDataProperties.onRootScanComplete");
        if (! checkBanksThread.isThreadRunning ())
        {
            LogBankList ("BankListComponent::init - directoryDataProperties.onRootScanComplete - starting thread");
            checkBanksThread.startThread ();
        }
        else
        {
            LogBankList ("BankListComponent::init - directoryDataProperties.onRootScanComplete - starting timer");
            startTimer (1);
        }
    };
//     directoryDataProperties.onStatusChange = [this] (DirectoryDataProperties::ScanStatus status)
//     {
//         switch (status)
//         {
//             case DirectoryDataProperties::ScanStatus::empty:
//             {
//             }
//             break;
//             case DirectoryDataProperties::ScanStatus::scanning:
//             {
//             }
//             break;
//             case DirectoryDataProperties::ScanStatus::canceled:
//             {
//             }
//             break;
//             case DirectoryDataProperties::ScanStatus::done:
//             {
//                 checkPresetsThread.startThread ();
//             }
//             break;
//         }
//     };
//     PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
//     unEditedPresetProperties.wrap (presetManagerProperties.getPreset ("unedited"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
//     presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);

    checkBanksThread.startThread ();
}

void BankListComponent::forEachBankDirectory (std::function<bool (juce::File presetFile, int index)> presetFileCallback)
{
#if 0
    jassert (presetFileCallback != nullptr);

    auto inPresetList { false };
    ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this, presetFileCallback, &inPresetList] (juce::ValueTree child)
    {
        if (FileProperties::isFileVT (child))
        {
            FileProperties fileProperties (child, FileProperties::WrapperType::client, FileProperties::EnableCallbacks::no);
            if (fileProperties.getType ()== DirectoryDataProperties::presetFile)
            {
                inPresetList = true;
                const auto fileToCheck { juce::File (fileProperties.getName ()) };
                const auto presetIndex { FileTypeHelpers::getPresetNumberFromName (fileToCheck) - 1 };
                if (presetIndex < 0 || presetIndex >= kMaxPresets)
                    return false;
                if (! presetFileCallback (fileToCheck, presetIndex))
                    return false;
            }
            else
            {
                // if the entry is not a preset file, but we were processing preset files, then we are done
                if (inPresetList)
                    return false;
            }
        }
        return true;
    });
#endif
}

void BankListComponent::checkBanks ()
{
    WatchdogTimer timer;
    timer.start (100000);

    FolderProperties rootFolder (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    currentFolder = juce::File (rootFolder.getName ());

    const auto showAll { showAllBanks.getToggleState () };

    // clear bank info list
    for (auto curBanknfoIndex { 0 }; curBanknfoIndex < bankInfoList.size (); ++curBanknfoIndex)
        bankInfoList[curBanknfoIndex] = { curBanknfoIndex + 1, false, "" };

    if (showAll)
        numBanks = kMaxBanks;
    else
        numBanks = 0;
    auto inBankList { false };
    ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this, &inBankList, showAll] (juce::ValueTree child)
    {
        if (FolderProperties::isFolderVT (child))
        {
            FolderProperties folderProperties (child, FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
            const auto folderName { juce::File (folderProperties.getName ()).getFileName () };
            const auto bankId { folderName.substring (5).getIntValue () };
            if (folderName.substring(0, 4) == "Bank" && bankId > 0 && bankId < 100)
            {
                inBankList = true;
                const auto fileToCheck { juce::File (folderProperties.getName ()) };

                auto infoTxtFile { fileToCheck.getChildFile ("info.txt") };
                auto bankName { juce::String () };
                // read bank name if file exists
                if (infoTxtFile.exists ())
                {
                    auto infoTxtInputStream { infoTxtFile.createInputStream () };
                    bankName = infoTxtInputStream->readNextLine ().substring(0, 11);
                }

                if (showAll)
                    bankInfoList [bankId - 1] = { bankId, true, bankName};
                else
                {
                    bankInfoList [numBanks] = { bankId, true, bankName};
                    ++numBanks;
                }
            }
            else
            {
                // if the entry is not a preset file, but we had started processing preset files, then we are done, because the files are sorted by type
                if (inBankList)
                    return false;
            }
        }
        return true; // keep looking
    });

    juce::MessageManager::callAsync ([this, newFolder = (currentFolder != previousFolder)] ()
    {
        bankListBox.updateContent ();
        if (newFolder)
        {
            bankListBox.scrollToEnsureRowIsOnscreen (0);
            loadFirstBank ();
        }
        bankListBox.repaint ();
    });
    previousFolder = currentFolder;

    //juce::Logger::outputDebugString ("PresetListComponent::checkPresets - elapsed time: " + juce::String (timer.getElapsedTime ()));
}

void BankListComponent::loadFirstBank ()
{
#if 0
    LogPresetList ("PresetListComponent::loadFirstPreset");
    bool presetLoaded { false };
    juce::File loadedPresetFile;
    forEachPresetFile ([this, &presetLoaded, &loadedPresetFile] (juce::File presetFile, int presetIndex)
    {
        if (auto [presetNumber, thisPresetExists, presetName] { presetInfoList [presetIndex] }; ! thisPresetExists)
            return true;

        presetListBox.selectRow (presetIndex, false, true);
        presetListBox.scrollToEnsureRowIsOnscreen (presetIndex);
        loadedPresetFile = presetFile;
        appProperties.addRecentlyUsedFile (loadedPresetFile.getFullPathName ());
        loadPreset (presetFile);
        presetLoaded = true;
        return false;
    });

    if (! presetLoaded)
    {
        presetListBox.selectRow (0, false, true);
        presetListBox.scrollToEnsureRowIsOnscreen (0);
        loadedPresetFile = getPresetFile (1);
        appProperties.addRecentlyUsedFile (loadedPresetFile.getFullPathName ());
        loadDefault (0);
    }
#endif
}

void BankListComponent::loadDefault (int row)
{
    jassertfalse;
#if 0
    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetProperties.getValueTree ());
    // set the ID, since the default that was just loaded always has Id 1
    presetProperties.setId (row + 1, false);
    PresetProperties::copyTreeProperties (presetProperties.getValueTree (), unEditedPresetProperties.getValueTree ());
#endif
}

void BankListComponent::loadBankDirectory (juce::File presetFile, juce::ValueTree presetPropertiesVT)
{
#if 0
    juce::StringArray fileContents;
    presetFile.readLines (fileContents);

    Assimil8orPreset assimil8orPreset;
    assimil8orPreset.parse (fileContents);

    // TODO - is this redundant, since assimil8orPreset.parse resets it's PresetProperties to defaults already
    // first set preset to defaults
    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetPropertiesVT);
    // then load new preset
    PresetProperties::copyTreeProperties (assimil8orPreset.getPresetVT (), presetPropertiesVT);

    //ValueTreeHelpers::dumpValueTreeContent (presetPropertiesVT, true, [this] (juce::String text) { juce::Logger::outputDebugString (text); });
#endif
}

void BankListComponent::loadBank (juce::File presetFile)
{
    editManager->loadBank (presetFile);
}

void BankListComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    auto toolRow { localBounds.removeFromTop (25) };
    showAllBanks.setBounds (toolRow.removeFromLeft (100));
    bankListBox.setBounds (localBounds);
}

int BankListComponent::getNumRows ()
{
    return numBanks;
}

void BankListComponent::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (row < numBanks)
    {
        juce::Colour textColor;
        juce::Colour rowColor;
        if (rowIsSelected)
        {
            lastSelectedBankIndex = row;
            rowColor = juce::Colours::darkslategrey;
            textColor = juce::Colours::yellow;
        }
        else
        {
            rowColor = juce::Colours::black;
            textColor = juce::Colours::whitesmoke;
        }
        auto [presetNumber, thisPresetExists, presetName] { bankInfoList [row] };
        if (thisPresetExists)
        {

        }
        else
        {
            presetName = "(bank)";
            textColor = textColor.withAlpha (0.5f);
        }
        g.setColour (rowColor);
        g.fillRect (width - 1, 0, 1, height);
        g.setColour (textColor);
        g.drawText ("  " + juce::String (presetNumber) + "-" + presetName, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
}

void BankListComponent::timerCallback ()
{
#if 0
    LogPresetList ("PresetListComponent::timerCallback - enter");
    if (! checkPresetsThread.isThreadRunning ())
    {
        LogPresetList ("PresetListComponent::timerCallback - starting thread, stopping timer");
        checkPresetsThread.start ();
        stopTimer ();
    }
    LogPresetList ("PresetListComponent::timerCallback - enter");
#endif
}

juce::String BankListComponent::getTooltipForRow (int row)
{
    return "Preset " + juce::String (row + 1);
}

void BankListComponent::copyBank (int presetNumber)
{
//    loadPresetFile (getPresetFile (presetNumber), copyBufferPresetProperties.getValueTree ());
}

void BankListComponent::pasteBank (int presetNumber)
{
#if 0
    auto doPaste = [this, presetNumber] ()
    {
        Assimil8orPreset assimil8orPreset;
        PresetProperties::copyTreeProperties (copyBufferPresetProperties.getValueTree (), assimil8orPreset.getPresetVT ());
        assimil8orPreset.write (getPresetFile (presetNumber));
        auto [lastSelectedPresetNumber, thisPresetExists, presetName] { presetInfoList [lastSelectedPresetIndex] };
        if (presetNumber == lastSelectedPresetNumber)
        {
            PresetProperties::copyTreeProperties (copyBufferPresetProperties.getValueTree (), unEditedPresetProperties.getValueTree ());
            unEditedPresetProperties.setId (presetNumber, false);
            PresetProperties::copyTreeProperties (copyBufferPresetProperties.getValueTree (), presetProperties.getValueTree ());
            presetProperties.setId (presetNumber, false);
        }
    };

    auto [thisPresetNumber, thisPresetExists, presetName] { presetInfoList [lastSelectedPresetIndex]};
    if (thisPresetExists)
    {
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "OVERWRITE PRESET", "Are you sure you want to overwrite '" + FileTypeHelpers::getPresetFileName (presetNumber) + "'", "YES", "NO", nullptr,
            juce::ModalCallbackFunction::create ([this, doPaste] (int option)
            {
                if (option == 0) // no
                    return;
                doPaste ();
            }));
    }
    else
    {
        doPaste ();
    }
#endif
}

void BankListComponent::deleteBank (int presetNumber)
{
#if 0
    juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "DELETE PRESET", "Are you sure you want to delete '" + FileTypeHelpers::getPresetFileName (presetNumber) + "'", "YES", "NO", nullptr,
        juce::ModalCallbackFunction::create ([this, presetNumber, presetFile = getPresetFile (presetNumber)] (int option)
        {
            if (option == 0) // no
                return;
            presetFile.deleteFile ();
            // TODO handle delete error
            auto [lastSelectedPresetNumber, thisPresetExists, presetName] { presetInfoList [lastSelectedPresetIndex] };
            if (presetNumber == lastSelectedPresetNumber)
            {
                auto defaultPreset { ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType) };
                PresetProperties::copyTreeProperties (defaultPreset, unEditedPresetProperties.getValueTree ());
                unEditedPresetProperties.setId (presetNumber, false);
                PresetProperties::copyTreeProperties (defaultPreset, presetProperties.getValueTree ());
                presetProperties.setId (presetNumber, false);
            }
        }));
#endif
}

juce::File BankListComponent::getBankDirectory (int bankNumber)
{
    jassert (bankNumber > 0 && bankNumber <= kMaxBanks);
    const auto bankDirectory { "Bank " + juce::String (bankNumber) };
    return currentFolder.getChildFile (bankDirectory);
}

void BankListComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (me.mods.isPopupMenu ())
    {
#if 0
        presetListBox.selectRow (lastSelectedPresetIndex, false, true);
        auto [presetNumber, thisPresetExists, presetName] { presetInfoList [row] };
        if (! thisPresetExists)
            presetName = "(preset)";

        auto* popupMenuLnF { new juce::LookAndFeel_V4 };
        popupMenuLnF->setColour (juce::PopupMenu::ColourIds::headerTextColourId, juce::Colours::white.withAlpha (0.3f));
        juce::PopupMenu pm;
        pm.setLookAndFeel (popupMenuLnF);
        pm.addSectionHeader (juce::String (presetNumber) + " - " + presetName);
        pm.addSeparator ();
        pm.addItem ("Copy", thisPresetExists, false, [this, presetNumber = presetNumber] () { copyPreset (presetNumber); });
        pm.addItem ("Paste", copyBufferPresetProperties.getName ().isNotEmpty (), false, [this, presetNumber = presetNumber] () { pastePreset (presetNumber); });
        pm.addItem ("Delete", thisPresetExists, false, [this, presetNumber = presetNumber] () { deletePreset (presetNumber); });
        pm.showMenuAsync ({}, [this, popupMenuLnF] (int) { delete popupMenuLnF; });
#endif
    }
    else
    {
        // don't reload the currently loaded preset
        if (row == lastSelectedBankIndex)
            return;

        auto completeSelection = [this, row] ()
        {
            auto [bankNumber, thisBankExists, presetName] { bankInfoList [row] };
            auto bankDirectory { getBankDirectory (bankNumber) };
            if (thisBankExists)
                loadBank (bankDirectory);
            else
                loadDefault (row);
            bankListBox.selectRow (row, false, true);
            bankListBox.scrollToEnsureRowIsOnscreen (row);
            // TODO - should this be done in EditManager::loadBank
            appProperties.addRecentlyUsedFile (bankDirectory.getFullPathName ());
        };

        if (overwriteBankOrCancel != nullptr)
        {
            bankListBox.selectRow (lastSelectedBankIndex, false, true);
            overwriteBankOrCancel (completeSelection, [this] () {});
        }
        else
        {
            completeSelection ();
        }
    }
}
