#include "MainComponent.h"
#include "Theme/SquidColourIds.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

const auto toolWindowHeight { 30 };

MainComponent::MainComponent (juce::ValueTree rootPropertiesVT)
{
    setSize (1117, 609);
    rootProperties = rootPropertiesVT;

    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::yes);
    guiProperties.onShowSettingsDialog = [this] () { showSettingsDialog (); };
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    addAndMakeVisible (squidEditorComponent);
    squidEditorComponent.init (rootPropertiesVT);
    fileViewComponent.init (rootPropertiesVT);
    bankListComponent.init (rootPropertiesVT);
    // NOTE: bottomStatusWindow uses the BankListProperties, so it has to be initialised after BankListComponent.
    //       I dislike these kinds of requirements, so maybe figure out a different way at some point
    bottomStatusWindow.init (rootPropertiesVT);
    currentFolderComponent.init (rootPropertiesVT);

    fileViewComponent.overwriteBankOrCancel = [this] (std::function<void ()> overwriteFunction, std::function<void ()> cancelFunction)
    {
        squidEditorComponent.bankLoseEditWarning ("Changing Bank Folder", overwriteFunction, cancelFunction);
    };
    bankListComponent.overwriteBankOrCancel = [this] (std::function<void ()> overwriteFunction, std::function<void ()> cancelFunction)
    {
        squidEditorComponent.bankLoseEditWarning ("Loading New Bank", overwriteFunction, cancelFunction);
    };

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

void MainComponent::showSettingsDialog ()
{
    juce::DialogWindow::LaunchOptions options;
    options.escapeKeyTriggersCloseButton = true;
    options.dialogBackgroundColour = findColour (SquidColours::dialogBackground);
    options.dialogTitle = "SETTINGS";
    options.resizable = false;
    auto* settingsComponent { new SettingsDialogComponent () };
    settingsComponent->init (rootProperties);
    // sized to the largest page, measured after the pages are built
    settingsComponent->setBounds (settingsComponent->getPreferredBounds ());
    options.content.setOwned (settingsComponent);
    options.launchAsync ();
}

void MainComponent::restoreLayout ()
{
    // TODO - I would like to abstract this so it is easier to customize what gets saved per app
    //        something like key/value pairs
    const auto [pane1Size, pane2Size, pane3Size] { guiProperties.getPaneSizes () };
    bankListEditorSplitter.setSplitOffset (pane1Size);
    folderBrowserEditorSplitter.setSplitOffset (pane2Size);
}

void MainComponent::saveLayoutChanges ()
{
    const auto splitter1Size { bankListEditorSplitter.getSplitOffset () };
    const auto splitter2Size { folderBrowserEditorSplitter.getSplitOffset () };
    guiProperties.setPaneSizes (splitter1Size, splitter2Size, 0, false);
}

void MainComponent::paint (juce::Graphics& g)
{
    // the children do not cover every pixel, so this has to paint the gaps -
    // without it, a palette change leaves the old ground showing through
    g.fillAll (findColour (SquidColours::windowBackground));
}

void MainComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    currentFolderComponent.setBounds (localBounds.removeFromTop (30));
    bottomStatusWindow.setBounds (localBounds.removeFromBottom (toolWindowHeight));
    localBounds.reduce (3, 3);
    folderBrowserEditorSplitter.setBounds (localBounds);
}
