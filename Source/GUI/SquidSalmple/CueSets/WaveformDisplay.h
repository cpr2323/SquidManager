#pragma once

#include <JuceHeader.h>

using AudioBufferType = juce::AudioBuffer<float>;

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay ();

    void init (juce::File theTestFile);
    void setCuePoints (juce::int64 newCueStart, juce::int64 newCueLoop, juce::int64 newCueEnd);

private:
    enum class EditHandleIndex
    {
        kNone = -1,
        kStart = 0,
        kLoop = 1,
        kEnd = 2,
    };
    juce::File testFile;

    juce::int64 cueStart { 0 };
    juce::int64 cueLoop { 0 };
    juce::int64 cueEnd { 0 };

    juce::AudioFormatManager audioFormatManager;
    AudioBufferType audioBuffer;
    AudioBufferType* audioBufferPtr { nullptr };

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
    void updateData ();

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};