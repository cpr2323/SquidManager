#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"

class CurrentFolderComponent : public juce::Component
{
public:
    CurrentFolderComponent ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;

    juce::Label currentFolderAndProgressLabel;

    juce::String getFolderAndProgressString (juce::String folder, juce::String);

    void resized () override;
};
