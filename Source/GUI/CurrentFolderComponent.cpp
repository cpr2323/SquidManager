#include "CurrentFolderComponent.h"
#include "Theme/SquidColourIds.h"
#include "../SystemServices.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

CurrentFolderComponent::CurrentFolderComponent ()
{
    setOpaque (true);
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
    appProperties.onMostRecentFileChange = [this] (juce::String folderName) { setFolder (folderName); };

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    audioDeviceManager = systemServices.getAudioDeviceManager ();

    setFolder (appProperties.getRecentlyUsedFile (0));
    refreshOutputName ();
}

void CurrentFolderComponent::setFolder (juce::String folderName)
{
    pathSegments.clear ();
    pathSegments.addTokens (folderName, juce::File::getSeparatorString (), {});
    pathSegments.removeEmptyStrings ();
    repaint ();
}

void CurrentFolderComponent::refreshOutputName ()
{
    auto deviceName { juce::String ("none") };
    if (audioDeviceManager != nullptr)
        if (auto* device { audioDeviceManager->getCurrentAudioDevice () })
            deviceName = device->getName ();

    outputButton.setButtonText ("OUT  " + deviceName);
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
    g.fillAll (findColour (SquidColours::panelHeader));
    g.setColour (findColour (SquidColours::outline));
    g.drawHorizontalLine (getHeight () - 1, 0.0f, static_cast<float> (getWidth ()));

    // breadcrumbs: everything before the current folder is context, so it is dimmed
    const auto dimColour { findColour (SquidColours::textDim) };
    const auto currentColour { findColour (SquidColours::text) };
    const auto separatorColour { findColour (SquidColours::textDim).withAlpha (0.55f) };

    g.setFont (juce::Font (juce::FontOptions (12.0f)));
    auto x { 10 };
    const auto rightEdge { outputButton.getX () - 12 };
    for (auto segmentIndex { 0 }; segmentIndex < pathSegments.size (); ++segmentIndex)
    {
        const auto isLast { segmentIndex == pathSegments.size () - 1 };
        const auto segment { pathSegments [segmentIndex] };
        const auto segmentWidth { juce::roundToInt (juce::GlyphArrangement::getStringWidth (g.getCurrentFont (), segment)) + 2 };
        if (x + segmentWidth > rightEdge)
            break;

        g.setColour (isLast ? currentColour : dimColour);
        g.drawText (segment, x, 0, segmentWidth, getHeight (), juce::Justification::centredLeft, false);
        x += segmentWidth;

        if (! isLast)
        {
            g.setColour (separatorColour);
            g.drawText (">", x + 3, 0, 10, getHeight (), juce::Justification::centredLeft, false);
            x += 15;
        }
    }
}

void CurrentFolderComponent::resized ()
{
    auto localBounds { getLocalBounds ().reduced (8, 5) };
    settingsButton.setBounds (localBounds.removeFromRight (74));
    localBounds.removeFromRight (6);

    const auto outputWidth { juce::jlimit (90, 260,
                                           juce::roundToInt (juce::GlyphArrangement::getStringWidth (juce::Font (juce::FontOptions (11.0f)),
                                                                                                     outputButton.getButtonText ())) + 24) };
    outputButton.setBounds (localBounds.removeFromRight (outputWidth));
}
