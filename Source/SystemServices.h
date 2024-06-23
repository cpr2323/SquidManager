#pragma once

#include <JuceHeader.h>
#include "SquidSalmple/EditManager/EditManager.h"
#include "Utility/ValueTreeWrapper.h"

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

    void setEditManager (EditManager* editManger);
    EditManager* getEditManager ();

    void initValueTree () {}
    void processValueTree () {}

private:
};
