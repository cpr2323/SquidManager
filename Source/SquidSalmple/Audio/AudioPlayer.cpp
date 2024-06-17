#include "AudioPlayer.h"
//#include "../../Assimil8or/PresetManagerProperties.h"
#include "../Bank/BankManagerProperties.h"
#include "../../Utility/DebugLog.h"
#include "../../Utility/PersistentRootProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

#define LOG_AUDIO_PLAYER 0
#if LOG_AUDIO_PLAYER
#define LogAudioPlayer(text) DebugLog ("AudioPlayer", text);
#else
#define LogAudioPlayer(text) ;
#endif

AudioPlayer::AudioPlayer ()
{
    audioFormatManager.registerBasicFormats ();
}

void AudioPlayer::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    BankManagerProperties bankManagerProperties (runtimeRootProperties.getValueTree (), BankManagerProperties::WrapperType::owner, BankManagerProperties::EnableCallbacks::no);
    bankProperties.wrap (bankManagerProperties.getBank ("edit"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);

    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::owner, AppProperties::EnableCallbacks::yes);

    audioSettingsProperties.wrap (persistentRootProperties.getValueTree (), AudioSettingsProperties::WrapperType::owner, AudioSettingsProperties::EnableCallbacks::yes);
    audioSettingsProperties.onConfigChange = [this] (juce::String config)
    {
            // TODO - do we need this callback?
        //configureAudioDevice (deviceName);
    };

    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::owner, AudioPlayerProperties::EnableCallbacks::yes);
    audioPlayerProperties.onShowConfigDialog = [this] () { showConfigDialog (); };
    audioPlayerProperties.onPlayStateChange = [this] (AudioPlayerProperties::PlayState newPlayState)
    {
        LogAudioPlayer ("init: audioPlayerProperties.onPlayStateChange");
        handlePlayState (newPlayState);
    };
    // Clients call this to setup the sample source
    audioPlayerProperties.onSampleSourceChanged = [this] (int channelIndex)
    {
        LogAudioPlayer ("init: audioPlayerProperties.onSampleSourceChanged");
        initFromChannel (channelIndex);
    };
    audioDeviceManager.addChangeListener (this);
    configureAudioDevice (audioSettingsProperties.getConfig ());
}

void AudioPlayer::initFromChannel (int channelIndex)
{
    LogAudioPlayer ("initFromChannel");
    jassert (channelIndex < 8);
    channelProperties.wrap (bankProperties.getChannelVT (channelIndex), SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
    sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (channelIndex), SampleProperties::WrapperType::owner, SampleProperties::EnableCallbacks::yes);

    channelProperties.onFileNameChange = [this] (juce::String fileNameName)
    {
        LogAudioPlayer ("zoneProperties.onSampleChange");
        initSamplePoints ();
        prepareSampleForPlayback ();
    };
    // TODO - can I refactor these callback?
    channelProperties.onStartCueChange = [this] (uint32_t newSampleStart)
    {
        LogAudioPlayer ("channelProperties.onStartCueChange ");
        jassert (sampleRateRatio > 0.0);
        juce::ScopedLock sl (dataCS);
        sampleStart = static_cast<int> (newSampleStart * sampleRateRatio);
        sampleLength = static_cast<int> ((channelProperties.getEndCue () - newSampleStart) * sampleRateRatio);
        if (curSampleOffset < sampleStart || curSampleOffset >= sampleStart + sampleLength)
            curSampleOffset = sampleStart;
        LogAudioPlayer ("channelProperties.onStartCueChange - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
    };
    channelProperties.onEndCueChange = [this] (uint32_t newSampleEnd)
    {
        LogAudioPlayer ("channelProperties.onEndCueChange ");
        jassert (sampleRateRatio > 0.0);
        juce::ScopedLock sl (dataCS);
        sampleLength = static_cast<int> ((newSampleEnd - channelProperties.getStartCue()) * sampleRateRatio);
        loopLength = static_cast<int> ((newSampleEnd - channelProperties.getLoopCue()) * sampleRateRatio);
        if (curSampleOffset >= sampleStart + sampleLength)
            curSampleOffset = sampleStart;
        LogAudioPlayer ("channelProperties.onEndCueChange - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
    };
    channelProperties.onLoopCueChange = [this] (uint32_t newLoopStart)
    {
        LogAudioPlayer ("channelProperties.onLoopCueChange ");
        jassert (sampleRateRatio > 0.0);
        juce::ScopedLock sl (dataCS);
        loopStart = static_cast<int> (newLoopStart * sampleRateRatio);
        loopLength = static_cast<int> ((channelProperties.getEndCue() - newLoopStart) * sampleRateRatio);
        if (curSampleOffset < sampleStart || curSampleOffset >= sampleStart + sampleLength)
            curSampleOffset = sampleStart;
        LogAudioPlayer ("channelProperties.onLoopCueChange  - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
    };
    sampleProperties.onStatusChange = [this] (SampleStatus status)
    {
        if (status == SampleStatus::exists)
        {
            LogAudioPlayer ("sampleProperties.onStatusChange: SampleStatus::exists");
            initSamplePoints ();
            prepareSampleForPlayback ();
        }
        else
        {
            // TODO - reset somethings?
            LogAudioPlayer ("sampleProperties.onStatusChange: NOT SampleStatus::exists");
            audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, true);
            {
                juce::ScopedLock sl (dataCS);
                sampleStart = 0;
                sampleLength = 0;
                sampleBuffer.reset ();
            }
        }
    };

    // create local copy of audio data, with resampling if needed
    prepareSampleForPlayback ();

    // setup local sample start and sample length based on samplePointsSource
    initSamplePoints ();
}

void AudioPlayer::initSamplePoints ()
{
    LogAudioPlayer ("initSamplePoints");
    jassert (sampleRateRatio > 0.0);
    juce::ScopedLock sl (dataCS);
    sampleStart = static_cast<int> (channelProperties.getStartCue() * sampleRateRatio);
    sampleLength = static_cast<int> ((channelProperties.getEndCue () - channelProperties.getStartCue ()) * sampleRateRatio);
    loopStart = static_cast<int> (channelProperties.getLoopCue () * sampleRateRatio);
    loopLength = static_cast<int> ((channelProperties.getEndCue () - channelProperties.getLoopCue ()) * sampleRateRatio);

    if (curSampleOffset < sampleStart || curSampleOffset >= sampleStart + sampleLength)
        curSampleOffset = sampleStart;
    LogAudioPlayer ("AudioPlayer::initSamplePoints - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
}

void AudioPlayer::prepareSampleForPlayback ()
{
    jassert (playState == AudioPlayerProperties::PlayState::stop);
    if (channelProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists)
    {
        LogAudioPlayer ("prepareSampleForPlayback: sample is ready");
        std::unique_ptr <juce::MemoryAudioSource> readerSource { std::make_unique<juce::MemoryAudioSource> (*sampleProperties.getAudioBufferPtr (), false, false) };
        std::unique_ptr<juce::ResamplingAudioSource> resamplingAudioSource { std::make_unique<juce::ResamplingAudioSource> (readerSource.get (), false, 2) };
        sampleRateRatio = sampleRate / sampleProperties.getSampleRate ();
        resamplingAudioSource->setResamplingRatio (sampleProperties.getSampleRate () / sampleRate);
        resamplingAudioSource->prepareToPlay (blockSize, sampleRate);
        sampleBuffer = std::make_unique<juce::AudioBuffer<float>> (sampleProperties.getNumChannels (), static_cast<int> (sampleProperties.getLengthInSamples () * sampleRate / sampleProperties.getSampleRate ()));
        resamplingAudioSource->getNextAudioBlock (juce::AudioSourceChannelInfo (*sampleBuffer.get ()));
        curSampleOffset = 0;
    }
    else
    {
        LogAudioPlayer ("prepareSampleForPlayback: sample is NOT ready");
    }
}

void AudioPlayer::shutdownAudio ()
{
    audioSourcePlayer.setSource (nullptr);
    audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
    audioDeviceManager.closeAudioDevice ();
}

void AudioPlayer::configureAudioDevice (juce::String config)
{
    juce::String audioConfigError;
    if (config.isEmpty ())
    {
        audioConfigError = audioDeviceManager.initialise (0, 2, nullptr, true);
    }
    else
    {
        auto audioConfigXml { juce::XmlDocument::parse (config) };
        audioConfigError = audioDeviceManager.initialise (0, 2, audioConfigXml.get (), true, {}, nullptr);
    }

    if (! audioConfigError.isEmpty ())
    {
        jassertfalse;
    }

    audioDeviceManager.addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (this);
}

void AudioPlayer::handlePlayState (AudioPlayerProperties::PlayState newPlayState)
{
    juce::ScopedLock sl (dataCS);
    if (newPlayState == AudioPlayerProperties::PlayState::stop)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: stop");
    }
    else if (newPlayState == AudioPlayerProperties::PlayState::loop)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: loop");
        curSampleOffset = sampleStart;
    }
    else if (newPlayState == AudioPlayerProperties::PlayState::play)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: play");
        curSampleOffset = sampleStart;
    }
    playState = newPlayState;
}

void AudioPlayer::showConfigDialog ()
{
    juce::DialogWindow::LaunchOptions o;
    o.escapeKeyTriggersCloseButton = true;
    o.dialogBackgroundColour = juce::Colours::grey;
    o.dialogTitle = "AUDIO SETTTINGS";
    audioSetupComp.setBounds (0, 0, 400, 600);
    o.content.set (&audioSetupComp, false);
    o.launchAsync ();
}

void AudioPlayer::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    LogAudioPlayer ("prepareToPlay");
    sampleRate = newSampleRate;
    blockSize = samplesPerBlockExpected;
    prepareSampleForPlayback ();
}

void AudioPlayer::releaseResources ()
{
}

void AudioPlayer::changeListenerCallback (juce::ChangeBroadcaster*)
{
    LogAudioPlayer ("audio device settings changed");
    auto audioDeviceSettings { audioDeviceManager.createStateXml () };
    if (audioDeviceSettings != nullptr)
    {
        auto xmlString { audioDeviceSettings->toString () };
        audioSettingsProperties.setConfig (xmlString, false);
    }
}

void AudioPlayer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion ();
    // fill buffer with data

    if (playState == AudioPlayerProperties::PlayState::stop)
        return;

    const auto numOutputSamples { bufferToFill.numSamples };
    auto& outputBuffer { *bufferToFill.buffer };
    const auto channels { juce::jmin (outputBuffer.getNumChannels (), sampleBuffer->getNumChannels ()) };

    auto originalSampleOffset { 0 };
    auto cachedSampleLength { 0 };
    auto cachedSampleStart { 0 };
    auto cachedLocalSampleOffset { 0 };
    auto chachedPlayState { AudioPlayerProperties::PlayState::stop };
    {
        // NOTE: I am using a lock in the audio callback ONLY BECAUSE the audio play back is a simple audition feature, not recording or performance playback
        juce::ScopedLock sl (dataCS);
        jassert (curSampleOffset >= sampleStart);
        jassert (curSampleOffset < sampleStart + sampleLength);
        originalSampleOffset =  curSampleOffset; // should be >= sampleStart and < sampleStart + sampleLength
        cachedSampleLength = sampleLength;
        cachedSampleStart = sampleStart;
        chachedPlayState = playState;
        cachedLocalSampleOffset = curSampleOffset - sampleStart;
        LogAudioPlayer ("AudioPlayer::getNextAudioBlock - cachedSampleStart: " + juce::String (cachedSampleStart) + ", chachedSampleLength: " + juce::String (cachedSampleLength) +
                        ", curSampleOffset: " + juce::String (curSampleOffset) + ", cachedLocalSampleOffset: " + juce::String (cachedLocalSampleOffset));
    }
    auto numSamplesToCopy { 0 };
    auto outputBufferWritePos { 0 };
    while (cachedSampleLength > 0 && outputBufferWritePos < numOutputSamples)
    {
        numSamplesToCopy = juce::jmin (numOutputSamples - outputBufferWritePos, cachedSampleLength - cachedLocalSampleOffset);
        LogAudioPlayer ("AudioPlayer::getNextAudioBlock - numSamplesToCopy: " + juce::String (numSamplesToCopy));
        jassert (numSamplesToCopy >= 0);

        // copy data from sample buffer to output buffer, this may, or may not, fill the entire output buffer
        auto ch { 0 };
        for (; ch < channels; ++ch)
            outputBuffer.copyFrom (ch, bufferToFill.startSample + outputBufferWritePos, *sampleBuffer, ch, cachedSampleStart + cachedLocalSampleOffset, numSamplesToCopy);

        // clear any unused channels
        for (; ch < outputBuffer.getNumChannels (); ++ch)
            outputBuffer.clear (ch, bufferToFill.startSample + outputBufferWritePos, numSamplesToCopy);

        outputBufferWritePos += numSamplesToCopy;
        cachedLocalSampleOffset += numSamplesToCopy;
        if (chachedPlayState == AudioPlayerProperties::PlayState::loop)
        {
            if (cachedLocalSampleOffset >= cachedSampleLength)
                cachedLocalSampleOffset = 0;
        }
        else
        {
            LogAudioPlayer ("AudioPlayer::getNextAudioBlock - outputBufferWritePos : " + juce::String (outputBufferWritePos) + ", numOutputSamples: " + juce::String (numOutputSamples));
            if (outputBufferWritePos < numOutputSamples)
            {
                outputBuffer.clear (bufferToFill.startSample + outputBufferWritePos, numOutputSamples - outputBufferWritePos);
                audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, true);
                break;
            }
        }
    }
    {
        // NOTE: I am using a lock in the audio callback ONLY BECAUSE the audio play back is a simple audition feature, not recording or performance playback
        juce::ScopedLock sl (dataCS);
        if (originalSampleOffset == curSampleOffset && cachedSampleStart == sampleStart && cachedSampleLength == sampleLength) // if the offset has not changed externally
        {
            curSampleOffset = cachedSampleStart + cachedLocalSampleOffset;
            LogAudioPlayer ("AudioPlayer::getNextAudioBlock setting new curSampleOffset: " + juce::String (curSampleOffset) + ", cachedSampleStart: " + juce::String (cachedSampleStart) + ", chachedSampleLength: " + juce::String (cachedSampleLength) +
                            ", cachedLocalSampleOffset: " + juce::String (cachedLocalSampleOffset));
        }
        else
        {
            LogAudioPlayer ("AudioPlayer::getNextAudioBlock - curSampleOffset: " + juce::String (cachedSampleStart) + " != originalSampleOffset: " + juce::String (originalSampleOffset));
        }
    }
}
