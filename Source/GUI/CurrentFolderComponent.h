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
    The open bank is added as the last crumb, but only while it is one of the
    banks of the folder being viewed; move to a folder with no banks and the
    crumbs stop at that folder.
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

    static constexpr int kPadding { 9 };
    juce::StringArray pathSegments;

    ChromeButton outputButton { "OUT", ChromeButton::Size::chip };
    ChromeButton settingsButton { "SETTINGS", ChromeButton::Size::chip };

    void refreshPath ();
    void showOutputMenu ();
    void refreshOutputName ();

    void paint (juce::Graphics& g) override;
    void resized () override;
};
