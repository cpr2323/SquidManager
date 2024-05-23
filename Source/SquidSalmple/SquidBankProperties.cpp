#include "SquidBankProperties.h"
#include "SquidChannelProperties.h"

const auto kNumChannels { 8 };

void SquidBankProperties::initValueTree ()
{
    setName ("", false);
    for (auto channelIndex { 0 }; channelIndex < kNumChannels; ++channelIndex)
    {
        auto channel { SquidChannelProperties::create (channelIndex) };
        data.addChild (channel, -1, nullptr);
    }
}

void SquidBankProperties::setName (juce::String name, bool includeSelfCallback)
{
    setValue (name, NamePropertyId, includeSelfCallback);
}

void SquidBankProperties::triggerLoadBegin (bool includeSelfCallback)
{
    toggleValue (LoadBeginPropertyId, includeSelfCallback);
}

void SquidBankProperties::triggerLoadComplete (bool includeSelfCallback)
{
    toggleValue (LoadCompletePropertyId, includeSelfCallback);
}

juce::String SquidBankProperties::getName ()
{
    return getValue<juce::String> (NamePropertyId);
}

void SquidBankProperties::forEachChannel (std::function<bool (juce::ValueTree channelVT, int channelIndex)> channelVTCallback)
{
    jassert (channelVTCallback != nullptr);
    auto curChannelIndex { 0 };
    ValueTreeHelpers::forEachChildOfType (data, SquidChannelProperties::SquidChannelTypeId, [this, &curChannelIndex, channelVTCallback] (juce::ValueTree channelVT)
    {
        auto keepIterating { channelVTCallback (channelVT, curChannelIndex) };
        ++curChannelIndex;
        return keepIterating;
    });
}

juce::ValueTree SquidBankProperties::getChannelVT (int channelIndex)
{
    jassert (channelIndex < kNumChannels);
    juce::ValueTree requestedChannelVT;
    forEachChannel ([this, &requestedChannelVT, channelIndex] (juce::ValueTree channelVT, int curChannelIndex)
    {
        if (curChannelIndex == channelIndex)
        {
            requestedChannelVT = channelVT;
            return false;
        }
        return true;
    });
    jassert (requestedChannelVT.isValid ());
    return requestedChannelVT;
}

void SquidBankProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt == data)
    {
        if (property == NamePropertyId)
        {
            if (onNameChange != nullptr)
                onNameChange (getName ());
        }
        else if (property == LoadBeginPropertyId)
        {
            if (onLoadBegin != nullptr)
                onLoadBegin ();
        }
        else if (property == LoadCompletePropertyId)
        {
            if (onLoadComplete != nullptr)
                onLoadComplete ();
        }
    }
}
