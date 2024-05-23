#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class SquidBankProperties : public ValueTreeWrapper<SquidBankProperties>
{
public:
    SquidBankProperties () noexcept : ValueTreeWrapper (SquidBankTypeId)
    {
    }

    SquidBankProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper (SquidBankTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setName (juce::String name, bool includeSelfCallback);
    void triggerLoadBegin (bool includeSelfCallback);
    void triggerLoadComplete (bool includeSelfCallback);

    juce::String getName ();

    std::function<void (juce::String name)> onNameChange;
    std::function<void ()> onLoadBegin;
    std::function<void ()> onLoadComplete;

    void copyFrom (juce::ValueTree sourceVT);
    static juce::ValueTree create ();
    void forEachChannel (std::function<bool (juce::ValueTree channelVT, int channelIndex)> channelVTCallback);
    juce::ValueTree getChannelVT (int channelIndex);

    static inline const juce::Identifier SquidBankTypeId { "SquidBank" };
    static inline const juce::Identifier NamePropertyId         { "name" };
    static inline const juce::Identifier LoadBeginPropertyId    { "_loadBegin" };
    static inline const juce::Identifier LoadCompletePropertyId { "_loadComplete" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
