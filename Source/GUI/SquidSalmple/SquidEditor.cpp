#include "SquidEditor.h"
#include "../../SquidSalmple/SquidMetaDataReader.h"
#include "../../SquidSalmple/SquidSalmpleDefs.h"
#include "../../Utility/PersistentRootProperties.h"


const auto kParameterLineHeight { 20 };
const auto kInterControlYOffset { 2 };
const auto kInitialYOffset { 5 };

const auto kScaleMax { 65535. };
const auto kScaleStep { kScaleMax / 100 };

SquidEditorComponent::SquidEditorComponent ()
{
    setOpaque (true);
    loadButton.setButtonText ("LOAD");
    loadButton.onClick = [this] ()
    {
        fileChooser.reset (new juce::FileChooser ("Please select the file to load...", {}, ""));
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            {
                auto wavFileToLoad { fc.getURLResults () [0].getLocalFile () };
                // TODO - should this entire 'load from file' code be moved outside of here?
                appProperties.addRecentlyUsedFile (wavFileToLoad.getFullPathName ());

                // TODO - check for import errors and handle accordingly
                SquidMetaDataReader squidMetaDataReader;
                SquidMetaDataProperties loadedSquidMetaDataProperties { squidMetaDataReader.read (wavFileToLoad),
                                                                        SquidMetaDataProperties::WrapperType::owner,
                                                                        SquidMetaDataProperties::EnableCallbacks::no };

                squidMetaDataProperties.copyFrom (loadedSquidMetaDataProperties.getValueTree ());
                //initCueSetTabs ();
                // TODO - is this redundant, since there should be a callback from squidMetaDataProperties.copyFrom when the curCueSet property is updated
                //setCurCue (squidMetaDataProperties.getCurCueSet ());
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
    squidMetaDataProperties.wrap (runtimeRootProperties.getValueTree (), SquidMetaDataProperties::WrapperType::client, SquidMetaDataProperties::EnableCallbacks::yes);
    for (auto channelIndex {0}; channelIndex < 8; ++channelIndex)
        channelEditorComponents [channelIndex].init (rootPropertiesVT);
}

void SquidEditorComponent::timerCallback ()
{
    // check if data has changed
}

void SquidEditorComponent::resized ()
{
    auto localBounds { getLocalBounds() };

    loadButton.setBounds (5, 5, 80, kParameterLineHeight);
    const auto channelSectionY { loadButton.getBottom () + 3 };
    const auto kWidthOfWaveformEditor { 962 };
    channelTabs.setBounds (3, channelSectionY, kWidthOfWaveformEditor + 30, getHeight() - channelSectionY - 5);
}

void SquidEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}
