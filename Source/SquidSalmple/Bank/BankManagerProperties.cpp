#include "BankManagerProperties.h"

void BankManagerProperties::addBank (juce::String bankName, juce::ValueTree bankPropertiesVT)
{
    // is name already used
    jassert (! data.getChildWithName (bankName).isValid ());
    if (data.getChildWithName (bankName).isValid ())
        return;
    juce::ValueTree newBankHolder { BankHolderPropertiesTypeId };
    newBankHolder.setProperty (BankHolderNamePropertyId, bankName, nullptr);
    newBankHolder.addChild (bankPropertiesVT, -1, nullptr);
    data.addChild (newBankHolder, -1, nullptr);
}

juce::ValueTree BankManagerProperties::getBank (juce::String bankName)
{
    auto bankHolder { data.getChildWithProperty (BankHolderNamePropertyId, bankName) };
    jassert (bankHolder.isValid ());
    if (! bankHolder.isValid ())
        return {};
    return bankHolder.getChild (0);
}
