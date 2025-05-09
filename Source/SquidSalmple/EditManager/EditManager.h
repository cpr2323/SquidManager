#pragma once

#include <JuceHeader.h>
#include "../SquidBankProperties.h"
#include "../SquidChannelProperties.h"
#include "../../AppProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

struct FileInfo
{
    bool supported { false };
    double sampleRate { 0.f };
    unsigned int bitsPerSample { 0 };
    int64_t lengthInSamples { 0 };
    unsigned int numChannels { 0 };
    bool usesFloatingPointData { false };
};

class EditManager
{
public:
    EditManager ();

    void init (juce::ValueTree rootPropertiesVT);

    void concatenateAndBuildCueSets (const juce::StringArray& files, int channelIndex, juce::String outputFileName, juce::ValueTree cueSetListVT);
    void cleanUpTempFiles (juce::File bankFolder);
    void clearChannel (int channelIndex);
    void cloneCvAssigns (int srcChannelIndex, int srcCvAssign, int destChannelIndex, int destCvAssing);
    bool copySampleToChannel (juce::File srcFile, juce::File destFile);
    juce::PopupMenu createChannelInteractionMenu (int channelIndex, juce::String interactionArticle, std::function <void (SquidChannelProperties&)> setter, std::function <bool (SquidChannelProperties&)> canInteractCallback, std::function <bool (SquidChannelProperties&)> canInteractToAllCallback);
    juce::PopupMenu createChannelEditMenu (juce::PopupMenu existingPopupMenu, int channelIndex, std::function <void (SquidChannelProperties&)> setter, std::function <void ()> resetter, std::function <void ()> reverter);
    int findNextZeroCrossing (int startSampleOffset, int maxSampleOffset, juce::AudioBuffer<float>& buffer);
    int findPreviousZeroCrossing (int startSampleOffset, int minSampleOffset, juce::AudioBuffer<float>& buffer);
    void forChannels (std::vector<int> channelIndexList, std::function<void (juce::ValueTree)> channelCallback);
    juce::ValueTree getChannelPropertiesVT (int channelIndex);
    std::unique_ptr<juce::AudioFormatReader> getReaderFor (const juce::File file);
    juce::String getFileTypesList ();
    bool isAltOutput (int channelIndex);
    bool isAltOutput (juce::ValueTree channelPropertiesVT);
    FileInfo getFileInfo (juce::File file);
    bool isCueRandomOn (int channelIndex);
    bool isCueRandomOn (juce::ValueTree channelPropertiesVT);
    bool isCueStepOn (int channelIndex);
    bool isCueStepOn (juce::ValueTree channelPropertiesVT);
    bool isSquidManagerSupportedAudioFile (const juce::File file);
    bool isSquidSalmpleSupportedAudioFile (const juce::File file);
    void loadBank (juce::File bankDirectory);
    void loadBankDefaults (uint8_t bankIndex);
    void loadChannel (juce::ValueTree squidChannelPropertiesVT, uint8_t channelIndex, juce::File sampleFile);
    void renameSample (int channelIndex, juce::String newSampleName);
    void saveChannel (juce::ValueTree squidChannelPropertiesVT, uint8_t channelIndex, juce::File sampleFile);
    void saveBank ();
    void setBankDefaults ();
    void setBankUnedited ();
    void setChannelDefaults (int channelIndex);
    void setChannelUnedited (int channelIndex);
    void setCueRandom (int channelIndex, bool on);
    void setCueStep (int channelIndex, bool on);
    void setAltOutput (int channelIndex, bool useAltOutput);
    void setAltOutput (juce::ValueTree channelPropertiesVT, bool useAltOutput);
    void swapChannels (int firstChannel, int secondChannel);
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
    juce::StringArray audioFileExtensions;

    void addSampleToChannelProperties (juce::ValueTree channelProperties, juce::File sampleFile);
    void cleanupChannelTempFiles ();
    void copyBank (SquidBankProperties& srcBankProperties, SquidBankProperties& destBankProperties);
    bool isAltOutput (SquidChannelProperties& channelProperties);
    bool isCueRandomOn (SquidChannelProperties& channelProperties);
    bool isCueStepOn (SquidChannelProperties& channelProperties);
    void sampleConvert (juce::AudioFormatReader* reader, juce::AudioBuffer<float>& outputBuffer);
    void setAltOutput (SquidChannelProperties& channelProperties, bool useAltOutput);
};