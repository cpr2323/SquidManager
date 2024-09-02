#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class EditMangagerProperties : public ValueTreeWrapper<EditMangagerProperties>
{
public:
    EditMangagerProperties () noexcept : ValueTreeWrapper (EditManagerTypeId)
    {
    }

    EditMangagerProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper (EditManagerTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    static inline const juce::Identifier EditManagerTypeId { "EditManager" };

    void initValueTree ();
    void processValueTree () {}

private:
};
