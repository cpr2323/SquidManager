#include "BottomStatusWindow.h"
#include "Theme/SquidColourIds.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

BottomStatusWindow::BottomStatusWindow ()
{
    setOpaque (true);

    addAndMakeVisible (statusLabel);

    settingsButton.setButtonText ("SETTINGS");
    settingsButton.onClick = [this] ()
    {
        guiProperties.showSettingsDialog (false);
    };
    addAndMakeVisible (settingsButton);
}

void BottomStatusWindow::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::owner, AudioPlayerProperties::EnableCallbacks::yes);
    bankListProperties.wrap (runtimeRootProperties.getValueTree (), BankListProperties::WrapperType::client, BankListProperties::EnableCallbacks::yes);
    bankListProperties.onStatusChange = [this] (juce::String status)
    {
        juce::MessageManager::callAsync ([this, status] ()
        {
            statusLabel.setText (status, juce::NotificationType::dontSendNotification);
        });
    };
}

void BottomStatusWindow::paint (juce::Graphics& g)
{
    g.fillAll (findColour (SquidColours::windowBackground));
    g.setColour (findColour (SquidColours::outline));
    g.drawRect (getLocalBounds (), 1);
}

void BottomStatusWindow::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (5, 3);
    statusLabel.setBounds (localBounds);
    const auto buttonWidth { 70 };
    settingsButton.setBounds (getWidth () - 5 - buttonWidth, getHeight () / 2 - 10, buttonWidth, 20);
}
