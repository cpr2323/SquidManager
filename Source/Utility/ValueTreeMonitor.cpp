#include "ValueTreeMonitor.h"

void ValueTreeMonitor::assign (juce::ValueTree& vtToListenTo)
{
    vtBeingListened = vtToListenTo;
    vtBeingListened.addListener (this);
}

void ValueTreeMonitor::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    juce::String debugString;
    debugString = "valueTreePropertyChanged(" + vtBeingListened.getType ().toString () + ")";
    if (vt == vtBeingListened)
    {
        if (vt.hasProperty (property))
        {
            // change in value
            debugString += " (property value changed) - property: " +
                property.toString () + ", value: " + vt.getProperty (property).toString ();
        }
        else
        {
            // property removed
            debugString += " (property removed) - property: " + property.toString ();
        }
    }
    else
    {
        if (vt.hasProperty (property))
        {
            // change in value
            debugString += " (value changed) - vt: " + vt.getType ().toString () + ", property: " + property.toString () + ", value: " + vt.getProperty (property).toString ();
        }
        else
        {
            // property removed
            debugString += " (property removed) - vt: " + vt.getType ().toString () + ", property: " + property.toString ();
        }
    }

    outputFunction (debugString);
}

void ValueTreeMonitor::valueTreeChildAdded (juce::ValueTree& vt, juce::ValueTree& child)
{
    juce::String debugString;
    debugString = "valueTreeChildAdded(" + vtBeingListened.getType ().toString () + ")";

    if (vt == vtBeingListened)
        debugString += " child: " + child.getType ().toString ();
    else
        debugString += " vt: " + vt.getType ().toString () + ", child: " + child.getType ().toString ();

}

void ValueTreeMonitor::valueTreeChildRemoved (juce::ValueTree& vt, juce::ValueTree& child, int index)
{
    juce::String debugString;
    debugString = "valueTreeChildRemoved(" + vtBeingListened.getType ().toString () + ")";
    if (vt == vtBeingListened)
        debugString += " child: " + child.getType ().toString () + ", index: " + juce::String (index);
    else
        debugString += " vt: " + vt.getType ().toString () + ", child: " + child.getType ().toString () + ", index: " + juce::String (index);
}

void ValueTreeMonitor::valueTreeChildOrderChanged (juce::ValueTree& vt, int oldIndex, int newIndex)
{
    juce::String debugString;
    debugString = "valueTreeChildOrderChanged(" + vtBeingListened.getType ().toString () + ")";
    if (vt == vtBeingListened)
        outputFunction (" old index: " + juce::String (oldIndex) + ", new index: " + juce::String (newIndex));
    else
        outputFunction (" vt: " + vt.getType ().toString () +
            ", old index: " + juce::String (oldIndex) + ", new index: " + juce::String (newIndex));
}

void ValueTreeMonitor::valueTreeParentChanged (juce::ValueTree& vt)
{
    juce::String debugString;
    debugString = "valueTreeParentChanged(" + vtBeingListened.getType ().toString () + ")";
    if (vt == vtBeingListened)
    {
        if (vt.getParent ().isValid ())
            debugString += " new parent: " + vt.getParent ().getType ().toString ();
        else
            debugString += " (removed)";
    }
    else
    {
        if (vt.getParent ().isValid ())
            debugString += " vt: " + vt.getType ().toString () + ", new parent: " + vt.getParent ().getType ().toString ();
        else
                debugString += " (removed) vt: " + vt.getType ().toString ();
    }
}

void ValueTreeMonitor::valueTreeRedirected (juce::ValueTree& vt)
{
    outputFunction ("valueTreeRedirected(" + vtBeingListened.getType ().toString () + "): " + vt.getType ().toString ());
    vtBeingListened = vt;
    //if (vt == vtBeingListened)
}
