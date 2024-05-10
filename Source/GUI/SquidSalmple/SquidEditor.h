#pragma once

#include <JuceHeader.h>
#include "ChannelEditorComponent.h"
#include "../../AppProperties.h"
#include "../../SquidSalmple/SquidMetaDataProperties.h"
#include "../../Utility/RuntimeRootProperties.h"


class SquidMetaDataEditorComponent : public juce::Component,
                                     public juce::Timer
{
public:
    SquidMetaDataEditorComponent ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    SquidMetaDataProperties squidMetaDataProperties;
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TextButton loadButton;

    ChannelEditorComponent channelEditorComponent;

    void timerCallback () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};
