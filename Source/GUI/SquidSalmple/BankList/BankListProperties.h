#pragma once

#include <JuceHeader.h>
#include "../../../Utility/ValueTreeWrapper.h"

class BankListProperties : public ValueTreeWrapper<BankListProperties>
{
public:
    BankListProperties () noexcept : ValueTreeWrapper<BankListProperties> (BankListPropertiesTypeId) {}
    BankListProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<BankListProperties> (BankListPropertiesTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void setStatus (juce::String status, bool includeSelfCallback);
    juce::String getStatus ();

    static inline const juce::Identifier BankListPropertiesTypeId { "BankList" };
    static inline const juce::Identifier BankListStatusPropertyId { "status" };

    std::function<void (juce::String status)> onStatusChange;

    void initValueTree () {};
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};