#include "BankListComponent.h"
#include "../../../SquidSalmple/Bank/BankManagerProperties.h"
#include "../../../SystemServices.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/WatchDogTimer.h"

#define LOG_BANK_LIST 0
#if LOG_BANK_LIST
#define LogBankList(text) DebugLog ("BankListComponent", text);
#else
#define LogBankList(text) ;
#endif

BankListComponent::BankListComponent ()
{
    setOpaque (true);
    showAllBanks.setToggleState (true, juce::NotificationType::dontSendNotification);
    showAllBanks.setButtonText ("Show All");
    showAllBanks.setTooltip ("Show all Banks, Show only existing Banks");
    showAllBanks.onClick = [this] () { checkBanksThread.start (); };
    addAndMakeVisible (showAllBanks);
    bankListBox.setColour (juce::ListBox::ColourIds::backgroundColourId, juce::Colours::black);
    addAndMakeVisible (bankListBox);

    checkBanksThread.onThreadLoop = [this] ()
    {
        checkBanks ();
        return false;
    };
}

void BankListComponent::init (juce::ValueTree rootPropertiesVT)
{
    LogBankList ("init");
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();

    bankListProperties.wrap (runtimeRootProperties.getValueTree (), BankListProperties::WrapperType::owner, BankListProperties::EnableCallbacks::no);

    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    directoryDataProperties.onRootScanComplete = [this] ()
    {
        LogBankList ("init - directoryDataProperties.onRootScanComplete");
        // clear list
        juce::MessageManager::callAsync ([this] ()
        {
            if (! checkBanksThread.isThreadRunning ())
            {
                checkForFolderChange ();
                LogBankList ("init - directoryDataProperties.onRootScanComplete - starting thread");
                checkBanksThread.startThread ();
            }
            else
            {
                LogBankList ("init - directoryDataProperties.onRootScanComplete - starting timer");
                startTimer (1);
            }
        });
    };
    BankManagerProperties bankManagerProperties (runtimeRootProperties.getValueTree (), BankManagerProperties::WrapperType::owner, BankManagerProperties::EnableCallbacks::no);
    squidBankProperties.wrap (bankManagerProperties.getBank ("edit"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    uneditedSquidBankProperties.wrap (bankManagerProperties.getBank ("unedited"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    uneditedSquidBankProperties.onNameChange = [this] (juce::String newName)
    {
        if (lastSelectedBankIndex < bankInfoList.size ())
        {
            auto [bankNumber, thisBankExists, bankName] { bankInfoList [lastSelectedBankIndex] };
            bankInfoList [lastSelectedBankIndex] = { bankNumber, thisBankExists, newName };
            bankListBox.repaintRow (lastSelectedBankIndex);
        }
    };
    checkBanksThread.startThread ();
}

void BankListComponent::checkForFolderChange ()
{
    LogBankList ("checkForFolderChange");
    FolderProperties rootFolder (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    auto newFolder = juce::File (rootFolder.getName ());
    const auto isNewFolder { newFolder != currentFolder };
    LogBankList (isNewFolder ? "new folder " + newFolder.getFileName () : "no folder change");
    if (isNewFolder)
    {
        LogBankList ("clearing bank list");
        numBanks = 0;
        bankListBox.updateContent ();
        bankListBox.scrollToEnsureRowIsOnscreen (0);
        bankListBox.repaint ();
    }
}

void BankListComponent::forEachBankDirectory (std::function<bool (juce::File bankDirectory, int index)> bankDirectoryCallback)
{
    jassert (bankDirectoryCallback != nullptr);

    ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this, bankDirectoryCallback] (juce::ValueTree child)
    {
        if (FolderProperties::isFolderVT (child))
        {
            FolderProperties folderProperties (child, FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
            auto directoryName { juce::File (folderProperties.getName ()).getFileName () };
            auto bankId { 0 };
            if (directoryName.substring (0, 5) == "Bank ")
            {
                bankId = directoryName.substring (5).getIntValue ();
                if (bankId < 1 || bankId > 99)
                    return false;
                if (! bankDirectoryCallback (folderProperties.getName (), bankId - 1))
                    return false;
            }
        }
        return true;
    });
}

void BankListComponent::checkBanks ()
{
    LogBankList ("BankListComponent::checkBanks - start");
    WatchdogTimer timer;
    timer.start (100000);

    FolderProperties rootFolder (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    currentFolder = juce::File (rootFolder.getName ());

    const auto showAll { showAllBanks.getToggleState () };

    // clear bank info list
     for (auto curBankInfoIndex { 0 }; curBankInfoIndex < bankInfoList.size (); ++curBankInfoIndex)
         bankInfoList [curBankInfoIndex] = { curBankInfoIndex + 1, false, "" };

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
            if (folderName.substring (0, 5) == "Bank " && bankId > 0 && bankId < 100)
            {
                bankListProperties.setStatus ("Scanning Bank Folder: " + folderName, false);
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
                    bankInfoList [bankId - 1] = { bankId, true, bankName };
                else
                {
                    bankInfoList [numBanks] = { bankId, true, bankName };
                    ++numBanks;
                }
            }
            else
            {
                // if the entry is not a bank file, but we had started processing bank files, then we are done, because the files are sorted by type
                if (inBankList)
                    return false;
            }
        }
        return true; // keep looking
    });
    bankListProperties.setStatus ("", false);

    const auto isNewFolder { currentFolder != previousFolder };
    LogBankList (isNewFolder ? "new folder " + currentFolder.getFileName () : "no folder change");
    juce::MessageManager::callAsync ([this, isNewFolder] ()
    {
        bankListBox.updateContent ();
        if (isNewFolder)
        {
            bankListBox.scrollToEnsureRowIsOnscreen (0);
            loadFirstBank ();
        }
        bankListBox.repaint ();
    });
    previousFolder = currentFolder;

    LogBankList("BankListComponent::checkBanks - elapsed time: " + juce::String (timer.getElapsedTime ()));
}

void BankListComponent::loadFirstBank ()
{
    LogBankList ("loadFirstBank");
    bool bankLoaded { false };
    juce::File loadedBankDirectory;
    forEachBankDirectory ([this, &bankLoaded, &loadedBankDirectory] (juce::File bankDirectory, int bankIndex)
    {
        LogBankList ("checking " + bankDirectory.getFileName ());
        if (auto [bankNumber, thisBankExists, bankName] { bankInfoList [bankIndex] }; ! thisBankExists)
            return true;

        LogBankList ("loading " + bankDirectory.getFileName ());
        bankListBox.selectRow (bankIndex, false, true);
        bankListBox.scrollToEnsureRowIsOnscreen (bankIndex);
        loadedBankDirectory = bankDirectory;
        appProperties.addRecentlyUsedFile (loadedBankDirectory.getFullPathName ());
        loadBank (bankDirectory);
        bankLoaded = true;
        return false;
    });

    if (! bankLoaded)
    {
        LogBankList ("no bank found. loading default");
        bankListBox.selectRow (0, false, true);
        bankListBox.scrollToEnsureRowIsOnscreen (0);
        loadedBankDirectory= getBankDirectory (1);
        appProperties.addRecentlyUsedFile (loadedBankDirectory.getFullPathName ());
        loadDefault (0);
    }
}

void BankListComponent::loadDefault (int row)
{
    editManager->loadBankDefaults (static_cast<uint8_t> (row));
}

void BankListComponent::loadBank (juce::File bankDirectory)
{
    editManager->loadBank (bankDirectory);
}

void BankListComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    auto toolRow { localBounds.removeFromTop (25) };
    showAllBanks.setBounds (toolRow.removeFromLeft (100));
    bankListBox.setBounds (localBounds);
}

void BankListComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
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
            rowColor = juce::Colours::black;
            textColor = juce::Colours::yellow;
        }
        else
        {
            rowColor = juce::Colours::black;
            textColor = juce::Colours::whitesmoke;
        }
        auto [bankNumber, thisBankExists, bankName] { bankInfoList [row] };
        if (thisBankExists)
        {
            if (bankName.isEmpty ())
                bankName = "(no name)";
        }
        else
        {
            bankName = "(empty)";
            textColor = textColor.withAlpha (0.5f);
        }
        g.setColour (textColor);
        g.drawText ("  " + juce::String (bankNumber) + "-" + bankName, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
}

void BankListComponent::timerCallback ()
{
    LogBankList ("timerCallback - enter");
    if (! checkBanksThread.isThreadRunning ())
    {
        checkForFolderChange ();
        LogBankList ("timerCallback - starting thread, stopping timer");
        checkBanksThread.start ();
        stopTimer ();
    }
    LogBankList ("timerCallback - enter");
}

juce::String BankListComponent::getTooltipForRow (int row)
{
    return "Bank " + juce::String (row + 1);
}

void BankListComponent::copyBank (int bankNumber)
{
    copyDirectory = getBankDirectory (bankNumber);
}

void BankListComponent::pasteBank (int bankNumber)
{
    auto doPaste = [this, bankNumber] ()
    {
        const auto destinationBankDirectory { getBankDirectory (bankNumber) };
        if (! destinationBankDirectory.exists ())
        {
            destinationBankDirectory.createDirectory ();
            // TODO - handle error
        }
        if (auto infoTextFile { copyDirectory.getChildFile ("info.txt") }; infoTextFile.exists ())
        {
            infoTextFile.copyFileTo (destinationBankDirectory.getChildFile ("info.txt"));
            // TODO - handle error
        }
        for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
        {
            const auto channelFolder { juce::String (channelIndex + 1) };
            auto sourceChannelFolder { copyDirectory.getChildFile (channelFolder) };
            auto destinationChannelFolder { destinationBankDirectory.getChildFile (channelFolder) };
            sourceChannelFolder.copyDirectoryTo (destinationChannelFolder);
            // TODO - handle error
        }
        auto [lastSelectedBankNumber, thisBankExists, bankName] { bankInfoList [lastSelectedBankIndex] };
        if (bankNumber == lastSelectedBankNumber)
        {
            editManager->loadBank (destinationBankDirectory);
        }
    };

    auto [thisBankNumber, thisBankExists, bankName] { bankInfoList [bankNumber - 1] };
    if (thisBankExists)
    {
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "OVERWRITE BANK", "Are you sure you want to overwrite '" + bankName + "'", "YES", "NO", nullptr,
            juce::ModalCallbackFunction::create ([this, doPaste, bankNumber] (int option)
            {
                if (option == 0) // no
                    return;
                // delete destination folder
                const auto destinationBankDirectory { getBankDirectory (bankNumber) };
                destinationBankDirectory.deleteRecursively (false);
                doPaste ();
            }));
    }
    else
    {
        doPaste ();
    }
}

void BankListComponent::deleteBank (int bankNumber)
{
    auto [_, __, bankName] { bankInfoList [bankNumber - 1] };
    const auto bankDirectory { getBankDirectory (bankNumber) };

    juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "DELETE BANK", "Are you sure you want to delete '" + bankName + "'", "YES", "NO", nullptr,
        juce::ModalCallbackFunction::create ([this, bankNumber, bankDirectory] (int option)
        {
            if (option == 0) // no
                return;
            // TODO - delete contents of folder and delete folder
            bankDirectory.deleteRecursively (false);
            // TODO handle delete error
            auto [lastSelectedBankNumber, thisBankExists, bankName] { bankInfoList [lastSelectedBankIndex] };
            if (bankNumber == lastSelectedBankNumber)
            {
                // clear editor
                editManager->setBankDefaults ();
                uneditedSquidBankProperties.copyFrom (squidBankProperties.getValueTree ());
            }
        }));
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
        bankListBox.selectRow (lastSelectedBankIndex, false, true);
        auto [bankNumber, thisBankExists, bankName] { bankInfoList [row] };
        if (! thisBankExists)
            bankName = "(empty)";

        auto* popupMenuLnF { new juce::LookAndFeel_V4 };
        popupMenuLnF->setColour (juce::PopupMenu::ColourIds::headerTextColourId, juce::Colours::white.withAlpha (0.3f));
        juce::PopupMenu pm;
        pm.setLookAndFeel (popupMenuLnF);
        pm.addSectionHeader (juce::String (bankNumber) + " - " + bankName);
        pm.addSeparator ();
        pm.addItem ("Copy", thisBankExists, false, [this, bankNumber = bankNumber] () { copyBank (bankNumber); });
        pm.addItem ("Paste", copyDirectory != juce::File (), false, [this, bankNumber = bankNumber] () { pasteBank (bankNumber); });
        pm.addItem ("Delete", thisBankExists, false, [this, bankNumber = bankNumber] () { deleteBank (bankNumber); });
        pm.showMenuAsync ({}, [this, popupMenuLnF] (int) { delete popupMenuLnF; });
    }
    else
    {
        // don't reload the currently loaded bank
        if (row == lastSelectedBankIndex)
            return;

        lastSelectedBankIndex = row;
        auto completeSelection = [this, row] ()
        {
            editManager->cleanUpTempFiles (appProperties.getRecentlyUsedFile (0));

            auto [bankNumber, thisBankExists, bankName] { bankInfoList [row] };
            auto bankDirectory { getBankDirectory (bankNumber) };
            appProperties.addRecentlyUsedFile (bankDirectory.getFullPathName ());
            if (! thisBankExists)
                bankDirectory.createDirectory ();
            loadBank (bankDirectory);
            bankListBox.selectRow (row, false, true);
            bankListBox.scrollToEnsureRowIsOnscreen (row);
            // TODO - should this be done in EditManager::loadBank
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
