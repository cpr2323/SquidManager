#pragma once

#include <JuceHeader.h>
#include "../../../SquidSalmple/EditManager/EditManager.h"

class WaveformDisplay : public juce::Component,
                        public juce::FileDragAndDropTarget
{
public:
    enum class DropType { none, replace, append };
    void init (juce::ValueTree rootPropertiesVT);
    void setChannelIndex (int theChannelIndex);
    void setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer);
    void setCueEndPoint (uint32_t newCueEnd);
    void setCueLoopPoint (uint32_t newCueLoop);
    void setCuePoints (uint32_t newCueStart, uint32_t newCueLoop, uint32_t newCueEnd);
    void setCueStartPoint (uint32_t newCueStart);

    std::function<void (uint32_t startPoint)> onStartPointChange;
    std::function<void (uint32_t loopPoint)> onLoopPointChange;
    std::function<void (uint32_t endPoint)> onEndPointChange;
    std::function<void (const juce::StringArray& files, DropType dropType)> onFilesDropped;
    std::function<bool (const juce::StringArray& files)> isInterestedInFiles;

private:
    enum class EditHandleIndex
    {
        kNone = -1,
        kStart = 0,
        kLoop = 1,
        kEnd = 2,
    };

    EditManager* editManager { nullptr };

    uint32_t cueStart { 0 };
    uint32_t cueLoop { 0 };
    uint32_t cueEnd { 0 };

    juce::AudioBuffer<float>* audioBuffer { nullptr };
    int channelIndex { 0 };

    juce::int64 numSamples { 0 };
    int halfHeight { 0 };
    int numPixels { 0 };
    float samplesPerPixel { 0.f };
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
    int draggingFilesCount { 0 };
    bool supportedFile { false };
    juce::String dropMsg;
    juce::StringArray dropDetails;
    DropType dropType { DropType::none };
    int dropAreaId { 0 };

    void displayMarkers (juce::Graphics& g);
    void displayWaveform (juce::Graphics& g);
    void resetDropInfo ();
    void setDropType (int x, int y);

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int, int) override;
    void updateDropMessage (const juce::StringArray& files);
    void fileDragEnter (const juce::StringArray& files, int, int) override;
    void fileDragMove (const juce::StringArray& files, int, int) override;
    void fileDragExit (const juce::StringArray& files) override;

    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void resized () override;
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
};