#pragma once

#include <JuceHeader.h>
#include "ChannelEditorComponent.h"
#include "../../AppProperties.h"
#include "../../SquidSalmple/SquidMetaDataProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

class SquidEditorComponent : public juce::Component,
                                     public juce::Timer
{
public:
    SquidEditorComponent ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    SquidMetaDataProperties squidMetaDataProperties;
    juce::TabbedComponent channelTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TextButton loadButton;

    //ChannelEditorComponent channelEditorComponent;
    std::array<ChannelEditorComponent, 8> channelEditorComponents;

    void timerCallback () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};
