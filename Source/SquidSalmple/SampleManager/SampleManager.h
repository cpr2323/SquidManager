#pragma once

#include <JuceHeader.h>
#include "SampleManagerProperties.h"
#include "SampleProperties.h"
#include "SampleStatus.h"
#include "../SquidChannelProperties.h"
#include "../../AppProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

class SampleManager
{
public:
    SampleManager ();
    void init (juce::ValueTree rootPropertiesVT);
    juce::ValueTree getSampleProperties (int channelIndex);
    bool isSupportedAudioFile (juce::File file);

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    SampleManagerProperties sampleManagerProperties;
    struct ChannelAndSampleProperties
    {
        SquidChannelProperties squidChannelProperties;
        SampleProperties sampleProperties;
    };
    std::array<ChannelAndSampleProperties, 8> channelAndSamplePropertiesList;

    juce::AudioFormatManager audioFormatManager;
    juce::File currentFolder;
    struct SampleData
    {
        int useCount { 0 };
        SampleStatus status { SampleStatus::uninitialized };
        int bitsPerSample { 0 };
        double sampleRate { 0.0 };
        int numChannels { 0 };
        juce::int64 lengthInSamples { 0 };
        juce::AudioBuffer<float> audioBuffer;
    };
    std::map <juce::String, SampleData> sampleList;

    SampleData& loadSample (juce::String fileName);
    void close (juce::String fileName);
    void clear ();
    void handleSampleChange (int channelIndex, juce::String sampleName);
    SampleData& open (juce::String fileName);
    void update ();
    void updateSample (juce::String fileName, SampleData& sampleData);
    void updateSampleProperties (juce::String fileName, SampleData& sampleData);
};