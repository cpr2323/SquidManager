#include "LoopPointsView.h"

void LoopPointsView::setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer)
{
    audioBuffer = theAudioBuffer;
}

void LoopPointsView::setLoopPoints (uint32_t theSampleOffset, uint32_t theNumSamples)
{
    sampleOffset = theSampleOffset;
    numSamples = theNumSamples;
}

void LoopPointsView::paint (juce::Graphics& g)
{
    const auto halfWidth { getWidth () / 2 };
    const auto halfHeight { getHeight () / 2 };

    g.setColour (juce::Colours::white);
    // NOTE: Squid Salmple samples can only be 11 seconds long, so we use a uint32_t to store offsets and length
    if (audioBuffer != nullptr && static_cast<uint32_t> (audioBuffer->getNumSamples ()) >= numSamples && numSamples > 4)
    {
        juce::dsp::AudioBlock<float> audioBlock { *audioBuffer };
        juce::dsp::AudioBlock<float> loopSamples { audioBlock.getSubBlock (sampleOffset, numSamples) };
        const auto samplesToDisplay { static_cast<int> (std::min<juce::int64> (numSamples, halfWidth)) };

        g.setColour (juce::Colours::lightgrey);
        const auto dashSize { getHeight () / 11.f };
        std::array<float, 2> dashedSpec { dashSize, dashSize };
        g.drawDashedLine (juce::Line<int>{ 0, halfHeight, getWidth (), halfHeight }.toFloat (), dashedSpec.data (), 2);

        g.setColour (juce::Colours::white);
        auto readPtr { loopSamples.getChannelPointer (0) };
        for (auto sampleCount { 0 }; sampleCount < samplesToDisplay - 1; ++sampleCount)
        {
            // draw one line of sample going reverse from middle to left
            const auto xOffset { halfWidth - sampleCount };
            const auto sampleIndex { numSamples - sampleCount };
            g.drawLine (static_cast<float> (xOffset),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [sampleIndex] * halfHeight))),
                        static_cast<float> (xOffset + 1),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [sampleIndex + 1] * halfHeight))));

            // draw one line of sample start going from middle to right
            g.drawLine (static_cast<float> (halfWidth + sampleCount),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [sampleCount] * halfHeight))),
                        static_cast<float> (halfWidth + sampleCount + 1),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [sampleCount + 1] * halfHeight))));
        }
    }
    else
    {
        //jassertfalse;
    }

    g.drawRect (getLocalBounds ());
    g.fillRect (getWidth () / 2, 0, 1, getHeight ());
}
