#pragma once

#include <JuceHeader.h>

// TODO - refactor to take a ChannelProperties VT and get the data from there
//        Will just need an function to set whether to use Sample or Loop points
class LoopPointsView : public juce::Component
{
public:
    void setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer);
    void setLoopPoints (uint32_t theSampleOffset, uint32_t theNumSamples);

private:
    juce::AudioBuffer<float>* audioBuffer { nullptr };
    uint32_t sampleOffset { 0 };
    uint32_t numSamples { 0 };

    void paint (juce::Graphics& g);
};