#pragma once

#include <JuceHeader.h>
#include "SquidSalmple/EditManager/EditManager.h"
#include "oolib/ValueTree/ValueTreeWrapper.h"

// the name SquidManager registers its audio file type under with DirectoryValueTree. clients turn it into
// the id that appears as the 'type' property of a scanned entry with DirectoryDataProperties::getFileTypeId
inline const juce::String kAudioFileTypeName { "audio" };

class SystemServices : public ValueTreeWrapper<SystemServices>
{
public:
    SystemServices () noexcept : ValueTreeWrapper (SystemServicesTypeId)
    {
    }

    SystemServices (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper (SystemServicesTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    static inline const juce::Identifier SystemServicesTypeId { "SystemServices" };
    static inline const juce::Identifier EditManagerPropertyId   { "editManager" };
    static inline const juce::Identifier AudioDeviceManagerPropertyId { "audioDeviceManager" };

    void setEditManager (EditManager* editManger);
    EditManager* getEditManager ();

    // published by the audio layer so the GUI can build a device selector without
    // the audio layer having to own one
    void setAudioDeviceManager (juce::AudioDeviceManager* audioDeviceManager);
    juce::AudioDeviceManager* getAudioDeviceManager ();

    void initValueTree () {}
    void processValueTree () {}

private:
};
