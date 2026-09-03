#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../SquidSalmple/EditManager/EditManager.h"
#include "oolib/Directory/DirectoryDataProperties.h"

class FileViewComponent : public juce::Component,
                          private juce::ListBoxModel
{
public:
    FileViewComponent ();
    ~FileViewComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

    std::function<void (juce::File audioFile)> onAudioFileSelected;
    std::function<void (std::function<void ()>, std::function<void ()>)> overwriteBankOrCancel;

private:
    AppProperties appProperties;
    DirectoryDataProperties directoryDataProperties;
    EditManager* editManager { nullptr };
    // the id DirectoryValueTree handed out for the audio file type, resolved in init once the scanner
    // has published its registry. -1 until then, which no entry will ever carry
    int audioFileTypeId { -1 };

    // built and read only on the message thread, so it needs no lock or double buffering
    std::vector<juce::ValueTree> directoryListQuickLookupList;

    juce::TextButton openFolderButton;
    juce::TextButton newFolderButton;
    juce::ToggleButton showAllFiles { "Show All" };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ListBox directoryContentsListBox { {}, this };
    juce::CriticalSection queuedFolderLock;
    juce::File queuedFolderToScan;
    bool isRootFolder { false };
    int lastSelectedRow { -1 };
    std::unique_ptr<juce::AlertWindow> renameAlertWindow;
    std::unique_ptr<juce::AlertWindow> newAlertWindow;

    void buildQuickLookupList ();
    juce::ValueTree getDirectoryEntryVT (int row);
    bool isAudioFile (juce::ValueTree directoryEntryVT);
    void newFolder ();
    void openFolder ();
    void updateFromNewData ();

    void resized () override;
    void paint (juce::Graphics& g) override;
    int getNumRows () override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
};
    