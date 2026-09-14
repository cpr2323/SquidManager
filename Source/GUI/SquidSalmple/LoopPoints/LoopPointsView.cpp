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

namespace
{
    // the tuner draws one sample per pixel, so a pixel of drag is a sample; with
    // Shift held it is this many, for covering a long sample quickly
    constexpr auto kCoarseDragSamplesPerPixel { 10 };
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
    if (event.mods.isPopupMenu ())
    {
        showLoopTunerMenu (event);
        return;
    }

    dragTarget = DragTarget::none;
    if (audioBuffer == nullptr || squidChannelProperties.getSampleDataAudioBuffer () == nullptr)
        return;

    dragTarget = event.x < getWidth () / 2 ? DragTarget::endCue : DragTarget::loopCue;
    dragStartCue = static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (dragTarget == DragTarget::endCue ? squidChannelProperties.getEndCue ()
                                                                                                                     : squidChannelProperties.getLoopCue ()));
    dragAnchorX = event.x;
    dragWasCoarse = event.mods.isShiftDown ();
    setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
}

void LoopPointsView::mouseDrag (const juce::MouseEvent& event)
{
    if (dragTarget == DragTarget::none)
        return;

    // Changing between fine and coarse part way through re-anchors the drag, so the
    // cue carries on from where it is rather than jumping.
    if (event.mods.isShiftDown () != dragWasCoarse)
    {
        dragStartCue = static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (dragTarget == DragTarget::endCue ? squidChannelProperties.getEndCue ()
                                                                                                                         : squidChannelProperties.getLoopCue ()));
        dragAnchorX = event.x;
        dragWasCoarse = event.mods.isShiftDown ();
    }

    // The audio is grabbed and slid, as the main waveform is panned: moving right
    // brings earlier audio to the divider, so the cue moves earlier.
    const auto samplesPerPixel { dragWasCoarse ? kCoarseDragSamplesPerPixel : 1 };
    const auto newCue { dragStartCue - ((event.x - dragAnchorX) * samplesPerPixel) };
    if (dragTarget == DragTarget::endCue)
        setEndCueFromDrag (newCue);
    else
        setLoopCueFromDrag (newCue);
}

void LoopPointsView::mouseUp (const juce::MouseEvent&)
{
    dragTarget = DragTarget::none;
    setMouseCursor (juce::MouseCursor::NormalCursor);
}

// The same rules the cue fields and the waveform markers apply: the end stays between
// the start and the end of the sample, and pulls the loop back with it if it is dragged
// past it; the loop stays between the start and the end.
void LoopPointsView::setEndCueFromDrag (int newEndCue)
{
    const auto startCue { static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ())) };
    const auto sampleLength { static_cast<int> (squidChannelProperties.getSampleDataNumSamples ()) };
    const auto endCue { std::clamp (newEndCue, startCue, std::max (startCue, sampleLength)) };
    if (static_cast<uint32_t> (endCue) == SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ()))
        return;

    const auto cueSetIndex { squidChannelProperties.getCurCueSet () };
    if (endCue < static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ())))
    {
        const auto loopByteOffset { SquidChannelProperties::sampleOffsetToByteOffset (static_cast<uint32_t> (endCue)) };
        squidChannelProperties.setCueSetLoopPoint (cueSetIndex, loopByteOffset);
        squidChannelProperties.setLoopCue (loopByteOffset, false);
    }
    const auto endByteOffset { SquidChannelProperties::sampleOffsetToByteOffset (static_cast<uint32_t> (endCue)) };
    squidChannelProperties.setCueSetEndPoint (cueSetIndex, endByteOffset);
    squidChannelProperties.setEndCue (endByteOffset, false);
}

void LoopPointsView::setLoopCueFromDrag (int newLoopCue)
{
    const auto startCue { static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getStartCue ())) };
    const auto endCue { static_cast<int> (SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getEndCue ())) };
    const auto loopCue { std::clamp (newLoopCue, startCue, std::max (startCue, endCue)) };
    if (static_cast<uint32_t> (loopCue) == SquidChannelProperties::byteOffsetToSampleOffset (squidChannelProperties.getLoopCue ()))
        return;

    const auto loopByteOffset { SquidChannelProperties::sampleOffsetToByteOffset (static_cast<uint32_t> (loopCue)) };
    squidChannelProperties.setCueSetLoopPoint (squidChannelProperties.getCurCueSet (), loopByteOffset);
    squidChannelProperties.setLoopCue (loopByteOffset, false);
}

void LoopPointsView::showLoopTunerMenu (const juce::MouseEvent& event)
{
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
