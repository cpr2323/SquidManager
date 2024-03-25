#include "WaveformDisplay.h"

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

    jassert (numSamples > 0);
    cueSets [0].start = 0;
    cueSets [0].loop = numSamples / 2;
    cueSets [0].end = numSamples - 1;
    numCueSets = 1;
    setCurCue (0);
}

void WaveformDisplay::setCurCue (int cueSetIndex)
{
    if (numCueSets > 0 && cueSetIndex < numCueSets)
    {
        curCueSet = cueSetIndex;

        resized ();
        repaint ();
    }
}

void WaveformDisplay::updateData ()
{
    if (audioBufferPtr != nullptr)
    {
//         numSamples  = sampleProperties.getLengthInSamples ();
//         sampleStart = zoneProperties.getSampleStart ().value_or (0);
//         sampleEnd = zoneProperties.getSampleEnd ().value_or (numSamples);
//         sampleLoop = zoneProperties.getLoopStart ().value_or (0);
        samplesPerPixel = static_cast<int> (numSamples / getWidth ());
    }
}

void WaveformDisplay::resized ()
{
    halfHeight = getHeight () / 2;
    numPixels = getWidth () - 2;
    samplesPerPixel = static_cast<int> (numSamples / getWidth ());
    markerEndY = getHeight () - 2;
    const auto dashSize { getHeight () / 11.f };
    dashedSpec = { dashSize, dashSize };

    const auto markerHandleSize { 5 };

    // draw sample start marker
    sampleStartMarkerX = 1 + static_cast<int> ((static_cast<float> (cueSets [curCueSet].start) / static_cast<float> (numSamples) * numPixels));
    sampleStartHandle = { sampleStartMarkerX, markerStartY, markerHandleSize, markerHandleSize };

    // draw loop start marker
    sampleLoopMarkerX = 1 + static_cast<int> ((static_cast<float> (cueSets [curCueSet].loop) / static_cast<float> (numSamples) * numPixels));
    sampleLoopHandle = { sampleLoopMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };

    // draw sample end marker
    sampleEndMarkerX = 1 + static_cast<int> ((static_cast<float> (cueSets [curCueSet].end) / static_cast<float> (numSamples) * numPixels));
    sampleEndHandle = { sampleEndMarkerX - markerHandleSize, markerStartY, markerHandleSize, markerHandleSize };

    samplesPerPixel = static_cast<int> (numSamples / getWidth ());
}

void WaveformDisplay::displayWaveform (juce::Graphics& g)
{
    if (audioBufferPtr != nullptr)
    {
        // TODO - implement side selection
        auto readPtr { audioBufferPtr->getReadPointer (0) };

        g.setColour (juce::Colours::black);
        // TODO - get proper end pixel if sample ends before end of display
        for (auto pixelIndex { 0 }; pixelIndex < numPixels - 1; ++pixelIndex)
        {
            if ((pixelIndex + 1) * samplesPerPixel < numSamples)
            {
                const auto pixelOffset { pixelIndex + 1 };
                g.drawLine (static_cast<float> (pixelOffset),
                    static_cast<float> (static_cast<int> (halfHeight + (readPtr [pixelIndex * samplesPerPixel] * halfHeight))),
                    static_cast<float> (pixelOffset + 1),
                    static_cast<float> (static_cast<int> (halfHeight + (readPtr [(pixelIndex + 1) * samplesPerPixel] * halfHeight))));
            }
            else
            {
                break;
            }
        }
    }
}

void WaveformDisplay::displayMarkers (juce::Graphics& g)
{
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
    if (audioBufferPtr != nullptr)
    {
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
}

void WaveformDisplay::mouseDown ([[maybe_unused]] const juce::MouseEvent& e)
{
    if (handleIndex == EditHandleIndex::kNone)
        return;
}

void WaveformDisplay::mouseDrag (const juce::MouseEvent& e)
{
    switch (handleIndex)
    {
        case EditHandleIndex::kNone:
        {
            return;
        }
        break;
        case EditHandleIndex::kStart:
        {
            //const auto newSampleStart { static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel) };
            //const auto clampedSampleStart { std::clamp (newSampleStart, static_cast<juce::int64> (0), zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()) - 1) };
            //zoneProperties.setSampleStart (clampedSampleStart == 0 ? -1 : clampedSampleStart, true);
        }
        break;
        case EditHandleIndex::kLoop:
        {
            //const auto newLoopLength { static_cast<double> ((e.getPosition ().getX () * samplesPerPixel) - zoneProperties.getLoopStart ().value_or (0)) };
            //const auto clampedLoopLength { std::clamp (newLoopLength, 4.0, static_cast<double> (sampleProperties.getLengthInSamples () - zoneProperties.getLoopStart ().value_or (0))) };
            //zoneProperties.setLoopLength (clampedLoopLength == sampleProperties.getLengthInSamples () ? -1.0 : clampedLoopLength, true);
        }
        break;
        case EditHandleIndex::kEnd:
        {
            //const auto newSampleEnd { static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel) };
            //const auto clampedSampleEnd { std::clamp (newSampleEnd, zoneProperties.getSampleStart ().value_or (0) + 1, sampleProperties.getLengthInSamples ()) };
            //zoneProperties.setSampleEnd (clampedSampleEnd == sampleProperties.getLengthInSamples () ? -1 : clampedSampleEnd, true);
        }
        break;
    }
}
