#include "SystemServices.h"

void SystemServices::setSampleManager (SampleManager* sampleManger) // retrieve pointer from VT and return it as a reference
{
    setValue (sampleManger, SampleManagerPropertyId, false);
}

SampleManager* SystemServices::getSampleManager () // retrieve pointer from VT and return it as a reference
{
    return getValue<SampleManager*> (SampleManagerPropertyId, data);
}

void SystemServices::setEditManager (EditManager* editManger)
{
    setValue (editManger, EditManagerPropertyId, false);
}

EditManager* SystemServices::getEditManager ()
{
    return getValue<EditManager*> (EditManagerPropertyId, data);
}
