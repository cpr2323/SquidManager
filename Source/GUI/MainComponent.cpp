#include "MainComponent.h"
#include "Theme/SquidColourIds.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

const auto kPathBarHeight { 34 };
const auto kStatusBarHeight { 26 };
// the ground showing around and between the panes
const auto kPaneMargin { 5 };

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
    // nested inside the splitter below, so it must not inset its panes a second time
    bankListEditorSplitter.setOuterMargin (0);

    folderBrowserEditorSplitter.setComponents (&fileViewComponent, &bankListEditorSplitter);
    folderBrowserEditorSplitter.setHorizontalSplit (false);
    folderBrowserEditorSplitter.setOuterMargin (kPaneMargin);

    // the panes cannot be dragged narrower than their headers need
    folderBrowserEditorSplitter.constrainSplitOffset = [this] (int proposedSplitOffset) { return constrainFolderPaneOffset (proposedSplitOffset); };
    bankListEditorSplitter.constrainSplitOffset = [this] (int proposedSplitOffset) { return constrainBankPaneOffset (proposedSplitOffset); };
    bankListEditorSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };
    folderBrowserEditorSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };

    restoreLayout ();

    addAndMakeVisible (currentFolderComponent);
    addAndMakeVisible (folderBrowserEditorSplitter);
    addAndMakeVisible (bottomStatusWindow);

    // The theme is installed before any window exists, so the startup pass of
    // sendLookAndFeelChange reached nothing. Without this, every component that
    // applies its own colours in lookAndFeelChanged stays unstyled until the
    // background slider is first moved.
    sendLookAndFeelChange ();
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
    applyMinimumPaneWidths ();
}

// A split offset is measured to the middle of its bar, so the pane before it is half
// a bar narrower than the offset.
constexpr auto kHalfSplitBar { 2 };

int MainComponent::constrainFolderPaneOffset (int proposedSplitOffset)
{
    return std::max (proposedSplitOffset, fileViewComponent.getMinimumWidth () + kHalfSplitBar);
}

int MainComponent::constrainBankPaneOffset (int proposedSplitOffset)
{
    return std::max (proposedSplitOffset, bankListComponent.getMinimumWidth () + kHalfSplitBar);
}

// for offsets that did not come from a drag: the stored layout, which may predate the minimums
void MainComponent::applyMinimumPaneWidths ()
{
    if (const auto folderOffset { constrainFolderPaneOffset (folderBrowserEditorSplitter.getSplitOffset ()) };
        folderOffset != folderBrowserEditorSplitter.getSplitOffset ())
        folderBrowserEditorSplitter.setSplitOffset (folderOffset);
    if (const auto bankOffset { constrainBankPaneOffset (bankListEditorSplitter.getSplitOffset ()) };
        bankOffset != bankListEditorSplitter.getSplitOffset ())
        bankListEditorSplitter.setSplitOffset (bankOffset);
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
    currentFolderComponent.setBounds (localBounds.removeFromTop (kPathBarHeight));
    bottomStatusWindow.setBounds (localBounds.removeFromBottom (kStatusBarHeight));
    folderBrowserEditorSplitter.setBounds (localBounds);
}
