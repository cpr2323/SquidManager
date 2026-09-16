#include "CurrentFolderComponent.h"
#include "Theme/SquidColourIds.h"
#include "../SystemServices.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

CurrentFolderComponent::CurrentFolderComponent ()
{
    setOpaque (true);
    outputButton.setShowsCaret (true);
    outputButton.onClick = [this] () { showOutputMenu (); };
    addAndMakeVisible (outputButton);

    settingsButton.onClick = [this] () { guiProperties.showSettingsDialog (false); };
    addAndMakeVisible (settingsButton);
}

void CurrentFolderComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);
    // the folder being viewed and the bank last opened change independently
    appProperties.onMostRecentFolderChange = [this] (juce::String) { refreshPath (); };
    appProperties.onMostRecentFileChange = [this] (juce::String) { refreshPath (); };

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    audioDeviceManager = systemServices.getAudioDeviceManager ();

    // a bank folder that did not exist yet appears when the folder is rescanned, such
    // as after the first save of a new bank
    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    directoryDataProperties.onRootScanComplete = [this] () { refreshPath (); };

    refreshPath ();
    refreshOutputName ();
}

void CurrentFolderComponent::refreshPath ()
{
    const auto viewedFolder { appProperties.getMostRecentFolder () };
    pathSegments.clear ();
    pathSegments.addTokens (viewedFolder, juce::File::getSeparatorString (), {});
    pathSegments.removeEmptyStrings ();

    // Banks are the numbered folders inside the folder being viewed, so the last
    // bank opened is only still open if it is one of those. A folder with no banks
    // opens a default bank in the first slot, which is not shown until it is saved.
    const auto lastBankOpened { appProperties.getRecentlyUsedFile (0) };
    pathEndsInBank = viewedFolder.isNotEmpty () && lastBankOpened.isNotEmpty ()
                     && juce::File (lastBankOpened).getParentDirectory () == juce::File (viewedFolder)
                     && juce::File (lastBankOpened).isDirectory ();
    if (pathEndsInBank)
        pathSegments.add (juce::File (lastBankOpened).getFileName ());

    repaint ();
}

void CurrentFolderComponent::refreshOutputName ()
{
    auto deviceName { juce::String ("none") };
    if (audioDeviceManager != nullptr)
        if (auto* device { audioDeviceManager->getCurrentAudioDevice () })
            deviceName = device->getName ();

    outputButton.setValueText (deviceName);
    resized ();
}

void CurrentFolderComponent::showOutputMenu ()
{
    if (audioDeviceManager == nullptr)
        return;

    auto* deviceType { audioDeviceManager->getCurrentDeviceTypeObject () };
    if (deviceType == nullptr)
        return;

    deviceType->scanForDevices ();
    const auto deviceNames { deviceType->getDeviceNames (false) };
    const auto currentName { audioDeviceManager->getCurrentAudioDevice () != nullptr
                             ? audioDeviceManager->getCurrentAudioDevice ()->getName ()
                             : juce::String () };

    juce::PopupMenu menu;
    menu.addSectionHeader (deviceType->getTypeName ());
    menu.addSeparator ();
    for (auto deviceIndex { 0 }; deviceIndex < deviceNames.size (); ++deviceIndex)
    {
        const auto name { deviceNames [deviceIndex] };
        menu.addItem (name, true, name == currentName, [this, name] ()
        {
            auto setup { audioDeviceManager->getAudioDeviceSetup () };
            setup.outputDeviceName = name;
            // an empty result means it opened; anything else is the reason it did not
            const auto result { audioDeviceManager->setAudioDeviceSetup (setup, true) };
            if (result.isNotEmpty ())
                juce::Logger::writeToLog ("could not open audio output '" + name + "': " + result);
            refreshOutputName ();
        });
    }
    menu.showMenuAsync (juce::PopupMenu::Options ().withTargetComponent (&outputButton));
}

void CurrentFolderComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (SquidColours::listBackground));
    g.setColour (findColour (SquidColours::outline));
    g.drawHorizontalLine (getHeight () - 1, 0.0f, static_cast<float> (getWidth ()));

    // breadcrumbs: the folders are context, so they are dimmed; the open bank, when
    // there is one, is the part that matters, so it is not
    const auto separator { juce::String (juce::CharPointer_UTF8 ("\xe2\x80\xba")) };
    constexpr auto kGap { 5 };
    const auto contextFont { SquidType::body () };
    const auto currentFont { SquidType::bodyStrong () };
    const auto textArea { getLocalBounds ().withTrimmedBottom (1) };

    auto x { kPadding };
    const auto rightEdge { outputButton.getX () - 12 };
    for (auto segmentIndex { 0 }; segmentIndex < pathSegments.size (); ++segmentIndex)
    {
        const auto isLast { segmentIndex == pathSegments.size () - 1 };
        const auto isBank { isLast && pathEndsInBank };
        const auto segment { pathSegments [segmentIndex] };
        const auto& font { isBank ? currentFont : contextFont };
        const auto segmentWidth { SquidPaint::textWidth (font, segment) };
        if (x + segmentWidth > rightEdge)
            break;

        g.setFont (font);
        g.setColour (findColour (isBank ? SquidColours::text : SquidColours::textDim));
        g.drawText (segment, textArea.withX (x).withWidth (segmentWidth + 1), juce::Justification::centredLeft, false);
        x += segmentWidth + kGap;

        if (! isLast)
        {
            const auto separatorWidth { SquidPaint::textWidth (contextFont, separator) };
            g.setFont (contextFont);
            g.setColour (findColour (SquidColours::menuHeaderText));
            g.drawText (separator, textArea.withX (x).withWidth (separatorWidth + 1), juce::Justification::centredLeft, false);
            x += separatorWidth + kGap;
        }
    }
}

void CurrentFolderComponent::resized ()
{
    auto localBounds { getLocalBounds ().withTrimmedBottom (1).reduced (kPadding, 0) };
    const auto settingsWidth { settingsButton.getIdealWidth () };
    settingsButton.setBounds (localBounds.removeFromRight (settingsWidth).withSizeKeepingCentre (settingsWidth, ChromeButton::kChipHeight));
    localBounds.removeFromRight (8);

    // a long device name is cut short rather than pushing the path out of view
    const auto outputWidth { std::min (outputButton.getIdealWidth (), 320) };
    outputButton.setBounds (localBounds.removeFromRight (outputWidth).withSizeKeepingCentre (outputWidth, ChromeButton::kChipHeight));
}
