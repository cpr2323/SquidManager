#include "WaveformDisplay.h"
#include "../../../Utility/DebugLog.h"

#define LOG_WAVEFORM_DISPLAY 0
#if LOG_WAVEFORM_DISPLAY 
#define LogWaveformDisplay(text) DebugLog ("WaveformDisplay", text);
#else
#define LogWaveformDisplay(text) ;
#endif

const auto markerHandleSize { 5 };

void WaveformDisplay::init (juce::AudioBuffer<float>* theAudioBuffer)
{
    audioBuffer = theAudioBuffer;
    if (audioBuffer == nullptr)
        return;
    numSamples = audioBuffer->getNumSamples();
}

void WaveformDisplay::setCueEndPoint (uint32_t newCueEnd)
{
    cueEnd = newCueEnd;
    resized ();
    repaint ();
}

void WaveformDisplay::setCueLoopPoint (uint32_t newCueLoop)
{
    cueLoop = newCueLoop;
    resized ();
    repaint ();
}

void WaveformDisplay::setCuePoints (uint32_t newCueStart, uint32_t newCueLoop, uint32_t newCueEnd)
{
    cueStart = newCueStart;
    cueLoop = newCueLoop;
    cueEnd = newCueEnd;
    resized ();
    repaint ();
}

void WaveformDisplay::setCueStartPoint (uint32_t newCueStart)
{
    cueStart = newCueStart;
    resized ();
    repaint ();
}

void WaveformDisplay::resized ()
{
    halfHeight = getHeight () / 2;
    numPixels = getWidth () - 2;
    markerEndY = getHeight () - 2;
    const auto dashSize { getHeight () / 11.f };
    dashedSpec = { dashSize, dashSize };

    if (audioBuffer == nullptr)
    {
        samplesPerPixel = 0.f;
        sampleStartMarkerX = 0;
        sampleLoopMarkerX = 0;
        sampleEndMarkerX = 0;
    }
    else
    {
        samplesPerPixel = static_cast<float> (numSamples) / getWidth ();
        sampleStartMarkerX = 1 + static_cast<int> ((static_cast<float> (cueStart) / static_cast<float> (numSamples) * numPixels));
        sampleLoopMarkerX = 1 + static_cast<int> ((static_cast<float> (cueLoop) / static_cast<float> (numSamples) * numPixels));
        sampleEndMarkerX = 1 + static_cast<int> ((static_cast<float> (cueEnd) / static_cast<float> (numSamples) * numPixels));
    }
    sampleStartHandle = { sampleStartMarkerX, markerStartY, markerHandleSize, markerHandleSize };
    sampleLoopHandle = { sampleLoopMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };
    sampleEndHandle = { sampleEndMarkerX - markerHandleSize, markerStartY, markerHandleSize, markerHandleSize };
}

void WaveformDisplay::displayWaveform (juce::Graphics& g)
{
    if (audioBuffer == nullptr)
        return;
    // TODO - implement side selection
    auto readPtr { audioBuffer->getReadPointer (0) };

    g.setColour (juce::Colours::black);
    // TODO - get proper end pixel if sample ends before end of display
    //auto curSampleValue { readPtr [0] };
    for (auto pixelIndex { 0 }; pixelIndex < numPixels - 1; ++pixelIndex)
    {
        if ((pixelIndex + 1) * samplesPerPixel < numSamples)
        {
            const auto pixelOffset { pixelIndex + 1 };
#if 0
            const auto nextSampleValue = [this, pixelIndex, readPtr] ()
            {
                auto newSampleValue { 0.f };
                for (auto curSampleIndexOffset { 0 }; curSampleIndexOffset < samplesPerPixel; ++curSampleIndexOffset)
                {
                    const auto sampleIndex { pixelIndex + 1 + curSampleIndexOffset };
                    //juce::Logger::outputDebugString ("  sample [" + juce::String (sampleIndex) + "] : " + juce::String (readPtr [sampleIndex]));
                    newSampleValue += readPtr [sampleIndex];
                }
                return newSampleValue / samplesPerPixel;
            } ();
            //juce::Logger::outputDebugString ("nextSampleValue: " + juce::String (nextSampleValue));
            g.drawLine (static_cast<float> (pixelOffset),     static_cast<float> (static_cast<int> (halfHeight + (curSampleValue * halfHeight))),
                        static_cast<float> (pixelOffset + 1), static_cast<float> (static_cast<int> (halfHeight + (nextSampleValue * halfHeight))));
            curSampleValue = nextSampleValue;
#else
            g.drawLine (static_cast<float> (pixelOffset),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [static_cast<int>(pixelIndex * samplesPerPixel)] * halfHeight))),
                        static_cast<float> (pixelOffset + 1),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [static_cast<int>((pixelIndex + 1) * samplesPerPixel)] * halfHeight))));

#endif
        }
        else
        {
            break;
        }
    }
}

void WaveformDisplay::displayMarkers (juce::Graphics& g)
{
    if (audioBuffer == nullptr)
        return; 

    g.setColour (juce::Colours::white);

    // draw sample start marker
    g.fillRect (sampleStartHandle);
    g.drawLine (juce::Line<int> {sampleStartMarkerX, markerStartY, sampleStartMarkerX, markerEndY}.toFloat ());

    // draw loop start marker
    g.fillRect (sampleLoopHandle);
    g.drawDashedLine (juce::Line<int>{ sampleLoopMarkerX, markerStartY, sampleLoopMarkerX, markerEndY }.toFloat (), dashedSpec.data (), 2);

    // draw sample end marker
    g.fillRect (sampleEndHandle);
    g.drawLine (juce::Line<int> {sampleEndMarkerX, markerStartY, sampleEndMarkerX, markerEndY}.toFloat ());
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey.darker (0.3f));
    g.fillRect (getLocalBounds ());

    displayWaveform (g);
    displayMarkers (g);

    g.setColour (juce::Colours::white);
    g.drawRect (getLocalBounds ());
}

void WaveformDisplay::mouseMove (const juce::MouseEvent& e)
{
    if (audioBuffer == nullptr)
        return;

    LogWaveformDisplay ("mouseMove");
    if (sampleStartHandle.contains (e.getPosition ()))
        handleIndex = EditHandleIndex::kStart;
    else if (sampleLoopHandle.contains (e.getPosition ()))
        handleIndex = EditHandleIndex::kLoop;
    else if (sampleEndHandle.contains (e.getPosition ()))
        handleIndex = EditHandleIndex::kEnd;
    else
        handleIndex = EditHandleIndex::kNone;
    repaint ();
    //DebugLog ("WaveformDisplay", "mouseMove - handleIndex: " + juce::String (handleIndex));
}

void WaveformDisplay::mouseDrag (const juce::MouseEvent& e)
{
    if (audioBuffer == nullptr)
        return;

    switch (handleIndex)
    {
        case EditHandleIndex::kNone:
        {
            LogWaveformDisplay ("mouseDrag - EditHandleIndex::kNone");
            return;
        }
        break;
        case EditHandleIndex::kStart:
        {
            LogWaveformDisplay ("mouseDrag - EditHandleIndex::kStart");
            const auto newSampleStart { static_cast<int64_t> (e.getPosition ().getX () * samplesPerPixel) };
            const auto clampedSampleStart { static_cast<uint32_t> (std::clamp (newSampleStart, static_cast<int64_t> (0), static_cast<int64_t> (cueEnd))) };
            cueStart = clampedSampleStart;
            if (cueStart > cueLoop)
            {
                cueLoop = cueStart;
                sampleLoopMarkerX = 1 + static_cast<int> ((static_cast<float> (cueLoop) / static_cast<float> (numSamples) * numPixels));
                sampleLoopHandle = { sampleLoopMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };
                if (onLoopPointChange != nullptr)
                    onLoopPointChange (cueLoop);
            }
            sampleStartMarkerX = 1 + static_cast<int> ((static_cast<float> (cueStart) / static_cast<float> (numSamples) * numPixels));
            sampleStartHandle = { sampleStartMarkerX, markerStartY, markerHandleSize, markerHandleSize };
            if (onStartPointChange != nullptr)
                onStartPointChange (cueStart);
            repaint ();
        }
        break;
        case EditHandleIndex::kLoop:
        {
            LogWaveformDisplay ("mouseDrag - EditHandleIndex::kLoop");
            const auto newSampleLoop { static_cast<int64_t> (e.getPosition ().getX () * samplesPerPixel) };
            const auto clampedSampleLoop { static_cast<uint32_t> (std::clamp (newSampleLoop, static_cast<int64_t> (cueStart), static_cast<int64_t> (cueEnd))) };
            cueLoop = clampedSampleLoop;
            sampleLoopMarkerX = 1 + static_cast<int> ((static_cast<float> (cueLoop) / static_cast<float> (numSamples) * numPixels));
            sampleLoopHandle = { sampleLoopMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };
            if (onLoopPointChange != nullptr)
                onLoopPointChange (cueLoop);
            repaint ();
        }
        break;
        case EditHandleIndex::kEnd:
        {
            LogWaveformDisplay ("mouseDrag - EditHandleIndex::kEnd - starting cueLoop/cueEnd: " + juce::String (cueLoop) + "/" + juce::String (cueEnd));
            const auto newSampleEnd { static_cast<int64_t> (e.getPosition ().getX () * samplesPerPixel) };
            const auto clampedSampleEnd { static_cast<uint32_t> (std::clamp (newSampleEnd, static_cast<int64_t> (cueStart), static_cast<int64_t> (audioBuffer->getNumSamples ()))) };
            cueEnd = clampedSampleEnd;
            if (cueEnd < cueLoop)
            {
                cueLoop = cueEnd;
                LogWaveformDisplay ("mouseDrag - moving loop: " + juce::String (cueLoop));
                sampleLoopMarkerX = 1 + static_cast<int> ((static_cast<float> (cueLoop) / static_cast<float> (numSamples) * numPixels));
                sampleLoopHandle = { sampleLoopMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };
                if (onLoopPointChange != nullptr)
                    onLoopPointChange (cueLoop);
            }
            LogWaveformDisplay ("mouseDrag - moving end: " + juce::String(cueEnd));
            sampleEndMarkerX = 1 + static_cast<int> ((static_cast<float> (cueEnd) / static_cast<float> (numSamples) * numPixels));
            sampleEndHandle = { sampleEndMarkerX - markerHandleSize, markerStartY, markerHandleSize, markerHandleSize };
            if (onEndPointChange != nullptr)
                onEndPointChange (cueEnd);
            repaint ();
        }
        break;
    }
}
