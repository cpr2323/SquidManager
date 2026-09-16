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
    // The left half shows the audio up to the end cue, the right half the audio from
    // the loop cue, so which half a drag starts in decides which cue it moves.
    enum class DragTarget { none, endCue, loopCue };

    juce::AudioBuffer<float>* audioBuffer { nullptr };
    DragTarget dragTarget { DragTarget::none };
    int dragStartCue { 0 };      // in samples
    int dragAnchorX { 0 };
    bool dragWasCoarse { false };
    uint32_t sampleOffset { 0 };
    uint32_t numSamples { 0 };
    SquidChannelProperties squidChannelProperties;

    EditManager* editManager { nullptr };

    void showLoopTunerMenu (const juce::MouseEvent& event);
    void setEndCueFromDrag (int newEndCue);
    void setLoopCueFromDrag (int newLoopCue);

    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;

    void paint (juce::Graphics& g) override;
};