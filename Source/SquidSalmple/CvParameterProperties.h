#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class CvParameterProperties : public ValueTreeWrapper<CvParameterProperties>
{
public:
    CvParameterProperties () noexcept : ValueTreeWrapper (CvParameterTypeId)
    {
    }

    CvParameterProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper (CvParameterTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setId (int id, bool includeSelfCallback);
    void setName (juce::String name, bool includeSelfCallback);
    void setEnabled (bool enabled, bool includeSelfCallback);
    void setAttenuation (int attenuation, bool includeSelfCallback);
    void setOffset (int offset, bool includeSelfCallback);

    int getId ();
    juce::String getName ();
    bool getEnabled ();
    int getAttenuation ();
    int getOffset ();

    std::function<void (int)> onIdChange;
    std::function<void (juce::String name)> onNameChange;
    std::function<void (bool)> onEnabledChange;
    std::function<void (int)> onAttenuateChange;
    std::function<void (int)> onOffsetChange;

    void copyFrom (juce::ValueTree sourceVT);

    static inline const juce::Identifier CvParameterTypeId { "CvParameter" };
    static inline const juce::Identifier CvParameterIdPropertyId        { "_id" };
    static inline const juce::Identifier CvParameterIdName              { "_name" };
    static inline const juce::Identifier CvParameterEnabledPropertyId   { "enabled" };
    static inline const juce::Identifier CvParameterAttenuatePropertyId { "attenuation" };
    static inline const juce::Identifier CvParameterOffsetPropertyId    { "offset" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
