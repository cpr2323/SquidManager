#pragma once

#include <JuceHeader.h>
#include "../SquidBankProperties.h"
#include "../SquidChannelProperties.h"
#include "../../AppProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

class EditManager
{
public:
    EditManager ();

    void init (juce::ValueTree rootPropertiesVT);

    void concatenateAndBuildCueSets (const juce::StringArray& files, int channelIndex);
    void cleanUpTempFiles (juce::File bankFolder);
    bool isSupportedAudioFile (juce::File file);
    void loadBank (juce::File bankDirectory);
    void loadBankDefaults (uint8_t bankIndex);
    void loadChannel (juce::ValueTree squidChannelPropertiesVT, uint8_t channelIndex, juce::File sampleFile);
    void saveChannel (juce::ValueTree squidChannelPropertiesVT, uint8_t channelIndex, juce::File sampleFile);
    void saveBank ();

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    SquidBankProperties uneditedSquidBankProperties;
    SquidBankProperties squidBankProperties;
    juce::File bankDirectory;
    std::array<SquidChannelProperties, 8> channelPropertiesList;

    juce::AudioFormatManager audioFormatManager;

    void copyBank (SquidBankProperties& srcBankProperties, SquidBankProperties& destBankProperties);
    void addSampleToChannelProperties (juce::ValueTree channelProperties, juce::File sampleFile);
public:
};