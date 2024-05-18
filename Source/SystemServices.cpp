#include "SystemServices.h"

void SystemServices::setSampleManager (SampleManager* sampleManger) // retrieve pointer from VT and return it as a reference
{
    setValue (sampleManger, SampleManagerPropertyId, false);
}

SampleManager& SystemServices::getSampleManager () // retrieve pointer from VT and return it as a reference
{
    return *(getValue<SampleManager*> (SampleManagerPropertyId, data));
}
