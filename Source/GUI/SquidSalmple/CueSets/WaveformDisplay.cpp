#include "WaveformDisplay.h"
#include "../../../SystemServices.h"
#include "../../../SquidSalmple/Metadata/SquidSalmpleDefs.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/DumpStack.h"

constexpr auto kMaxSampleLength { 524287 };

#define LOG_WAVEFORM_DISPLAY 1
#if LOG_WAVEFORM_DISPLAY 
#define LogWaveformDisplay(text) DebugLog ("WaveformDisplay", text);
#else
#define LogWaveformDisplay(text) ;
#endif

const auto markerHandleSize { 10 };

void WaveformDisplay::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties { rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no };
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    editManager = systemServices.getEditManager ();

}

void WaveformDisplay::setChannelIndex (int theChannelIndex)
{
    channelIndex = theChannelIndex;
}

void WaveformDisplay::setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer)
{
    LogWaveformDisplay ("setAudioBuffer");
    audioBuffer = theAudioBuffer;
    if (audioBuffer == nullptr)
        numSamples = 0;
    else
        numSamples = audioBuffer->getNumSamples();
    resized ();
    repaint ();
}

void WaveformDisplay::setCueEndPoint (uint32_t newCueEnd)
{
    LogWaveformDisplay ("setCueEndPoint");
    cueEnd = newCueEnd;
    resized ();
    repaint ();
}

void WaveformDisplay::setCueLoopPoint (uint32_t newCueLoop)
{
    LogWaveformDisplay ("setCueEndPoint");
    cueLoop = newCueLoop;
    resized ();
    repaint ();
}

void WaveformDisplay::setCuePoints (uint32_t newCueStart, uint32_t newCueLoop, uint32_t newCueEnd)
{
    LogWaveformDisplay ("setCuePoints");
    cueStart = newCueStart;
    cueLoop = newCueLoop;
    cueEnd = newCueEnd;
    resized ();
    repaint ();
}

void WaveformDisplay::setCueStartPoint (uint32_t newCueStart)
{
    LogWaveformDisplay ("setCueStartPoint");
    cueStart = newCueStart;
    resized ();
    repaint ();
}

void WaveformDisplay::resized ()
{
    LogWaveformDisplay ("resized");
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
    LogWaveformDisplay ("displayWaveform");
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
    LogWaveformDisplay ("displayMarkers");
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

void WaveformDisplay::paintOverChildren (juce::Graphics& g)
{
    // dropMsg
    // dropDetails
    constexpr auto dropMsgFontSizeSingle { 30.f };
    constexpr auto dropMsgFontSizeDouble { 20.f };
    constexpr auto dropDetailsFontSize   { 15.f };
    if (draggingFilesCount > 0)
    {
        jassert (dropType != DropType::none);
        if (audioBuffer == nullptr)
        {
            g.fillAll (juce::Colours::white.withAlpha (0.1f));
            g.setFont (dropMsgFontSizeSingle);
            g.setColour (juce::Colours::black);
            if (draggingFilesCount == 1)
                g.drawText ("Assign sample to Channel " + juce::String(channelIndex + 1), getLocalBounds(), juce::Justification::centred, false);
            else
                g.drawText ("Concatenate samples with Cue Sets and assign to Channel " + juce::String(channelIndex + 1), getLocalBounds(), juce::Justification::centred, false);
        }
        else
        {
            auto displayTextWithBackground = [&g, this] (juce::StringRef text, int fontSize, const juce::Rectangle<int>& bounds)
            {
                g.setFont (fontSize);
                // TODO - replace hardcoded 10.f with value derived from text height
                if (supportedFile)
                    g.setColour (juce::Colours::white.withAlpha (0.7f));
                else
                    g.setColour (juce::Colours::black.withAlpha (0.7f));
                auto stringWidthPixels { g.getCurrentFont ().getStringWidthFloat (text) + 10.f };
                auto center { bounds.getCentre () };
                g.fillRoundedRectangle ({ static_cast<float>(center.getX ()) - (stringWidthPixels / 2.f), static_cast<float>(center.getY ()) - (fontSize / 2.f), stringWidthPixels, fontSize + 5.f }, 10.f);
                if (supportedFile)
                    g.setColour (juce::Colours::black);
                else
                    g.setColour (juce::Colours::red.darker (0.5f));
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
                // display dropMsg and dropDetails
                const auto dropBoundsHalfHeight { dropBounds.getHeight () / 2 };
                auto dropMsgBounds { juce::Rectangle<int> { 0, dropBounds.getY (), dropBounds.getWidth (), dropBoundsHalfHeight }};
                auto dropDetailsBounds { juce::Rectangle<int> { 0, dropBounds.getY () + dropBoundsHalfHeight, dropBounds.getWidth (), dropBounds.getHeight () - dropBoundsHalfHeight } };
                displayTextWithBackground (dropMessage, dropMsgFontSizeDouble, dropMsgBounds);
                displayTextWithBackground (dropDetails, dropDetailsFontSize, dropDetailsBounds);
            }
        }
    }
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
    resetDropInfo ();
    repaint ();
    if (onFilesDropped != nullptr)
        onFilesDropped (files, dropType);
}

void WaveformDisplay::updateDropMessage (const juce::StringArray& files)
{
    auto filesConcatenated { 0 };
    auto sampleTypeMismatch { false };
    auto bitDepthMismatch { false };
    auto channelCountMismatch { false };
    auto sampleRateMismatch { false };
    uint64_t totalSize { 0 };
    dropDetails = {};
    dropMsg = {};
    auto updateDropDetails = [this] (juce::String errorMsg)
    {
        dropDetails += (dropDetails.isNotEmpty () ? ", " : "") + errorMsg;
    };
    supportedFile = true;
    for (auto& fileName : files)
    {
        auto draggedFile { juce::File (fileName) };
        if (draggedFile.getFileExtension () == ".wav")
        {
            if (const auto fileInfo { editManager->getFileInfo (draggedFile) }; fileInfo.supported)
            {
                totalSize += fileInfo.lengthInSamples;
                if (totalSize + fileInfo.lengthInSamples < kMaxSampleLength)
                    ++filesConcatenated;
            }
            else
            {
                updateDropDetails ("Unsupported wav file: " + draggedFile.getFileName () + " [");
                juce::String formatErrors;
                auto updateFormatError = [&formatErrors] (juce::String formatError)
                {
                    formatErrors += (formatErrors.isNotEmpty () ? ", " : "") + formatError;
                };
                if (fileInfo.usesFloatingPointData == true)
                {
                    updateFormatError ("Data type 'float'");
                    sampleTypeMismatch = true;
                }
                if (fileInfo.bitsPerSample != 16 && fileInfo.bitsPerSample != 24)
                {
                    updateFormatError ("Bit Depth '" + juce::String (fileInfo.bitsPerSample) + "'");
                    bitDepthMismatch = true;
                }
                if (fileInfo.numChannels > 2)
                {
                    updateFormatError ("Channel '" + juce::String (fileInfo.numChannels) + "'");
                    channelCountMismatch = true;
                }
                if (fileInfo.sampleRate != 44100)
                {
                    updateFormatError ("Sample Rate '" + juce::String (fileInfo.sampleRate, 2) + "'");
                    sampleRateMismatch = true;
                }
                dropDetails += formatErrors + "]";

                supportedFile = false;
            }
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
                    dropDetails = "Sample will be truncated to 11 seconds";
                }
            }
            else
            {
                dropMsg = "Concatenate samples with Cue Sets and assign to Channel " + juce::String (channelIndex + 1);
                if (totalSize > kMaxSampleLength)
                {
                    // append truncation msg and some msg about how many files will be added, or which ones won't, or, etc
                    // use filesConcatenated value
                    dropDetails = "Only " + juce::String (filesConcatenated) + " samples will fit in 11 seconds. The remaining " + juce::String (files.size () - filesConcatenated)  + " samples will be ignored";
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
                    dropDetails = "They do not fit in the remaining time of " + juce::String ((kMaxSampleLength - audioBuffer->getNumSamples ()) / 44100.f, 2) + " seconds";
                    supportedFile = false;
                }
                else
                {
                    dropMsg = "Samples will be appended and new Cue Sets created for them";
                    dropDetails = "Only " + juce::String (filesConcatenated) + " more samples will fit in 11 seconds. The remaining " + juce::String (files.size () - filesConcatenated) + " samples will be ignored";
                }
            }
        }
    }
    else
    {
        // cannot perform the drop because a file is either an unsupported wav file format, or an unsupported file type
        dropMsg = "Cannot accept files";
        // TODO - still need to display 'dropDetails'
        // TODO - still need to process sampleTypeMismatch, bitDepthMismatch, channelCountMismatch, sampleRateMismatch to produce a proper error msg for dropDetails
    }
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
