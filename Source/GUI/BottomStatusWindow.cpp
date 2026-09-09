#include "BottomStatusWindow.h"
#include "Theme/SquidColourIds.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

BottomStatusWindow::BottomStatusWindow ()
{
    setOpaque (true);

    addAndMakeVisible (statusLabel);

    // Settings now lives in the path bar at the top, alongside the output device;
    // this strip is left for transient status messages.
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
}
