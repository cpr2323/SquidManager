#include "LoopPointsView.h"
#include "../../Theme/SquidColourIds.h"
#include "../../Theme/SquidFonts.h"
#include "../../../SystemServices.h"

void LoopPointsView::init (juce::ValueTree squidChannelPropertiesVT, juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties { rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no };
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();
    jassert (editManager != nullptr);
    squidChannelProperties.wrap (squidChannelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);
}

void LoopPointsView::setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer)
{
    audioBuffer = theAudioBuffer;
}

void LoopPointsView::setLoopPoints (uint32_t theSampleOffset, uint32_t theNumSamples)
{
    sampleOffset = theSampleOffset;
    numSamples = theNumSamples;
}

void LoopPointsView::mouseDown (const juce::MouseEvent& event)
{
    if (! event.mods.isPopupMenu ())
        return;

    enum WhichEndOfLoop { loopCue, endCue };
    WhichEndOfLoop whichEndOfLoop { event.getMouseDownPosition ().getX () < getWidth () / 2 ? WhichEndOfLoop::endCue : WhichEndOfLoop::loopCue };
    juce::PopupMenu loopTunerMenu;
    loopTunerMenu.addSectionHeader ("Sample " + juce::String (whichEndOfLoop == WhichEndOfLoop::endCue ? "End" : "Loop"));
    loopTunerMenu.addSeparator ();
    {
        juce::PopupMenu zeroCrossingMenuOptions;
        if (whichEndOfLoop == WhichEndOfLoop::endCue)
        {
            zeroCrossingMenuOptions.addItem ("Left  <<", true, false, [this] ()
            {
                auto newEndCue { editManager->findPreviousZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()),
                                                                        SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()),
                                                                        *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newEndCue != -1)
                    squidChannelProperties.setEndCue (SquidChannelProperties::sampleOffsetToByteOffset (newEndCue), true);
            });
            zeroCrossingMenuOptions.addItem ("Right >>", true, false, [this] ()
            {
                auto newEndCue { editManager->findNextZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()),
                                                                    squidChannelProperties.getSampleDataNumSamples (),
                                                                    *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newEndCue != -1)
                    squidChannelProperties.setEndCue (SquidChannelProperties::sampleOffsetToByteOffset (newEndCue), true);
            });
        }
        else
        {
            zeroCrossingMenuOptions.addItem ("Left  <<", true, false, [this] ()
            {
                auto newLoopCue { editManager->findPreviousZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ()),
                                                                        SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ()),
                                                                        *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newLoopCue != -1)
                    squidChannelProperties.setLoopCue (SquidChannelProperties::sampleOffsetToByteOffset (newLoopCue), true);
            });
            zeroCrossingMenuOptions.addItem ("Right >>", true, false, [this] ()
            {
                auto newLoopCue { editManager->findNextZeroCrossing (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ()),
                                                                    SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()),
                                                                    *squidChannelProperties.getSampleDataAudioBuffer ().get ()->getAudioBuffer ()) };
                if (newLoopCue != -1)
                    squidChannelProperties.setLoopCue (SquidChannelProperties::sampleOffsetToByteOffset (newLoopCue), true);
            });
        }
        loopTunerMenu.addSubMenu ("Zero Crossing", zeroCrossingMenuOptions);
    }
    loopTunerMenu.showMenuAsync ({});
}

void LoopPointsView::paint (juce::Graphics& g)
{
    const auto halfWidth { getWidth () / 2 };
    const auto halfHeight { getHeight () / 2 };

    const auto area { getLocalBounds ().toFloat () };
    g.setColour (findColour (SquidColours::tunerBackground));
    g.fillRoundedRectangle (area, 2.0f);

    // the zero line is there whether or not there is audio to hang on it
    g.setColour (findColour (SquidColours::tunerDash));
    const auto dashSize { getHeight () / 11.f };
    std::array<float, 2> dashedSpec { dashSize, dashSize };
    g.drawDashedLine (juce::Line<int>{ 0, halfHeight, getWidth (), halfHeight }.toFloat (), dashedSpec.data (), 2);
    // NOTE: Squid Salmple samples can only be 11 seconds long, so we use a uint32_t to store offsets and length
    if (audioBuffer != nullptr && static_cast<uint32_t> (audioBuffer->getNumSamples ()) >= numSamples && numSamples > 4)
    {
        juce::dsp::AudioBlock<float> audioBlock { *audioBuffer };
        juce::dsp::AudioBlock<float> loopSamples { audioBlock.getSubBlock (sampleOffset, numSamples) };
        const auto samplesToDisplay { static_cast<int> (std::min<juce::int64> (numSamples, halfWidth)) };

        g.setColour (findColour (SquidColours::waveformForeground));
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

    // where the end of the loop meets its start
    g.setColour (findColour (SquidColours::tunerDivider));
    g.fillRect (halfWidth, 0, 1, getHeight ());

    // which side is which, along the bottom either side of the divider
    constexpr auto kCaptionGap { 7 };
    constexpr auto kCaptionHeight { 13 };
    const auto captionRow { getLocalBounds ().removeFromBottom (kCaptionHeight + 1).withTrimmedBottom (1) };
    g.setFont (SquidType::caption ());
    g.setColour (findColour (SquidColours::tunerCaption));
    g.drawText ("END", captionRow.withRight (halfWidth - kCaptionGap), juce::Justification::centredRight, false);
    g.drawText ("LOOP", captionRow.withLeft (halfWidth + kCaptionGap + 1), juce::Justification::centredLeft, false);

    // set explicitly: these used to inherit whatever colour the trace left behind
    g.setColour (findColour (SquidColours::outline));
    g.drawRoundedRectangle (area.reduced (0.5f), 2.0f, 1.0f);
}
