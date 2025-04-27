#pragma once

#include <JuceHeader.h>
#include "AudioPlayerProperties.h"
#include "AudioSettingsProperties.h"
#include "../SquidBankProperties.h"
#include "../SquidChannelProperties.h"
#include "../../AppProperties.h"

class AudioPlayer : public juce::AudioSource,
                    public juce::ChangeListener
{
public:

    void init (juce::ValueTree rootProperties);
    void shutdownAudio ();

private:
    AudioSettingsProperties audioSettingsProperties;
    AudioPlayerProperties audioPlayerProperties;
    AppProperties appProperties;
    SquidBankProperties bankProperties;
    SquidChannelProperties channelProperties;
    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    std::unique_ptr <juce::AudioBuffer<float>> sampleBuffer;
    juce::AudioDeviceSelectorComponent audioSetupComp { audioDeviceManager, 0, 0, 0, 256, false, false, true, false };

    juce::CriticalSection dataCS;
    AudioPlayerProperties::PlayState playState { AudioPlayerProperties::PlayState::stop };
    AudioPlayerProperties::PlayMode playMode { AudioPlayerProperties::PlayMode::once };
    int curSampleOffset { 0 };
    int sampleStart { 0 };
    int sampleLength { 0 };

    double sampleRate { 44100.0 };
    int blockSize { 128 };
    double sampleRateRatio { 0.0 };

    class MonoToStereoAudioSource : public juce::AudioSource
    {
    public:
        MonoToStereoAudioSource (AudioSource* inputSource, const bool deleteInputWhenDeleted)
            : input { inputSource, deleteInputWhenDeleted }
        {
            jassert (input != nullptr);
        }

        void prepareToPlay (int, double) {}
        void releaseResources () {}
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
        {
            jassert (bufferToFill.buffer->getNumChannels () == 2);
            // use getNextAudioBlock to fill in the first channel
            input->getNextAudioBlock (bufferToFill);
            // and copy it to the second channel
            bufferToFill.buffer->copyFrom (1, bufferToFill.startSample, bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample), bufferToFill.numSamples);
        }
    private:
        juce::OptionalScopedPointer<AudioSource> input;
    };

    void configureAudioDevice (juce::String deviceName);
    void handlePlayMode (AudioPlayerProperties::PlayMode newPlayMode);
    void handlePlayState (AudioPlayerProperties::PlayState playState);
    void initFromChannel (int channelIndex);
    void initSamplePoints ();
    void prepareSampleForPlayback ();
    void showConfigDialog ();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources () override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
};