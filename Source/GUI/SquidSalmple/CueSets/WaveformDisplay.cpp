#include "WaveformDisplay.h"
#include "../../../SystemServices.h"
#include "../../../SquidSalmple/Metadata/SquidSalmpleDefs.h"
#include "oolib/Debug/DebugLog.h"
#include "oolib/Debug/DumpStack.h"
#include "oolib/Properties/RuntimeRootProperties.h"

constexpr auto kMaxSampleLength { 524287 };

#define LOG_WAVEFORM_DISPLAY 0
#if LOG_WAVEFORM_DISPLAY
#define LogWaveformDisplay(text) DebugLog ("WaveformDisplay", text);
#else
#define LogWaveformDisplay(text) ;
#endif

// The Squid only ever deals in 44k1 samples, so the timeline needs no other rate.
constexpr auto kSquidSampleRate { 44100.0 };
constexpr auto kTimelineHeight { 18 };

// Where the view stops drawing the real sample line and switches to the min/max
// peak envelope. Well above the point where the two representations coincide
// (1 sample per pixel), so a cue set stays on the sample line until it is quite
// zoomed out.
constexpr auto kPeakEnvelopeThreshold { 30.0 };

// The colours the cue set editor has always used: a black waveform on mid grey,
// with the markers and the border in white.
const juce::Colour kBackgroundColour { juce::Colours::grey.darker (0.3f) };
const juce::Colour kForegroundColour { juce::Colours::black };
const juce::Colour kMarkerColour { juce::Colours::white };

WaveformDisplay::WaveformDisplay ()
{
    setupColours ();
    waveformView.setPeakEnvelopeThreshold (kPeakEnvelopeThreshold);
    waveformView.onViewChanged = [this] () { syncTimelineToView (); };
    addAndMakeVisible (waveformView);

    setupMarkers ();
    markerOverlay.setWaveformView (&waveformView);
    markerOverlay.formatPosition = [this] (double sample) { return timeline.formatSamplePosition (sample); };
    markerOverlay.constrainPosition = [this] (int markerIndex, double proposedPosition) { return constrainMarker (markerIndex, proposedPosition); };
    markerOverlay.onMarkerMoved = [this] (int markerIndex) { markerMoved (markerIndex); };
    addAndMakeVisible (markerOverlay);

    // Bars and beats mean nothing to a Squid cue set, so the ruler offers only
    // samples - the units the cue points themselves are edited in, and so the
    // default - and time. Right-clicking the ruler switches between them.
    timeline.setSampleRate (kSquidSampleRate);
    timeline.setAvailableUnits ({ TimelineComponent::Unit::samples, TimelineComponent::Unit::timeMinutesSeconds });
    timeline.setUnit (TimelineComponent::Unit::samples);
    // The marker drag labels are formatted by the timeline, so they follow it.
    timeline.onUnitChanged = [this] (TimelineComponent::Unit) { markerOverlay.repaint (); };
    addAndMakeVisible (timeline);
}

void WaveformDisplay::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties { rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no };
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();
}

void WaveformDisplay::setupColours ()
{
    // Every part of the waveform is drawn in the one foreground colour, so the
    // peak envelope, the RMS body inside it and the per-sample line all read as
    // the single black trace this editor has always shown.
    WaveformView::ColourScheme waveformColours;
    waveformColours.background = kBackgroundColour;
    waveformColours.peak = kForegroundColour;
    waveformColours.rms = kForegroundColour;
    waveformColours.sampleLine = kForegroundColour;
    waveformColours.sampleDot = kForegroundColour;
    // The old display had no centre line at all, so keep it to a hint of the
    // foreground rather than a line that competes with the waveform.
    waveformColours.centreLine = kForegroundColour.withAlpha (0.25f);
    waveformView.setColourScheme (waveformColours);

    TimelineComponent::ColourScheme timelineColours;
    timelineColours.background = kBackgroundColour;
    timelineColours.majorTick = kForegroundColour;
    timelineColours.minorTick = kForegroundColour.withAlpha (0.55f);
    timelineColours.text = kForegroundColour;
    timeline.setColourScheme (timelineColours);
}

void WaveformDisplay::setupMarkers ()
{
    // Start and end bracket the cue set, so their handles point inwards, which
    // keeps them apart and readable when the two markers meet. The loop point
    // hangs off the bottom edge on a dashed line, as it always has, so it never
    // reads as one of that pair.
    MarkerOverlay::Style startStyle;
    startStyle.colour = kMarkerColour;
    startStyle.shape = MarkerOverlay::HandleShape::rectangle;
    startStyle.placement = MarkerOverlay::HandlePlacement::top;
    startStyle.alignment = MarkerOverlay::HandleAlignment::rightOfLine;

    auto loopStyle { startStyle };
    loopStyle.dashed = true;
    loopStyle.placement = MarkerOverlay::HandlePlacement::bottom;

    auto endStyle { startStyle };
    endStyle.alignment = MarkerOverlay::HandleAlignment::leftOfLine;

    auto addMarker = [this] (juce::StringRef name, const MarkerOverlay::Style& style)
    {
        MarkerOverlay::Marker marker;
        marker.name = name;
        marker.style = style;
        markerOverlay.addMarker (marker);
    };
    addMarker ("Start", startStyle); // kStartMarker
    addMarker ("Loop", loopStyle);   // kLoopMarker
    addMarker ("End", endStyle);     // kEndMarker
}

void WaveformDisplay::setChannelIndex (int theChannelIndex)
{
    channelIndex = theChannelIndex;
}

void WaveformDisplay::setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer)
{
    LogWaveformDisplay ("setAudioBuffer");
    // Being handed the sample that is already on display has to leave the view
    // alone - anything else would throw away where the user has zoomed to every
    // time some other part of the editor refreshes itself.
    const auto sameSample { theAudioBuffer != nullptr && theAudioBuffer == audioBuffer
                            && theAudioBuffer->getNumSamples () == waveformView.getNumSamples () };
    const auto viewStartSample { waveformView.getVisibleStartSample () };
    const auto viewSamplesPerPixel { waveformView.getSamplesPerPixel () };

    audioBuffer = theAudioBuffer;
    waveformView.setAudioBuffer (audioBuffer); // this rebuilds the peaks, and fits the view to the sample
    if (sameSample)
        waveformView.setVisibleRange (viewStartSample, viewSamplesPerPixel * waveformView.getWidth ());
    // The markers were bounded by the previous sample's length, so they have to
    // be re-applied now that the new one has set the bounds.
    updateMarkerPositions ();
    syncTimelineToView ();
    repaint ();
}

void WaveformDisplay::setCueEndPoint (uint32_t newCueEnd)
{
    LogWaveformDisplay ("setCueEndPoint");
    cueEnd = newCueEnd;
    markerOverlay.setPosition (kEndMarker, cueEnd);
}

void WaveformDisplay::setCueLoopPoint (uint32_t newCueLoop)
{
    LogWaveformDisplay ("setCueLoopPoint");
    cueLoop = newCueLoop;
    markerOverlay.setPosition (kLoopMarker, cueLoop);
}

void WaveformDisplay::setCuePoints (uint32_t newCueStart, uint32_t newCueLoop, uint32_t newCueEnd)
{
    LogWaveformDisplay ("setCuePoints");
    cueStart = newCueStart;
    cueLoop = newCueLoop;
    cueEnd = newCueEnd;
    updateMarkerPositions ();
}

void WaveformDisplay::setCueStartPoint (uint32_t newCueStart)
{
    LogWaveformDisplay ("setCueStartPoint");
    cueStart = newCueStart;
    markerOverlay.setPosition (kStartMarker, cueStart);
}

void WaveformDisplay::setTimelineUnit (TimelineComponent::Unit unit)
{
    timeline.setUnit (unit);
    markerOverlay.repaint ();
}

TimelineComponent::Unit WaveformDisplay::getTimelineUnit () const
{
    return timeline.getUnit ();
}

double WaveformDisplay::constrainMarker (int markerIndex, double proposedPosition) const
{
    // MarkerOverlay treats each marker as independent, and only keeps them inside
    // the audio. The cue points additionally have to stay in start <= loop <= end
    // order, which is this app's rule to enforce.
    switch (markerIndex)
    {
        case kStartMarker:
            return std::min (proposedPosition, static_cast<double> (cueEnd));
        case kLoopMarker:
            return std::clamp (proposedPosition, static_cast<double> (cueStart), static_cast<double> (cueEnd));
        case kEndMarker:
            return std::max (proposedPosition, static_cast<double> (cueStart));
        default:
            jassertfalse;
            return proposedPosition;
    }
}

void WaveformDisplay::markerMoved (int markerIndex)
{
    const auto newPosition { static_cast<uint32_t> (markerOverlay.getPosition (markerIndex)) };
    switch (markerIndex)
    {
        case kStartMarker:
        {
            LogWaveformDisplay ("markerMoved - kStartMarker");
            cueStart = newPosition;
            // Dragging the start up to the loop takes the loop along with it.
            if (cueStart > cueLoop)
                moveLoopTo (cueStart);
            if (onStartPointChange != nullptr)
                onStartPointChange (cueStart);
        }
        break;
        case kLoopMarker:
        {
            LogWaveformDisplay ("markerMoved - kLoopMarker");
            moveLoopTo (newPosition);
        }
        break;
        case kEndMarker:
        {
            LogWaveformDisplay ("markerMoved - kEndMarker");
            cueEnd = newPosition;
            // ...and dragging the end back onto the loop pulls the loop in.
            if (cueEnd < cueLoop)
                moveLoopTo (cueEnd);
            if (onEndPointChange != nullptr)
                onEndPointChange (cueEnd);
        }
        break;
        default:
        {
            jassertfalse;
        }
        break;
    }
}

void WaveformDisplay::moveLoopTo (uint32_t newCueLoop)
{
    cueLoop = newCueLoop;
    markerOverlay.setPosition (kLoopMarker, cueLoop);
    if (onLoopPointChange != nullptr)
        onLoopPointChange (cueLoop);
}

void WaveformDisplay::updateMarkerPositions ()
{
    markerOverlay.setPosition (kStartMarker, cueStart);
    markerOverlay.setPosition (kLoopMarker, cueLoop);
    markerOverlay.setPosition (kEndMarker, cueEnd);
}

void WaveformDisplay::syncTimelineToView ()
{
    // The ruler and the waveform share their horizontal bounds, so feeding the
    // ruler the waveform's view mapping keeps a pixel column meaning the same
    // sample in both. The markers map through the waveform directly.
    timeline.setView (waveformView.getVisibleStartSample (), waveformView.getSamplesPerPixel ());
    markerOverlay.repaint ();
}

void WaveformDisplay::resized ()
{
    LogWaveformDisplay ("resized");
    // The children sit inside the border this component draws around them.
    auto bounds { getLocalBounds ().reduced (1) };
    timeline.setBounds (bounds.removeFromTop (kTimelineHeight));
    waveformView.setBounds (bounds);
    markerOverlay.setBounds (bounds);
    syncTimelineToView ();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    // The children cover everything but the border.
    g.setColour (kMarkerColour);
    g.drawRect (getLocalBounds ());
}

void WaveformDisplay::paintOverChildren (juce::Graphics& g)
{
    // dropMsg
    // dropDetails
    constexpr auto dropMsgFontSizeSingle { 30.f };
    constexpr auto dropMsgFontSizeDouble { 20.f };
    constexpr auto dropDetailsFontSize   { 15.f };
    auto setBackgroundColor = [this, &g] ()
    {
        if (supportedFile)
            g.setColour (juce::Colours::white.withAlpha (0.7f));
        else
            g.setColour (juce::Colours::black.withAlpha (0.7f));
    };
    auto setTextColor = [this, &g] ()
    {
        if (supportedFile)
            g.setColour (juce::Colours::black);
        else
            g.setColour (juce::Colours::red.darker (0.5f));
    };
    if (draggingFilesCount > 0)
    {
        jassert (dropType != DropType::none);
        if (audioBuffer == nullptr)
        {
            g.fillAll (juce::Colours::white.withAlpha (0.1f));
            g.setFont (dropMsgFontSizeSingle);
            g.setColour (juce::Colours::black);
            if (draggingFilesCount == 1)
                g.drawText ("Assign sample to Channel " + juce::String (channelIndex + 1), getLocalBounds (), juce::Justification::centred, false);
            else
                g.drawText ("Concatenate samples with Cue Sets and assign to Channel " + juce::String (channelIndex + 1), getLocalBounds (), juce::Justification::centred, false);
        }
        else
        {
            auto displayTextWithBackground = [&g, this, &setBackgroundColor, &setTextColor] (juce::StringRef text, float fontSize, const juce::Rectangle<int>& bounds)
            {
                g.setFont (fontSize);
                setBackgroundColor ();
                // TODO - replace hardcoded 10.f with value derived from text height
                auto stringWidthPixels { juce::GlyphArrangement::getStringWidth (g.getCurrentFont (), text) + 10.f };
                auto center { bounds.getCentre () };
                g.fillRoundedRectangle ({ static_cast<float> (center.getX ()) - (stringWidthPixels / 2.f), static_cast<float> (center.getY ()) - (fontSize / 2.f), stringWidthPixels, fontSize + 5.f }, 10.f);
                setTextColor ();
                g.drawText (text, bounds, juce::Justification::centred, false);
            };
            auto localBounds { getLocalBounds () };
            juce::Colour fillColor { juce::Colours::white };
            const float activeAlpha { 0.1f };
            const float nonActiveAlpha { 0.5f };
            g.setColour (fillColor.withAlpha (dropType == DropType::replace ? activeAlpha : nonActiveAlpha));
            const auto topHalfBounds { localBounds.removeFromTop (localBounds.getHeight () / 2) };
            g.fillRect (topHalfBounds);
            g.setColour (fillColor.withAlpha (dropType == DropType::append ? activeAlpha : nonActiveAlpha));
            g.fillRect (localBounds);
            const auto dropBounds { dropType == DropType::replace ? topHalfBounds : localBounds };
            juce::String dropMessage { ((dropType == DropType::replace) ? "Replace: " : "Append: ") + dropMsg };
            if (dropDetails.isEmpty ())
            {
                // just display dropMsg using all the space in the drop zone
                displayTextWithBackground (dropMessage, dropMsgFontSizeSingle, dropBounds);
            }
            else
            {
                // sectionSpacing = (totalSpace - (bigFontHeight + (numberLines * (smallFontHeight + spaceBetweenLines))) / 3
                //
                // verticalSpace
                // bigFont
                // verticalSpace
                // littleFont * numLines
                // verticalSpace
                //
                // display dropMsg and dropDetails
                const auto linesToDisplay { std::min (5, dropDetails.size ()) };
                const auto backgroundLines { (linesToDisplay == dropDetails.size () ? dropDetails.size () : linesToDisplay + 1) + 1 };
                auto totalDropBounds { dropBounds };
                const auto sectionSpacing { (dropBounds.getHeight () - (dropMsgFontSizeDouble + (backgroundLines * (dropDetailsFontSize + 4)))) / 3 };
                const auto dropMsgBounds { totalDropBounds.removeFromTop (static_cast<int> (sectionSpacing + dropMsgFontSizeDouble + (sectionSpacing / 2))) };
                const auto dropDetailsBounds { totalDropBounds };
                displayTextWithBackground (dropMessage, dropMsgFontSizeDouble, dropMsgBounds);
                g.setFont (dropDetailsFontSize);
                auto maxDetailsWidthPixels = [this, &g, linesToDisplay] ()
                {
                    auto maxStringPixels { 0.f };
                    for (auto curDropDetailLineIndex { 0 }; curDropDetailLineIndex < linesToDisplay; ++curDropDetailLineIndex)
                    {
                        const auto& detailLine { dropDetails [curDropDetailLineIndex] };
                        if (auto stringWidthPixels { juce::GlyphArrangement::getStringWidth (g.getCurrentFont (), detailLine) + 10.f }; stringWidthPixels > maxStringPixels)
                            maxStringPixels = stringWidthPixels;
                    }
                    return maxStringPixels;
                } ();
                auto dropDetailsDisplayBounds { juce::Rectangle<int> { 0, 0, static_cast<int> (maxDetailsWidthPixels), backgroundLines * static_cast<int> (dropDetailsFontSize) }.withCentre (dropDetailsBounds.getCentre ()) };
                setBackgroundColor ();
                g.fillRoundedRectangle (dropDetailsDisplayBounds.toFloat (), 10.f);
                setTextColor ();
                dropDetailsDisplayBounds.removeFromTop (static_cast<int> (dropDetailsFontSize / 2));
                for (auto curDropDetailLineIndex { 0 }; curDropDetailLineIndex < linesToDisplay; ++curDropDetailLineIndex)
                {
                    const auto textBounds { dropDetailsDisplayBounds.removeFromTop (static_cast<int> (dropDetailsFontSize)) };
                    const auto& detailLine { dropDetails [curDropDetailLineIndex] };
                    g.drawText (detailLine, textBounds, juce::Justification::centred, false);
                }
                if (linesToDisplay < dropDetails.size ())
                    g.drawText ("...", dropDetailsDisplayBounds, juce::Justification::centred, false);
            }
        }
    }
}

void WaveformDisplay::setDropType (int x, int y)
{
    // if no file assigned
    if (audioBuffer == nullptr)
    {
         // only adding files
        dropType = DropType::replace;
        dropAreaId = 0;
    }
    else
    {
        // option to replace or append, present which based on hover location
        if (getLocalBounds ().removeFromTop (getLocalBounds ().getHeight () / 2).contains (x, y))
        {
            dropAreaId = 0;
            dropType = DropType::replace;
        }
        else
        {
            dropAreaId = 1;
            dropType = DropType::append;
        }
    }
}

bool WaveformDisplay::isInterestedInFileDrag (const juce::StringArray& files)
{
    if (isInterestedInFiles == nullptr)
        return false;
    return isInterestedInFiles (files);
}

void WaveformDisplay::resetDropInfo ()
{
    draggingFilesCount = 0;
    dropType = DropType::none;
    dropAreaId = 0;
    dropMsg = {};
    dropDetails = {};
}

void WaveformDisplay::filesDropped (const juce::StringArray& files, int x, int y)
{
    // TODO - do I really need setDropType here? ie. this is already called by fileDragEnter and fileDragMove
    setDropType (x, y);
    if (onFilesDropped != nullptr && supportedFile)
        onFilesDropped (files, dropType);
    resetDropInfo ();
    repaint ();
}

void WaveformDisplay::updateDropMessage (const juce::StringArray& files)
{
    auto filesConcatenated { 0 };
    uint64_t totalSize { 0 };
    juce::String tempDropDetails;
    dropMsg = {};
    auto updateDropDetails = [&tempDropDetails] (juce::String errorMsg)
    {
        tempDropDetails += (tempDropDetails.isNotEmpty () ? ", " : "") + errorMsg;
    };
    supportedFile = true;
    for (auto& fileName : files)
    {
        auto draggedFile { juce::File (fileName) };
        if (editManager->isSquidManagerSupportedAudioFile (draggedFile))
        {
            auto reader { editManager->getReaderFor (draggedFile) };
            const double ratio { kSquidSampleRate / reader->sampleRate };
            const int actualNumSamples { static_cast<int> (reader->lengthInSamples * ratio) };

            totalSize += actualNumSamples;
            if (totalSize < kMaxSampleLength)
                ++filesConcatenated;
        }
        else
        {
            updateDropDetails ("Unsupported file type: " + draggedFile.getFileName ());
            supportedFile = false;
        }
    }

    if (supportedFile)
    {
        // everything is perfect
        if (dropType == DropType::replace)
        {
            if (files.size () == 1)
            {
                dropMsg = "Assign sample to Channel " + juce::String (channelIndex + 1);
                if (totalSize > kMaxSampleLength)
                {
                    tempDropDetails = "Sample will be truncated to 11 seconds";
                }
            }
            else
            {
                dropMsg = "Concatenate samples with Cue Sets and assign to Channel " + juce::String (channelIndex + 1);
                if (totalSize > kMaxSampleLength)
                {
                    // append truncation msg and some msg about how many files will be added, or which ones won't, or, etc
                    // use filesConcatenated value
                    tempDropDetails = "Only " + juce::String (filesConcatenated) + " samples will fit in 11 seconds. The remaining " + juce::String (files.size () - filesConcatenated)  + " samples will be ignored";
                }
            }
        }
        else // dropType == DropType::append
        {
            if (audioBuffer->getNumSamples () + totalSize <= kMaxSampleLength)
            {
                if (files.size () == 1)
                {
                    dropMsg = "Sample will be appended and a new Cue Set created for it";
                }
                else
                {
                    dropMsg = "Samples will be appended and new Cue Sets created for them";
                }
            }
            else
            {
                if (filesConcatenated == 0)
                {
                    // indicate no append happening
                    // TODO - this is not an unsupported file, but we want the error colors and the drop ignored, which uses the supportedFile flag. We should change that to a generic error flag
                    dropMsg = "No samples can be appended";
                    tempDropDetails = "They do not fit in the remaining time of " + juce::String ((kMaxSampleLength - audioBuffer->getNumSamples ()) / kSquidSampleRate, 2) + " seconds";
                    supportedFile = false;
                }
                else
                {
                    dropMsg = "Samples will be appended and new Cue Sets created for them";
                    tempDropDetails = "Only " + juce::String (filesConcatenated) + " more samples will fit in 11 seconds. The remaining " + juce::String (files.size () - filesConcatenated) + " samples will be ignored";
                }
            }
        }
    }
    else
    {
        // cannot perform the drop because a file is either an unsupported wav file format, or an unsupported file type
        dropMsg = "Cannot accept files";
    }

    dropDetails.clear ();
    auto stringTokens { juce::StringArray::fromTokens (tempDropDetails, false) };
    juce::String tempDetails;
    constexpr auto maxLineLength { 140 };
    for (auto tokenIndex { 0 }; tokenIndex < stringTokens.size (); ++tokenIndex)
    {
        if (tempDetails.length () + stringTokens [tokenIndex].length () + 1 > maxLineLength)
        {
            dropDetails.add (tempDetails);
            tempDetails = {};
        }
        tempDetails += stringTokens [tokenIndex] + " ";
    }
    if (tempDetails.isNotEmpty ())
        dropDetails.add (tempDetails);
}

void WaveformDisplay::fileDragEnter (const juce::StringArray& files, int x, int y)
{
    draggingFilesCount = files.size ();
    setDropType (x, y);
    updateDropMessage (files);
    repaint ();
}

void WaveformDisplay::fileDragMove (const juce::StringArray& files, int x, int y)
{
    const auto prevDropType = dropType;
    setDropType (x, y);
    if (prevDropType != dropType)
    {
        updateDropMessage (files);
        repaint ();
    }
}

void WaveformDisplay::fileDragExit (const juce::StringArray&)
{
    resetDropInfo ();
    repaint ();
}
