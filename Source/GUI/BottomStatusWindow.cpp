#include "BottomStatusWindow.h"
#include "../Utility/RuntimeRootProperties.h"

BottomStatusWindow::BottomStatusWindow ()
{
    setOpaque (true);

    addAndMakeVisible (statusLabel);

    settingsButton.setButtonText ("SETTINGS");
    settingsButton.onClick = [this] ()
    {
        audioPlayerProperties.showConfigDialog (false);
    };
    addAndMakeVisible (settingsButton);
}

void BottomStatusWindow::init (juce::ValueTree rootPropertiesVT)
{
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
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colours::white);
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
