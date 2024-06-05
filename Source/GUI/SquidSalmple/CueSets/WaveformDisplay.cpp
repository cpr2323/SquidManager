#include "WaveformDisplay.h"
#include "../../../Utility/DebugLog.h"

const auto markerHandleSize { 5 };

WaveformDisplay::WaveformDisplay ()
{
    audioFormatManager.registerBasicFormats ();
}

void WaveformDisplay::init (juce::File theTestFile)
{
    testFile = theTestFile;

    if (std::unique_ptr<juce::AudioFormatReader> sampleFileReader { audioFormatManager.createReaderFor (testFile) }; sampleFileReader != nullptr)
    {
        numSamples = sampleFileReader->lengthInSamples;
        DebugLog ("WaveformDisplay", "init [" + testFile.getFileName () + "] - numSamples = " + juce::String (numSamples).paddedLeft ('0', 6) +
                  " [0x" + juce::String::toHexString (numSamples).paddedLeft ('0', 6) + "], bitDepth = " + juce::String (sampleFileReader->bitsPerSample) +
                  ", channels = " + juce::String(sampleFileReader->numChannels) +
                  ", sampleRate = " + juce::String (sampleFileReader->sampleRate));

        // read in audio data
        audioBuffer.setSize (sampleFileReader->numChannels, static_cast<int> (numSamples), false, true, false);
        sampleFileReader->read (&audioBuffer, 0, static_cast<int> (numSamples), 0, true, false);
        audioBufferPtr = &audioBuffer;
    }
    else
    {
        audioBufferPtr = nullptr;
        jassertfalse;
    }
}

void WaveformDisplay::setCuePoints (juce::int64 newCueStart, juce::int64 newCueLoop, juce::int64 newCueEnd)
{
    cueStart = newCueStart;
    cueLoop = newCueLoop;
    cueEnd = newCueEnd;
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

    if (audioBufferPtr == nullptr)
    {
        samplesPerPixel = 0;
        sampleStartMarkerX = 0;
        sampleLoopMarkerX = 0;
        sampleEndMarkerX = 0;
    }
    else
    {
        samplesPerPixel = static_cast<int> (numSamples / getWidth ());
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
    if (audioBufferPtr == nullptr)
        return;
    // TODO - implement side selection
    auto readPtr { audioBufferPtr->getReadPointer (0) };

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
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [pixelIndex * samplesPerPixel] * halfHeight))),
                        static_cast<float> (pixelOffset + 1),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [(pixelIndex + 1) * samplesPerPixel] * halfHeight))));

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
    if (audioBufferPtr == nullptr)
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
    if (audioBufferPtr == nullptr)
        return;

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
    if (audioBufferPtr == nullptr)
        return;

    switch (handleIndex)
    {
        case EditHandleIndex::kNone:
        {
            return;
        }
        break;
        case EditHandleIndex::kStart:
        {
            const auto newSampleStart { static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel) };
            const auto clampedSampleStart { std::clamp (newSampleStart, static_cast<juce::int64> (0), static_cast<juce::int64> (cueEnd)) };
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
            const auto newLoop { static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel) };
            const auto clampedLoopLength { std::clamp (newLoop, static_cast<juce::int64> (cueStart), static_cast<juce::int64> (cueEnd)) };
            cueLoop = clampedLoopLength;
            sampleLoopMarkerX = 1 + static_cast<int> ((static_cast<float> (cueLoop) / static_cast<float> (numSamples) * numPixels));
            sampleLoopHandle = { sampleLoopMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };
            if (onLoopPointChange != nullptr)
                onLoopPointChange (cueLoop);
            repaint ();
        }
        break;
        case EditHandleIndex::kEnd:
        {
            const auto newSampleEnd { static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel) };
            const auto clampedSampleEnd{ std::clamp (newSampleEnd, static_cast<juce::int64> (cueStart), static_cast<juce::int64> (audioBufferPtr->getNumSamples ())) };
            cueEnd = clampedSampleEnd;
            if (cueEnd < cueLoop)
            {
                cueLoop = cueEnd;
                sampleLoopMarkerX = 1 + static_cast<int> ((static_cast<float> (cueLoop) / static_cast<float> (numSamples) * numPixels));
                sampleLoopHandle = { sampleLoopMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };
                if (onLoopPointChange != nullptr)
                    onLoopPointChange (cueLoop);
            }
            sampleEndMarkerX = 1 + static_cast<int> ((static_cast<float> (cueEnd) / static_cast<float> (numSamples) * numPixels));
            sampleEndHandle = { sampleEndMarkerX - markerHandleSize, markerStartY, markerHandleSize, markerHandleSize };
            if (onEndPointChange != nullptr)
                onEndPointChange (cueEnd);
            repaint ();
        }
        break;
    }
}
