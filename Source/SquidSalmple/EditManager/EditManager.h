#pragma once

#include <JuceHeader.h>
#include "../SquidBankProperties.h"
#include "../SquidChannelProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

class EditManager
{
public:
    void init (juce::ValueTree rootPropertiesVT);

    void loadBank (juce::File bankDirectory);
    void loadBankDefaults (uint8_t bankIndex);
    void loadChannel (juce::ValueTree squidChannelPropertiesVT, uint8_t channelIndex, juce::File sampleFile);
    void saveBank ();

private:
    RuntimeRootProperties runtimeRootProperties;
    SquidBankProperties uneditedSquidBankProperties;
    SquidBankProperties squidBankProperties;
    std::array<SquidChannelProperties, 8> squidChannelPropertiesList;
    juce::File bankDirectory;

    void copyBank (SquidBankProperties& srcBankProperties, SquidBankProperties& destBankProperties);

};