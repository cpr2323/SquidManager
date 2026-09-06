#pragma once

#include <JuceHeader.h>
#include "../../../SquidSalmple/EditManager/EditManager.h"
#include "oolib/GUI/InteractiveWaveform.h"
#include "oolib/GUI/MarkerOverlay.h"
#include "oolib/GUI/TimelineComponent.h"

// A cue set editor: the timeline ruler, the waveform (pan/zoom), and the three
// cue markers on top of it. The drawing, navigation and marker editing all come
// from oolib; what lives here is the file drag and drop, and the cue point rules
// the Squid imposes on the markers (start <= loop <= end), which MarkerOverlay
// deliberately knows nothing about.
class WaveformDisplay : public juce::Component,
                        public juce::FileDragAndDropTarget
{
public:
    enum class DropType { none, replace, append };

    WaveformDisplay ();

    void init (juce::ValueTree rootPropertiesVT);
    void setChannelIndex (int theChannelIndex);
    void setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer);
    void setCueEndPoint (uint32_t newCueEnd);
    void setCueLoopPoint (uint32_t newCueLoop);
    void setCuePoints (uint32_t newCueStart, uint32_t newCueLoop, uint32_t newCueEnd);
    void setCueStartPoint (uint32_t newCueStart);

    // The units the ruler (and the marker drag labels) are shown in. Samples by
    // default; right-clicking the ruler switches between samples and time.
    void setTimelineUnit (TimelineComponent::Unit unit);
    TimelineComponent::Unit getTimelineUnit () const;

    std::function<void (uint32_t startPoint)> onStartPointChange;
    std::function<void (uint32_t loopPoint)> onLoopPointChange;
    std::function<void (uint32_t endPoint)> onEndPointChange;
    std::function<void (const juce::StringArray& files, DropType dropType)> onFilesDropped;
    std::function<bool (const juce::StringArray& files)> isInterestedInFiles;

private:
    // Marker indices, in the order they are added to the overlay.
    enum MarkerIndex
    {
        kStartMarker = 0,
        kLoopMarker = 1,
        kEndMarker = 2,
    };

    EditManager* editManager { nullptr };

    InteractiveWaveform waveformView;
    MarkerOverlay markerOverlay;
    TimelineComponent timeline;

    uint32_t cueStart { 0 };
    uint32_t cueLoop { 0 };
    uint32_t cueEnd { 0 };

    juce::AudioBuffer<float>* audioBuffer { nullptr };
    int channelIndex { 0 };

    int draggingFilesCount { 0 };
    bool supportedFile { false };
    juce::String dropMsg;
    juce::StringArray dropDetails;
    DropType dropType { DropType::none };
    int dropAreaId { 0 };

    void setupColours ();
    void setupMarkers ();
    double constrainMarker (int markerIndex, double proposedPosition) const;
    void markerMoved (int markerIndex);
    void moveLoopTo (uint32_t newCueLoop);
    void updateMarkerPositions ();
    void syncTimelineToView ();

    void resetDropInfo ();
    void setDropType (int x, int y);

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int, int) override;
    void updateDropMessage (const juce::StringArray& files);
    void fileDragEnter (const juce::StringArray& files, int, int) override;
    void fileDragMove (const juce::StringArray& files, int, int) override;
    void fileDragExit (const juce::StringArray& files) override;

    void resized () override;
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
};
