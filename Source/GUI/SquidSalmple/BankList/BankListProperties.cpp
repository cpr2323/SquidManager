#include "BankListProperties.h"

void BankListProperties::setStatus (juce::String status, bool includeSelfCallback)
{
    setValue (status, BankListStatusPropertyId, includeSelfCallback);
}

juce::String BankListProperties::getStatus ()
{
    return getValue<juce::String> (BankListStatusPropertyId);
}

void BankListProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt != data)
        return;

    if (property == BankListStatusPropertyId)
    {
        if (onStatusChange)
            onStatusChange (getStatus ());
    }
}