#include "AudioPlayerProperties.h"

void AudioPlayerProperties::initValueTree ()
{
    setPlayState (PlayState::stop, false);
    setSampleSource (-1, false);
}

void AudioPlayerProperties::setPlayState (PlayState playState, bool includeSelfCallback)
{
    setValue (static_cast<int> (playState), PlayStatePropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setPlayMode (PlayMode playMode, bool includeSelfCallback)
{
    setValue (static_cast<int> (playMode), PlayModePropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setSampleSource (int channelIndex, bool includeSelfCallback)
{
    setValue (channelIndex, SampleSourcePropertyId, includeSelfCallback);
}

void AudioPlayerProperties::showConfigDialog (bool includeSelfCallback)
{
    toggleValue (ShowConfigDialogPropertyId, includeSelfCallback);
}

AudioPlayerProperties::PlayState AudioPlayerProperties::getPlayState ()
{
    return static_cast<PlayState> (getValue<int> (PlayStatePropertyId));
}

AudioPlayerProperties::PlayMode AudioPlayerProperties::getPlayMode ()
{
    return static_cast<PlayMode> (getValue<int> (PlayModePropertyId));
}

int AudioPlayerProperties::getSampleSource ()
{
    return getValue<int> (SampleSourcePropertyId);
}

void AudioPlayerProperties::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == data)
    {
        if (property == PlayStatePropertyId)
        {
            if (onPlayStateChange != nullptr)
                onPlayStateChange (getPlayState ());
        }
        else if (property == PlayModePropertyId)
        {
            if (onPlayModeChange != nullptr)
                onPlayModeChange (getPlayMode ());
        }
        else if (property == SampleSourcePropertyId)
        {
            if (onSampleSourceChanged != nullptr)
                onSampleSourceChanged (getSampleSource ());
        }
        else if (property == ShowConfigDialogPropertyId)
        {
            if (onShowConfigDialog != nullptr)
                onShowConfigDialog ();
        }
    }
}

