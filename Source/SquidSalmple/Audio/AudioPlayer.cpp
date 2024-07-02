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

void AudioPlayer::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    BankManagerProperties bankManagerProperties (runtimeRootProperties.getValueTree (), BankManagerProperties::WrapperType::owner, BankManagerProperties::EnableCallbacks::no);
    bankProperties.wrap (bankManagerProperties.getBank ("edit"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);

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
    audioPlayerProperties.onPlayModeChange = [this] (AudioPlayerProperties::PlayMode newPlayMode)
    {
        LogAudioPlayer ("init: audioPlayerProperties.onPlayStateChange");
        handlePlayMode (newPlayMode);
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
    channelProperties.wrap (bankProperties.getChannelVT (channelIndex), SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);

    channelProperties.onSampleDataAudioBufferChange = [this] (AudioBufferRefCounted::RefCountedPtr)
    {
        LogAudioPlayer ("channelProperties.onSampleDataAudioBufferChange");
        initSamplePoints ();
        prepareSampleForPlayback ();
    };
    // TODO - can I refactor these callback?
    channelProperties.onStartCueChange = [this] (uint32_t newSampleStart)
    {
        LogAudioPlayer ("channelProperties.onStartCueChange ");
        jassert (sampleRateRatio > 0.0);
        const auto actualSampleStart { newSampleStart / 2 };
        juce::ScopedLock sl (dataCS);
        if (playMode == AudioPlayerProperties::PlayMode::once)
        {
            sampleStart = static_cast<int> (actualSampleStart * sampleRateRatio);
            sampleLength = static_cast<int> (((channelProperties.getEndCue () / 2) - actualSampleStart) * sampleRateRatio);
            if (curSampleOffset < sampleStart || curSampleOffset >= sampleStart + sampleLength)
                curSampleOffset = sampleStart;
            LogAudioPlayer ("channelProperties.onStartCueChange - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
        }
    };
    channelProperties.onEndCueChange = [this] (uint32_t newSampleEnd)
    {
        LogAudioPlayer ("channelProperties.onEndCueChange ");
        jassert (sampleRateRatio > 0.0);
        const auto actualSampleEnd { newSampleEnd / 2 };
        juce::ScopedLock sl (dataCS);
        if (playMode == AudioPlayerProperties::PlayMode::once)
            sampleLength = static_cast<int> ((actualSampleEnd - (channelProperties.getStartCue() / 2)) * sampleRateRatio);
        else 
            sampleLength = static_cast<int> ((actualSampleEnd - (channelProperties.getLoopCue() / 2)) * sampleRateRatio);
        if (curSampleOffset >= sampleStart + sampleLength)
            curSampleOffset = sampleStart;
        LogAudioPlayer ("channelProperties.onEndCueChange - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
    };
    channelProperties.onLoopCueChange = [this] (uint32_t newLoopStart)
    {
        LogAudioPlayer ("channelProperties.onLoopCueChange ");
        jassert (sampleRateRatio > 0.0);
        const auto actualLoopEnd { newLoopStart / 2 };
        juce::ScopedLock sl (dataCS);
        if (playMode == AudioPlayerProperties::PlayMode::loop)
        {
            sampleStart = static_cast<int> (actualLoopEnd * sampleRateRatio);
            sampleLength = static_cast<int> (((channelProperties.getEndCue () / 2) - actualLoopEnd) * sampleRateRatio);
            if (curSampleOffset < sampleStart || curSampleOffset >= sampleStart + sampleLength)
                curSampleOffset = sampleStart;
            LogAudioPlayer ("channelProperties.onLoopCueChange  - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
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
    if (playMode == AudioPlayerProperties::PlayMode::once)
    {
        sampleStart = static_cast<int> ((channelProperties.getStartCue () / 2) * sampleRateRatio);
        sampleLength = static_cast<int> (((channelProperties.getEndCue () / 2) - (channelProperties.getStartCue () / 2)) * sampleRateRatio);
    }
    else
    {
        sampleStart = static_cast<int> ((channelProperties.getLoopCue () / 2) * sampleRateRatio);
        sampleLength = static_cast<int> (((channelProperties.getEndCue () / 2) - (channelProperties.getLoopCue () / 2)) * sampleRateRatio);
    }

    if (curSampleOffset < sampleStart || curSampleOffset >= sampleStart + sampleLength)
        curSampleOffset = sampleStart;
    LogAudioPlayer ("AudioPlayer::initSamplePoints - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
}

void AudioPlayer::prepareSampleForPlayback ()
{
    jassert (playState == AudioPlayerProperties::PlayState::stop);
    if (channelProperties.isValid () && channelProperties.getSampleDataAudioBuffer () != nullptr)
    {
        LogAudioPlayer ("prepareSampleForPlayback: sample is ready");
        std::unique_ptr <juce::MemoryAudioSource> readerSource { std::make_unique<juce::MemoryAudioSource> (*channelProperties.getSampleDataAudioBuffer ()->getAudioBuffer(), false, false) };
        std::unique_ptr<MonoToStereoAudioSource> monoToStereoAudioSource { std::make_unique<MonoToStereoAudioSource> (readerSource.get (), false) };
        std::unique_ptr<juce::ResamplingAudioSource> resamplingAudioSource { std::make_unique<juce::ResamplingAudioSource> (monoToStereoAudioSource.get (), false, 2) };
        sampleRateRatio = sampleRate / channelProperties.getSampleDataSampleRate ();
        resamplingAudioSource->setResamplingRatio (channelProperties.getSampleDataSampleRate () / sampleRate);
        resamplingAudioSource->prepareToPlay (blockSize, sampleRate);
        sampleBuffer = std::make_unique<juce::AudioBuffer<float>> (2, static_cast<int> (channelProperties.getSampleDataNumSamples () * sampleRate / channelProperties.getSampleDataSampleRate ()));
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

void AudioPlayer::handlePlayMode (AudioPlayerProperties::PlayMode newPlayMode)
{
    juce::ScopedLock sl (dataCS);
    playMode = newPlayMode;
    initSamplePoints ();
}

void AudioPlayer::handlePlayState (AudioPlayerProperties::PlayState newPlayState)
{
    juce::ScopedLock sl (dataCS);
    if (newPlayState == AudioPlayerProperties::PlayState::stop)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: stop");
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
    auto chachedPlayMode { AudioPlayerProperties::PlayMode::once };
    {
        // NOTE: I am using a lock in the audio callback ONLY BECAUSE the audio play back is a simple audition feature, not recording or performance playback
        juce::ScopedLock sl (dataCS);
        jassert (curSampleOffset >= sampleStart);
        jassert (curSampleOffset < sampleStart + sampleLength);
        originalSampleOffset =  curSampleOffset; // should be >= sampleStart and < sampleStart + sampleLength
        cachedSampleLength = sampleLength;
        cachedSampleStart = sampleStart;
        chachedPlayMode = playMode;
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
        if (chachedPlayMode == AudioPlayerProperties::PlayMode::loop)
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
