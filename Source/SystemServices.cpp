#include "SystemServices.h"

void SystemServices::setEditManager (EditManager* editManger)
{
    setValue (editManger, EditManagerPropertyId, false);
}

EditManager* SystemServices::getEditManager ()
{
    return getValue<EditManager*> (EditManagerPropertyId, data);
}

void SystemServices::setAudioDeviceManager (juce::AudioDeviceManager* audioDeviceManager)
{
    setValue (audioDeviceManager, AudioDeviceManagerPropertyId, false);
}

juce::AudioDeviceManager* SystemServices::getAudioDeviceManager ()
{
    return getValue<juce::AudioDeviceManager*> (AudioDeviceManagerPropertyId, data);
}
