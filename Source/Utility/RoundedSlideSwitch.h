#pragma once

#include <JuceHeader.h>

using OnPopupMenuCallback = std::function<void ()>;

class RoundedSlideSwitch : public juce::Button,
                                  juce::Timer
{
public:
    RoundedSlideSwitch ();

    OnPopupMenuCallback onPopupMenuCallback;

private:
    float position { 0.0f };
    bool wasPopupMenuClick { false };

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void buttonStateChanged () override;
    void paintButton (juce::Graphics& g, bool, bool) override;
    void timerCallback () override;
};
