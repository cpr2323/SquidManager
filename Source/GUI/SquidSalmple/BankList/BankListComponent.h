#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../SquidSalmple/EditManager/EditManager.h"
#include "../../../Utility/DirectoryDataProperties.h"
#include "../../../Utility/LambdaThread.h"

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
    EditManager* editManager { nullptr };
//     PresetProperties presetProperties;
//     PresetProperties unEditedPresetProperties;
//     PresetProperties copyBufferPresetProperties;

    juce::ToggleButton showAllBanks { "Show All" };
    juce::ListBox bankListBox { {}, this };
    std::array<std::tuple <int, bool, juce::String>, kMaxBanks> bankInfoList;
    int numBanks { kMaxBanks };
    juce::File currentFolder;
    juce::File previousFolder;
    int lastSelectedBankIndex { -1 };
    LambdaThread checkBanksThread { "CheckBanksThread", 100 };

    void copyBank (int bankNumber);
    void checkBanks ();
    void deleteBank (int bankNumber);
    juce::File getBankDirectory (int bankNumber);
    void forEachBankDirectory (std::function<bool (juce::File bankDirectory, int index)> bankDirectoryCallback);
    void loadBankDirectory (juce::File bankDirectory, juce::ValueTree vt);
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
