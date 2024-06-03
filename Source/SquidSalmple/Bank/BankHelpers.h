#pragma once

#include <JuceHeader.h>

namespace BankHelpers
{
    bool areEntireBanksEqual (juce::ValueTree bankOneVT, juce::ValueTree bankTwoVT);
    bool areBanksEqual (juce::ValueTree bankOneVT, juce::ValueTree bankTwoVT);
    bool areChannelsEqual (juce::ValueTree channelOneVT, juce::ValueTree channelTwoVT);
};
