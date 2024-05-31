#pragma once

#include <JuceHeader.h>
#include "ChannelEditorComponent.h"
#include "../../AppProperties.h"
#include "../../SquidSalmple/SquidBankProperties.h"
#include "../../SquidSalmple/EditManager/EditManager.h"
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
    SquidBankProperties squidBankProperties;
    SquidBankProperties unEditedSquidBankProperties;
    EditManager* editManager { nullptr };

    juce::TabbedComponent channelTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::Label bankNameLabel;
    juce::TextEditor bankNameEditor;
    juce::TextButton loadButton;
    juce::TextButton saveButton;

    std::array<ChannelEditorComponent, 8> channelEditorComponents;

    void nameUiChanged (juce::String name);
    void nameDataChanged (juce::String name);

    void timerCallback () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};
