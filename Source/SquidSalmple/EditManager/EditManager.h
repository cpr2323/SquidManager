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
    void forChannels (std::vector<int> channelIndexList, std::function<void (juce::ValueTree)> channelCallback);
    bool isAltOutput (int channelIndex);
    bool isAltOutput (juce::ValueTree channelPropertiesVT);
    bool isSupportedAudioFile (juce::File file);
    bool isCueRandomOn (int channelIndex);
    bool isCueRandomOn (juce::ValueTree channelPropertiesVT);
    bool isCueStepOn (int channelIndex);
    bool isCueStepOn (juce::ValueTree channelPropertiesVT);
    void loadBank (juce::File bankDirectory);
    void loadBankDefaults (uint8_t bankIndex);
    void loadChannel (juce::ValueTree squidChannelPropertiesVT, uint8_t channelIndex, juce::File sampleFile);
    void saveChannel (juce::ValueTree squidChannelPropertiesVT, uint8_t channelIndex, juce::File sampleFile);
    void saveBank ();
    void setCueRandom (int channelIndex, bool on);
    void setCueStep (int channelIndex, bool on);
    void setAltOutput (int channelIndex, bool useAltOutput);
    void setAltOutput (juce::ValueTree channelPropertiesVT, bool useAltOutput);
    juce::ValueTree getUneditedBankProperties ();
    juce::ValueTree getDefaultBankProperties ();
    juce::ValueTree getUneditedChannelProperties (int channelIndex);
    juce::ValueTree getDefaultChannelProperties (int channelIndex);

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    SquidBankProperties uneditedSquidBankProperties;
    SquidBankProperties defaultSquidBankProperties;
    SquidBankProperties squidBankProperties;
    juce::File bankDirectory;
    std::array<SquidChannelProperties, 8> channelPropertiesList;

    juce::AudioFormatManager audioFormatManager;

    void addSampleToChannelProperties (juce::ValueTree channelProperties, juce::File sampleFile);
    void copyBank (SquidBankProperties& srcBankProperties, SquidBankProperties& destBankProperties);
    bool isAltOutput (SquidChannelProperties& channelProperties);
    bool isCueRandomOn (SquidChannelProperties& channelProperties);
    bool isCueStepOn (SquidChannelProperties& channelProperties);
    void setAltOutput (SquidChannelProperties& channelProperties, bool useAltOutput);
};