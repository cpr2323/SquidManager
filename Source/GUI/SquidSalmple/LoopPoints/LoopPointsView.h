#pragma once

#include <JuceHeader.h>
#include "../../../SquidSalmple/EditManager/EditManager.h"

class LoopPointsView : public juce::Component
{
public:
    void init (juce::ValueTree squidChannelPropertiesVT, juce::ValueTree rootPropertiesVT);
    void setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer);
    void setLoopPoints (uint32_t theSampleOffset, uint32_t theNumSamples);

private:
    juce::AudioBuffer<float>* audioBuffer { nullptr };
    uint32_t sampleOffset { 0 };
    uint32_t numSamples { 0 };
    SquidChannelProperties squidChannelProperties;

    EditManager* editManager { nullptr };

    void mouseDown (const juce::MouseEvent& event) override;

    void paint (juce::Graphics& g) override;
};