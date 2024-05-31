#include "BankHelpers.h"
#include "SquidBankProperties.h"
#include "SquidChannelProperties.h"

namespace BankHelpers
{
#define LOG_DIFFERENCE 0
#if LOG_DIFFERENCE
#define LogDifference (parameter, value1, value2) DebugLog ("areEntireBanksEqual - [" + parameter + "]: " + value1 + " != " + value2);
#endif
    bool areEntireBanksEqual (juce::ValueTree bankOneVT, juce::ValueTree bankTwoVT)
    {
        auto banksAreEqual { true };
        if (! areBanksEqual (bankOneVT, bankTwoVT))
        {
            //juce::Logger::outputDebugString ("Bank mismatch");
            banksAreEqual = false;
        }
        else
        {
            SquidBankProperties bankOne (bankOneVT, SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::no);
            SquidBankProperties bankTwo (bankTwoVT, SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::no);
            for (auto channelIndex { 0 }; channelIndex < 8 && banksAreEqual; ++channelIndex)
            {
                if (! areChannelsEqual (bankOne.getChannelVT (channelIndex), bankTwo.getChannelVT (channelIndex)))
                {
                    //juce::Logger::outputDebugString ("Channel "+juce::String (channelIndex) + " mismatch");
                    banksAreEqual = false;
                }
            }
        }

        return banksAreEqual;
    }

    bool areBanksEqual (juce::ValueTree bankOneVT, juce::ValueTree bankTwoVT)
    {
        SquidBankProperties bankPropertiesOne (bankOneVT, SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::no);
        SquidBankProperties bankPropertiesTwo (bankTwoVT, SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::no);
#if LOG_DIFFERENCE
#endif
        return  bankPropertiesOne.getName () == bankPropertiesTwo.getName ();
    };

    bool areCvAssignsEqual (juce::ValueTree channelOneVT, juce::ValueTree channelTwoVT)
    {
        SquidChannelProperties channelPropertiesOne (channelOneVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
        SquidChannelProperties channelPropertiesTwo (channelTwoVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
        for (auto cvAssignIndex { 0 }; cvAssignIndex < 8; ++cvAssignIndex)
            for (auto cvAssignParameterIndex { 0 }; cvAssignParameterIndex < 15; ++cvAssignParameterIndex)
            {
                if ((channelPropertiesOne.getCvAssignEnabled (cvAssignIndex, cvAssignParameterIndex) != channelPropertiesTwo.getCvAssignEnabled(cvAssignIndex, cvAssignParameterIndex)) ||
                    (channelPropertiesOne.getCvAssignAttenuate(cvAssignIndex, cvAssignParameterIndex) != channelPropertiesTwo.getCvAssignAttenuate (cvAssignIndex, cvAssignParameterIndex)) ||
                    (channelPropertiesOne.getCvAssignOffset (cvAssignIndex, cvAssignParameterIndex) != channelPropertiesTwo.getCvAssignOffset (cvAssignIndex, cvAssignParameterIndex)))
                    return false;
            }
        return true;
    }

    bool areCueSetsEqual (juce::ValueTree channelOneVT, juce::ValueTree channelTwoVT)
    {
        SquidChannelProperties channelPropertiesOne (channelOneVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
        SquidChannelProperties channelPropertiesTwo (channelTwoVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
        if (channelPropertiesOne.getNumCueSets () != channelPropertiesTwo.getNumCueSets ())
            return false;

        const auto numCueSets { channelPropertiesOne.getNumCueSets () };
        for (auto cueSetIndex { 0 }; cueSetIndex < numCueSets; ++cueSetIndex)
            if ((channelPropertiesOne.getEndCueSet (cueSetIndex) != channelPropertiesTwo.getEndCueSet (cueSetIndex)) ||
                (channelPropertiesOne.getLoopCueSet (cueSetIndex) != channelPropertiesTwo.getLoopCueSet (cueSetIndex)) ||
                (channelPropertiesOne.getStartCueSet (cueSetIndex) != channelPropertiesTwo.getStartCueSet (cueSetIndex)))
                return false;
        return true;
    }
    bool areChannelsEqual (juce::ValueTree channelOneVT, juce::ValueTree channelTwoVT)
    {
        SquidChannelProperties channelPropertiesOne (channelOneVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
        SquidChannelProperties channelPropertiesTwo (channelTwoVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
#if LOG_DIFFERENCE
#endif
        // compare CV Assigns
        if (! areCvAssignsEqual (channelOneVT, channelTwoVT))
            return false;

        // compare Cue Sets
        if (! areCueSetsEqual (channelOneVT, channelTwoVT))
            return false;

        // if the cv Assigns and cue sets match, lastly compare base properties
        return channelPropertiesOne.getAttack () == channelPropertiesTwo.getAttack () &&
               channelPropertiesOne.getBits () == channelPropertiesTwo.getBits () &&
               channelPropertiesOne.getChannelFlags () == channelPropertiesTwo.getChannelFlags () &&
               channelPropertiesOne.getChannelSource () == channelPropertiesTwo.getChannelSource () &&
               channelPropertiesOne.getChoke () == channelPropertiesTwo.getChoke () &&
               channelPropertiesOne.getCurCueSet () == channelPropertiesTwo.getCurCueSet () &&
               channelPropertiesOne.getDecay () == channelPropertiesTwo.getDecay () &&
               channelPropertiesOne.getETrig () == channelPropertiesTwo.getETrig () &&
               channelPropertiesOne.getEndCue () == channelPropertiesTwo.getEndCue () &&
               channelPropertiesOne.getFilterFrequency () == channelPropertiesTwo.getFilterFrequency () &&
               channelPropertiesOne.getFilterResonance () == channelPropertiesTwo.getFilterResonance () &&
               channelPropertiesOne.getFilterType () == channelPropertiesTwo.getFilterType () &&
               channelPropertiesOne.getLevel () == channelPropertiesTwo.getLevel () &&
               channelPropertiesOne.getLoopCue () == channelPropertiesTwo.getLoopCue () &&
               channelPropertiesOne.getLoopMode () == channelPropertiesTwo.getLoopMode () &&
               channelPropertiesOne.getQuant () == channelPropertiesTwo.getQuant () &&
               channelPropertiesOne.getRate () == channelPropertiesTwo.getRate () &&
               channelPropertiesOne.getRecDest () == channelPropertiesTwo.getRecDest () &&
               channelPropertiesOne.getReverse () == channelPropertiesTwo.getReverse () &&
               channelPropertiesOne.getSpeed () == channelPropertiesTwo.getSpeed () &&
               channelPropertiesOne.getStartCue () == channelPropertiesTwo.getStartCue () &&
               channelPropertiesOne.getSteps () == channelPropertiesTwo.getSteps () &&
               channelPropertiesOne.getXfade () == channelPropertiesTwo.getXfade ();
    };
};