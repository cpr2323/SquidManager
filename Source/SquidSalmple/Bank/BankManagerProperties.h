#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class BankManagerProperties : public ValueTreeWrapper<BankManagerProperties>
{
public:
    BankManagerProperties () noexcept : ValueTreeWrapper<BankManagerProperties > (BankManagerPropertiesTypeId) {}
    BankManagerProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<BankManagerProperties> (BankManagerPropertiesTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void addBank (juce::String bankName, juce::ValueTree bankPropertiesVT);
    juce::ValueTree getBank (juce::String bankName);

    static inline const juce::Identifier BankManagerPropertiesTypeId { "BankManager" };

    static inline const juce::Identifier BankHolderPropertiesTypeId { "BankHolder" };
    static inline const juce::Identifier BankHolderNamePropertyId { "name" };

    void initValueTree () {};
    void processValueTree () {}

private:
};
