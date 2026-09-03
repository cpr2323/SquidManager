#include "FileViewComponent.h"
#include "../../../SystemServices.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

#define LOG_FILE_VIEW 0
#if LOG_FILE_VIEW
#define LogFileView(text) juce::Logger::outputDebugString (text);
#else
#define LogFileView(text) ;
#endif

const auto kDialogTextEditorName { "foldername" };

FileViewComponent::FileViewComponent ()
{
    setOpaque (true);
    openFolderButton.setButtonText ("Open");
    openFolderButton.setTooltip ("Navigate to a specific folder");
    openFolderButton.onClick = [this] () { openFolder (); };
    addAndMakeVisible (openFolderButton);
    newFolderButton.setButtonText ("New");
    newFolderButton.setTooltip ("Create a new folder");
    newFolderButton.onClick = [this] () { newFolder (); };
    addAndMakeVisible (newFolderButton);
    directoryContentsListBox.setColour (juce::ListBox::ColourIds::backgroundColourId, juce::Colours::black);
    addAndMakeVisible (directoryContentsListBox);
    showAllFiles.setToggleState (false, juce::NotificationType::dontSendNotification);
    showAllFiles.setButtonText ("Show All");
    showAllFiles.setTooltip ("Show all files, or show just Squid Salmple files");
    showAllFiles.onClick = [this] () { updateFromNewData (); };
    addAndMakeVisible (showAllFiles);
}

void FileViewComponent::init (juce::ValueTree rootPropertiesVT)
{
    LogFileView ("FileViewComponent::init");
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();

    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    // the file types are registered before the UI is built, so the registry is already published by now
    audioFileTypeId = directoryDataProperties.getFileTypeId (kAudioFileTypeName);
    directoryDataProperties.onRootScanComplete = [this] ()
    {
        LogFileView ("FileViewComponent/onRootScanComplete");
        isRootFolder = juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory () == juce::File (directoryDataProperties.getRootFolder ());
        updateFromNewData ();
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
//                 isRootFolder = juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory () == juce::File (directoryDataProperties.getRootFolder ());
//                 updateFromNewData ();
//             }
//             break;
//         }
//     };

    updateFromNewData ();
}

void FileViewComponent::updateFromNewData ()
{
    LogFileView ("FileViewComponent::updateFromNewData ()");
    // the directory ValueTree is shared, so it is only ever read on the message thread
    jassert (juce::MessageManager::existsAndIsCurrentThread ());
    buildQuickLookupList ();
    directoryContentsListBox.updateContent ();
    directoryContentsListBox.repaint ();
}

// entries carry the type id DirectoryValueTree assigned them, so an audio file is one whose id matches
// the one the audio type was registered under. folders are identified structurally, and carry no type
bool FileViewComponent::isAudioFile (juce::ValueTree directoryEntryVT)
{
    return FileProperties::isFileVT (directoryEntryVT) &&
           static_cast<int> (directoryEntryVT.getProperty (FileProperties::TypePropertyId)) == audioFileTypeId;
}

void FileViewComponent::buildQuickLookupList ()
{
    jassert (juce::MessageManager::existsAndIsCurrentThread ());
    directoryListQuickLookupList.clear ();
    ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this] (juce::ValueTree child)
    {
        if (showAllFiles.getToggleState () || FolderProperties::isFolderVT (child) || isAudioFile (child))
            directoryListQuickLookupList.emplace_back (child);
        return true;
    });
}

void FileViewComponent::openFolder ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the folder to scan as an Squid Salmple USB drive...",
                                               appProperties.getMostRecentFolder (), ""));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [this] (const juce::FileChooser& fc) mutable
    {
        if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            appProperties.setMostRecentFolder (fc.getURLResults () [0].getLocalFile ().getFullPathName ());
    }, nullptr);
}

void FileViewComponent::newFolder ()
{
    newAlertWindow = std::make_unique<juce::AlertWindow> ("NEW FOLDER", "Enter the name for new folder", juce::MessageBoxIconType::NoIcon);
    newAlertWindow->addTextEditor (kDialogTextEditorName, {}, {});
    newAlertWindow->addButton ("CREATE", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
    newAlertWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
    auto* textEdtitor { newAlertWindow->getTextEditor (kDialogTextEditorName) };
    auto* createButton { newAlertWindow->getButton ("CREATE") };
    auto* cancelButton { newAlertWindow->getButton ("CANCEL") };
    textEdtitor->setExplicitFocusOrder (1);
    createButton->setExplicitFocusOrder (2);
    cancelButton->setExplicitFocusOrder (3);
    newAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this] (int option)
    {
        newAlertWindow->exitModalState (option);
        newAlertWindow->setVisible (false);
        if (option == 1) // ok
        {
            auto newFolderName { newAlertWindow->getTextEditorContents (kDialogTextEditorName) };
            auto newFolder { juce::File (appProperties.getMostRecentFolder ()).getChildFile (newFolderName) };
            newFolder.createDirectory ();
            // TODO handle error
        }
        newAlertWindow.reset ();
    }));
}

int FileViewComponent::getNumRows ()
{
    return static_cast<int> (directoryListQuickLookupList.size () + (isRootFolder ? 0 : 1));
}

juce::ValueTree FileViewComponent::getDirectoryEntryVT (int row)
{
    const auto quickLookupIndex { row - (isRootFolder ? 0 : 1) };
    return directoryListQuickLookupList [quickLookupIndex];
}

void FileViewComponent::paintListBoxItem (int row, juce::Graphics& g, int width, int height, [[maybe_unused]] bool rowIsSelected)
{
    if (row >= getNumRows ())
        return;

    if (rowIsSelected)
        lastSelectedRow = row;

    juce::Colour textColor { juce::Colours::whitesmoke };
    juce::String fileListItem;
    if (! isRootFolder && row == 0)
    {
        fileListItem = " >  ..";
    }
    else
    {
        const auto directoryEntryVT { getDirectoryEntryVT (row) };
        juce::String filePrefix;
        if (FolderProperties::isFolderVT (directoryEntryVT))
        {
            filePrefix = "> ";
        }
        else if (isAudioFile (directoryEntryVT))
        {
            filePrefix = "-  ";
            textColor = juce::Colours::forestgreen;
        }
        else
        {
            filePrefix = "   ";
            textColor = textColor.darker (0.4f);
        }
        auto file { juce::File (directoryEntryVT.getProperty ("name").toString ()) };
        fileListItem = " " + filePrefix + file.getFileName ();
    }

    g.setColour (textColor);
    g.drawText (fileListItem, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
}

juce::String FileViewComponent::getTooltipForRow (int row)
{
    if (row >= getNumRows ())
        return {};

    if (! isRootFolder && row == 0)
        return juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory ().getFullPathName ();
    else
    {
        const auto directoryEntryVT { getDirectoryEntryVT (row) };

        juce::String toolTip { juce::File (directoryEntryVT.getProperty ("name").toString ()).getFileName () };
        if (isAudioFile (directoryEntryVT))
        {
            const auto sampleRate { static_cast<int> (directoryEntryVT.getProperty ("sampleRate")) };
            if (auto errorString { directoryEntryVT.getProperty ("error").toString () }; errorString != "")
                toolTip += juce::String ("\r") + "Error: " + errorString;
            toolTip += juce::String ("\r") + "DataType: " + directoryEntryVT.getProperty ("dataType").toString ();
            toolTip += juce::String ("\r") + "BitDepth: " + juce::String (static_cast<int> (directoryEntryVT.getProperty ("bitDepth")));
            toolTip += juce::String ("\r") + "Channels: " + juce::String (static_cast<int> (directoryEntryVT.getProperty ("numChannels")));
            toolTip += juce::String ("\r") + "SampleRate: " + juce::String (sampleRate);
            toolTip += juce::String ("\r") + "Length: " + juce::String (static_cast<double> (static_cast<juce::int64> (directoryEntryVT.getProperty ("lengthSamples"))) / sampleRate, 2);
        }
        return toolTip;
    }
}

void FileViewComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (row >= getNumRows ())
        return;

    if (! isRootFolder && row != 0)
    {
        const auto directoryEntryVT { getDirectoryEntryVT (row) };
        if (! FolderProperties::isFolderVT (directoryEntryVT))
            return;
    }

    if (me.mods.isPopupMenu ())
    {
        if (! isRootFolder && row == 0)
            return;
        const auto directoryEntryVT { getDirectoryEntryVT (row) };
        auto folder { juce::File (directoryEntryVT.getProperty ("name").toString ()) };
        auto* popupMenuLnF { new juce::LookAndFeel_V4 };
        popupMenuLnF->setColour (juce::PopupMenu::ColourIds::headerTextColourId, juce::Colours::white.withAlpha (0.3f));
        juce::PopupMenu pm;
        pm.setLookAndFeel (popupMenuLnF);
        pm.addSectionHeader (folder.getFileName ());
        pm.addSeparator ();
        pm.addItem ("Rename", true, false, [this, folder] ()
        {
            renameAlertWindow = std::make_unique<juce::AlertWindow> ("RENAME FOLDER", "Enter the new name for '" + folder.getFileName ()  + "'", juce::MessageBoxIconType::NoIcon);
            renameAlertWindow->addTextEditor (kDialogTextEditorName, folder.getFileName (), {});
            renameAlertWindow->addButton ("RENAME", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
            renameAlertWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
            auto* textEdtitor { renameAlertWindow->getTextEditor (kDialogTextEditorName) };
            auto* createButton { renameAlertWindow->getButton ("RENAME") };
            auto* cancelButton { renameAlertWindow->getButton ("CANCEL") };
            textEdtitor->setExplicitFocusOrder (1);
            createButton->setExplicitFocusOrder (2);
            cancelButton->setExplicitFocusOrder (3);
            renameAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this, folder] (int option)
            {
                renameAlertWindow->exitModalState (option);
                renameAlertWindow->setVisible (false);
                if (option == 1) // ok
                {
                    auto newFolderName { renameAlertWindow->getTextEditorContents (kDialogTextEditorName) };
                    folder.moveFileTo (folder.getParentDirectory ().getChildFile (newFolderName));
                    // TODO handle error
                }
                renameAlertWindow.reset ();
            }));
        });
        pm.addItem ("Delete", true, false, [this, folder] ()
        {
            juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "DELETE FOLDER",
            "Are you sure you want to delete the folder '" + folder.getFileName () + "'", "YES", "NO", nullptr,
            juce::ModalCallbackFunction::create ([this, folder] (int option)
            {
                if (option == 0) // no
                    return;
                if (! folder.deleteFile ())
                {
                    // TODO handle delete error
                }
            }));
        });
        pm.showMenuAsync ({}, [this, popupMenuLnF] (int) { delete popupMenuLnF; });
    }
    else
    {
        auto completeSelection = [this, row] ()
        {
            editManager->cleanUpTempFiles (appProperties.getRecentlyUsedFile (0));
            if (! isRootFolder && row == 0)
            {
                appProperties.setMostRecentFolder (juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory ().getFullPathName ());
            }
            else
            {
                const auto directoryEntryVT { getDirectoryEntryVT (row) };
                auto folder { juce::File (directoryEntryVT.getProperty ("name").toString ()) };
                appProperties.setMostRecentFolder (folder.getFullPathName ());
            }
        };

        if (overwriteBankOrCancel != nullptr)
        {
            auto cancelSelection = [this] ()
            {
                directoryContentsListBox.selectRow (lastSelectedRow, false, true);
            };

            overwriteBankOrCancel (completeSelection, cancelSelection);
        }
        else
        {
            completeSelection ();
        }
    }
}

void FileViewComponent::listBoxItemDoubleClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (row >= getNumRows () || (! isRootFolder && row == 0))
        return;

    const auto directoryEntryVT { getDirectoryEntryVT (row) };
    if (isAudioFile (directoryEntryVT))
    {
        if (onAudioFileSelected != nullptr)
            onAudioFileSelected (juce::File (directoryEntryVT.getProperty ("name").toString ()));
    }
}

void FileViewComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (3, 3);
    auto toolRow { localBounds.removeFromTop (25) };
    openFolderButton.setBounds (toolRow.removeFromLeft (50));
    toolRow.removeFromLeft (5);
    newFolderButton.setBounds (toolRow.removeFromLeft (50));
    showAllFiles.setBounds (toolRow);

    localBounds.removeFromTop (3);
    directoryContentsListBox.setBounds (localBounds);
}

void FileViewComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

