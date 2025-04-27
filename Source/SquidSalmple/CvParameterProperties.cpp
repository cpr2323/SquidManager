#include "CvParameterProperties.h"

void CvParameterProperties::initValueTree ()
{
    setId (0, false);
    setName ("", false);
    setEnabled (false, false);
    setAttenuation (0, false);
    setOffset (0, false);
}

void CvParameterProperties::setId (int id, bool includeSelfCallback)
{
    setValue (id, CvParameterIdPropertyId, includeSelfCallback);
}

void CvParameterProperties::setName (juce::String name, bool includeSelfCallback)
{
    setValue (name, CvParameterIdName, includeSelfCallback);
}

void CvParameterProperties::setEnabled (bool enabled, bool includeSelfCallback)
{
    setValue (enabled, CvParameterEnabledPropertyId, includeSelfCallback);
}

void CvParameterProperties::setAttenuation (int attenuation, bool includeSelfCallback)
{
    setValue (attenuation, CvParameterAttenuatePropertyId, includeSelfCallback);
}

void CvParameterProperties::setOffset (int offset, bool includeSelfCallback)
{
    setValue (offset, CvParameterOffsetPropertyId, includeSelfCallback);
}

int CvParameterProperties::getId ()
{
    return getValue<int> (CvParameterIdPropertyId);
}

juce::String CvParameterProperties::getName ()
{
    return getValue<juce::String> (CvParameterIdName);
}

bool CvParameterProperties::getEnabled ()
{
    return getValue<bool> (CvParameterEnabledPropertyId);
}

int CvParameterProperties::getAttenuation ()
{
    return getValue<int> (CvParameterAttenuatePropertyId);
}

int CvParameterProperties::getOffset ()
{
    return getValue<int> (CvParameterOffsetPropertyId);
}

void CvParameterProperties::copyFrom (juce::ValueTree sourceVT)
{
    setId (sourceVT [CvParameterIdPropertyId], false);
    setName (sourceVT [CvParameterIdName], false);
    setEnabled (sourceVT [CvParameterEnabledPropertyId], false);
    setAttenuation (sourceVT [CvParameterAttenuatePropertyId], false);
    setOffset (sourceVT [CvParameterOffsetPropertyId], false);
}

void CvParameterProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt == data)
    {
        if (property == CvParameterIdPropertyId)
        {
            if (onIdChange != nullptr)
                onIdChange (getId ());
        }
        else if (property == CvParameterIdName)
        {
            if (onNameChange != nullptr)
                onNameChange (getName ());
        }
        else if (property == CvParameterEnabledPropertyId)
        {
            if (onEnabledChange != nullptr)
                onEnabledChange (getEnabled ());
        }
        else if (property == CvParameterAttenuatePropertyId)
        {
            if (onAttenuateChange != nullptr)
                onAttenuateChange (getAttenuation ());
        }
        else if (property == CvParameterOffsetPropertyId)
        {
            if (onOffsetChange != nullptr)
                onOffsetChange (getOffset ());
        }
    }
}