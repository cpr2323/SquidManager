#include "SystemServices.h"

void SystemServices::setEditManager (EditManager* editManger)
{
    setValue (editManger, EditManagerPropertyId, false);
}

EditManager* SystemServices::getEditManager ()
{
    return getValue<EditManager*> (EditManagerPropertyId, data);
}
