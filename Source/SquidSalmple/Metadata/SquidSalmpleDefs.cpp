#include "SquidSalmpleDefs.h"

namespace CvParameterIndex
{
    juce::String getParameterName (uint32_t cvAssignFlag)
    {
        if (cvAssignFlag & CvAssignedFlag::bits)
            return "bits";
        if (cvAssignFlag & CvAssignedFlag::rate)
            return "rate";
        if (cvAssignFlag & CvAssignedFlag::level)
            return "level";
        if (cvAssignFlag & CvAssignedFlag::decay)
            return "decay";
        if (cvAssignFlag & CvAssignedFlag::speed)
            return "speed";
        if (cvAssignFlag & CvAssignedFlag::loopMode)
            return "loopMode";
        if (cvAssignFlag & CvAssignedFlag::reverse)
            return "reverse";
        if (cvAssignFlag & CvAssignedFlag::startCue)
            return "startCue";
        if (cvAssignFlag & CvAssignedFlag::endCue)
            return "endCue";
        if (cvAssignFlag & CvAssignedFlag::loopCue)
            return "loopCue";
        if (cvAssignFlag & CvAssignedFlag::attack)
            return "attack";
        if (cvAssignFlag & CvAssignedFlag::cueSet)
            return "cue Set";
        if (cvAssignFlag & CvAssignedFlag::eTrig)
            return "eTrig";
        if (cvAssignFlag & CvAssignedFlag::filtFreq)
            return "filterFrequency";
        if (cvAssignFlag & CvAssignedFlag::filtRes)
            return "filterResonance";
        if (cvAssignFlag & CvAssignedFlag::pitchShift)
            return "pitchShift";
        if (cvAssignFlag & CvAssignedFlag::unused2)
            return "unused2";
        if (cvAssignFlag & CvAssignedFlag::unused)
        {
            // this value is not yet mapped
            jassertfalse;
            return "<error - bit 1>";
        }
        jassertfalse;
        return "<error - unknown bit>";
    };
};
