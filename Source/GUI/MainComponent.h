#pragma once

#include <JuceHeader.h>
#include "GuiProperties.h"
#include "SquidSalmple/SquidEditor.h"
#include "../Utility/SplitWindowComponent.h"

class MainComponent : public juce::Component
{
public:
    MainComponent (juce::ValueTree rootPropertiesVT);
    ~MainComponent () = default;

private:
    GuiProperties guiProperties;
    SquidMetaDataEditorComponent squidMetaDataEditorComponent;

    juce::TooltipWindow tooltipWindow;

    void restoreLayout ();
    void saveLayoutChanges ();

    void resized () override;
    void paint (juce::Graphics& g) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
