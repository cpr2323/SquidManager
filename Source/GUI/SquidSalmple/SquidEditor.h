#pragma once

#include <JuceHeader.h>
#include "ChannelEditorComponent.h"
#include "../../AppProperties.h"
#include "../../SquidSalmple/Audio/AudioPlayerProperties.h"
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
    AudioPlayerProperties audioPlayerProperties;
    SquidBankProperties squidBankProperties;
    SquidBankProperties unEditedSquidBankProperties;
    EditManager* editManager { nullptr };

    class TabbedComponentWithChangeCallback : public juce::TabbedComponent
    {
    public:
        TabbedComponentWithChangeCallback (juce::TabbedButtonBar::Orientation orientation) : juce::TabbedComponent (orientation) {}

        std::function<void (int)> onSelectedTabChanged;

    private:
        void currentTabChanged (int newTabIndex, [[maybe_unused]] const juce::String& tabName)
        {
            if (onSelectedTabChanged != nullptr)
                onSelectedTabChanged (newTabIndex);
        }
    };

    juce::Label bankNameLabel;
    juce::TextEditor bankNameEditor;
    juce::TextButton saveButton;
    juce::TextButton toolsButton;
    TabbedComponentWithChangeCallback channelTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
    std::unique_ptr<juce::FileChooser> fileChooser;

    std::array<ChannelEditorComponent, 8> channelEditorComponents;

    void nameUiChanged (juce::String name);
    void nameDataChanged (juce::String name);

    void timerCallback () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};
