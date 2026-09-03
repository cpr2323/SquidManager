#pragma once

#include <JuceHeader.h>
#include "BankListProperties.h"
#include "../../../AppProperties.h"
#include "../../../SquidSalmple/EditManager/EditManager.h"
#include "oolib/Core/LambdaThread.h"
#include "oolib/Directory/DirectoryDataProperties.h"

const auto kMaxBanks { 99 };
class BankListComponent : public juce::Component,
                            private juce::ListBoxModel,
                            private juce::Timer
{
public:
    BankListComponent ();
    ~BankListComponent () = default;
    void init (juce::ValueTree rootPropertiesVT);

    std::function<void (std::function<void ()>, std::function<void ()>)> overwriteBankOrCancel;

private:
    AppProperties appProperties;
    DirectoryDataProperties directoryDataProperties;
    BankListProperties bankListProperties;
    SquidBankProperties squidBankProperties;
    SquidBankProperties uneditedSquidBankProperties;
    EditManager* editManager { nullptr };
    juce::File copyDirectory;

    juce::ToggleButton showAllBanks { "Show All" };
    juce::ListBox bankListBox { {}, this };
    std::array<std::tuple <int, bool, juce::String>, kMaxBanks> bankInfoList;
    int numBanks { kMaxBanks };
    juce::File currentFolder;
    // set when a folder change means the first bank should be selected and loaded, cleared once
    // that has happened. message thread only
    bool firstBankLoadPending { true };
    int lastSelectedBankIndex { -1 };
    LambdaThread checkBanksThread { "CheckBanksThread", 100 };

    // one bank folder found in the directory ValueTree. the tree is walked on the message thread to
    // produce these, so that checkBanksThread only ever works with plain files
    struct BankDirectoryEntry
    {
        int bankId {};
        juce::File directory;
    };
    juce::CriticalSection bankDirectorySnapshotCS;
    std::vector<BankDirectoryEntry> bankDirectorySnapshot;
    juce::File snapshotRootFolder;
    bool snapshotShowAllBanks { true };

    void copyBank (int bankNumber);
    void checkBanks ();
    void checkForFolderChange ();
    void snapshotBankDirectories ();
    void startCheckBanksThread ();
    void sendStatusUpdate (juce::String status);
    void deleteBank (int bankNumber);
    juce::File getBankDirectory (int bankNumber);
    void forEachBankDirectory (std::function<bool (juce::File bankDirectory, int index)> bankDirectoryCallback);
    void loadDefault (int row);
    void loadFirstBank ();
    void loadBank (juce::File bankDirectory);
    void pasteBank (int bankNumber);

    void resized () override;
    void paint (juce::Graphics& g) override;
    int getNumRows () override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void timerCallback () override;
};
