#include "RoundedSlideSwitch.h"

RoundedSlideSwitch::RoundedSlideSwitch () : Button ({})
{
    setClickingTogglesState (true);
}

void RoundedSlideSwitch::buttonStateChanged ()
{
    startTimer (10);
}

void RoundedSlideSwitch::timerCallback ()
{
    auto rate = 0.1f;
    rate *= getToggleState () ? 1.0f : -1.0f;

    position = juce::jlimit (0.0f, 1.0f, position + rate);

    if (position == 0.0f || position == 1.0f)
        stopTimer ();

    repaint ();
}

void RoundedSlideSwitch::paintButton (juce::Graphics& g, bool, bool)
{
    auto area = getLocalBounds ().toFloat ();

    g.setColour (juce::Colours::darkgrey.interpolatedWith (juce::Colours::lightgrey, position));
    g.fillRoundedRectangle (area, 8.0f);

    g.setColour (juce::Colours::lightgrey.interpolatedWith (juce::Colours::darkgrey, position));
    auto buttonWidth { area.getWidth () / 2.0f };
    g.fillRoundedRectangle (2.0f + (buttonWidth * position), 2.0f, buttonWidth - 4.0f, area.getHeight () - 4.0f, 6.0f);
}
