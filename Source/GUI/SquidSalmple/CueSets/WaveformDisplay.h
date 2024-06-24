#pragma once

#include <JuceHeader.h>

class WaveformDisplay : public juce::Component
{
public:
    void init (juce::AudioBuffer<float>* theAudioBuffer);
    void setCuePoints (uint32_t newCueStart, uint32_t newCueLoop, uint32_t newCueEnd);

    std::function<void (uint32_t startPoint)> onStartPointChange;
    std::function<void (uint32_t loopPoint)> onLoopPointChange;
    std::function<void (uint32_t endPoint)> onEndPointChange;
private:
    enum class EditHandleIndex
    {
        kNone = -1,
        kStart = 0,
        kLoop = 1,
        kEnd = 2,
    };

    uint32_t cueStart { 0 };
    uint32_t cueLoop { 0 };
    uint32_t cueEnd { 0 };

    juce::AudioBuffer<float>* audioBuffer { nullptr };

    juce::int64 numSamples { 0 };
    int halfHeight { 0 };
    int numPixels { 0 };
    int samplesPerPixel { 0 };
    int markerStartY { 1 };
    int markerEndY { 0 };
    std::array<float, 2> dashedSpec;
    juce::Rectangle<int> sampleStartHandle;
    juce::Rectangle<int> sampleLoopHandle;
    juce::Rectangle<int> sampleEndHandle;
    int sampleStartMarkerX { 0 };
    int sampleLoopMarkerX { 0 };
    int sampleEndMarkerX { 0 };
    EditHandleIndex handleIndex { EditHandleIndex::kNone };

    void displayWaveform (juce::Graphics& g);
    void displayMarkers (juce::Graphics& g);

    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};