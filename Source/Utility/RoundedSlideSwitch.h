#pragma once

#include <JuceHeader.h>

class RoundedSlideSwitch : public juce::Button,
                                  juce::Timer
{
public:
    RoundedSlideSwitch ();

private:
    float position { 0.0f };

    void buttonStateChanged () override;
    void paintButton (juce::Graphics& g, bool, bool) override;
    void timerCallback () override;
};
