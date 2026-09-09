#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"
#include "GuiProperties.h"
#include "Theme/UiComponents.h"

/*
    The strip along the top of the window: where you are, which output you are
    listening on, and the way in to Settings.

    The path is drawn as breadcrumbs rather than a raw path string - the folder
    you are in is the part that matters, so it is the part that is not dimmed.
*/
class CurrentFolderComponent : public juce::Component
{
public:
    CurrentFolderComponent ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;
    GuiProperties guiProperties;
    juce::AudioDeviceManager* audioDeviceManager { nullptr };

    juce::StringArray pathSegments;

    ChromeButton outputButton { "OUT" };
    ChromeButton settingsButton { "SETTINGS" };

    void setFolder (juce::String folderName);
    void showOutputMenu ();
    void refreshOutputName ();

    void paint (juce::Graphics& g) override;
    void resized () override;
};
