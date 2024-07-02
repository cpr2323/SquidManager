#include "MainComponent.h"
#include "../Utility/PersistentRootProperties.h"
#include "../Utility/RuntimeRootProperties.h"

const auto toolWindowHeight { 30 };

MainComponent::MainComponent (juce::ValueTree rootPropertiesVT)
{
    setSize (1117, 609);

    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    addAndMakeVisible (squidEditorComponent);
    squidEditorComponent.init (rootPropertiesVT);
    fileViewComponent.init (rootPropertiesVT);
    bankListComponent.init (rootPropertiesVT);
    bottomStatusWindow.init (rootPropertiesVT);
    currentFolderComponent.init (rootPropertiesVT);

    bankListEditorSplitter.setComponents (&bankListComponent, &squidEditorComponent);
    bankListEditorSplitter.setHorizontalSplit (false);

    folderBrowserEditorSplitter.setComponents (&fileViewComponent, &bankListEditorSplitter);
    folderBrowserEditorSplitter.setHorizontalSplit (false);

    bankListEditorSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };
    folderBrowserEditorSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };

    restoreLayout ();

    addAndMakeVisible (currentFolderComponent);
    addAndMakeVisible (folderBrowserEditorSplitter);
    addAndMakeVisible (bottomStatusWindow);
}

void MainComponent::restoreLayout ()
{
    // TODO - I would like to abstract this so it is easier to customize what gets saved per app
    //        something like key/value pairs
    const auto [pane1Size, pane2Size, pane3Size] {guiProperties.getPaneSizes ()};
    bankListEditorSplitter.setSplitOffset (pane1Size);
    folderBrowserEditorSplitter.setSplitOffset (pane2Size);
}

void MainComponent::saveLayoutChanges ()
{
    const auto splitter1Size { bankListEditorSplitter.getSplitOffset () };
    const auto splitter2Size { folderBrowserEditorSplitter.getSplitOffset () };
    guiProperties.setPaneSizes (splitter1Size, splitter2Size, 0, false);
}

void MainComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.setColour (juce::Colours::red);
}

void MainComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    currentFolderComponent.setBounds (localBounds.removeFromTop (30));
    bottomStatusWindow.setBounds (localBounds.removeFromBottom (toolWindowHeight));
    localBounds.reduce (3, 3);
    folderBrowserEditorSplitter.setBounds (localBounds);
}
