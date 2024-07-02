#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"
#include "../SquidSalmple/Audio/AudioPlayerProperties.h"

class BottomStatusWindow : public juce::Component
{
public:
    BottomStatusWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    AudioPlayerProperties audioPlayerProperties;

    juce::TextButton settingsButton;
    std::unique_ptr<juce::AlertWindow> settingsAlertWindow;

    void paint (juce::Graphics& g) override;
    void resized () override;
};
