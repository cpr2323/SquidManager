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
    void bankLoseEditWarning (juce::String title, std::function<void ()> overwriteFunction, std::function<void ()> cancelFunction);

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

    class TabbedComponentWithDropTabs : public TabbedComponentWithChangeCallback
    {
    public:
        TabbedComponentWithDropTabs (juce::TabbedButtonBar::Orientation orientation) : TabbedComponentWithChangeCallback (orientation) {}
        std::function<bool (juce::String fileName)> isSupportedFile;
        std::function<bool (juce::String fileName, int channelIndex)> loadFile;

    protected:
        juce::TabBarButton* createTabButton (const juce::String& tabName, int /*tabIndex*/) override
        {
            return new FileDropTargetTabBarButton (tabName, getTabbedButtonBar (), isSupportedFile, loadFile);
        }

    private:
        class FileDropTargetTabBarButton : public juce::TabBarButton,
                                           public juce::FileDragAndDropTarget
        {
        public:
            FileDropTargetTabBarButton (const juce::String& tabName, juce::TabbedButtonBar& tabbedButtonBar,
                                        std::function<bool (juce::String fileName)> isSupportedFileCallback,
                                        std::function<bool (juce::String fileName, int channelIndex)> loadFileCallback)
                : TabBarButton (tabName, tabbedButtonBar),
                  isSupportedFile { isSupportedFileCallback },
                  loadFile { loadFileCallback }
            {
            }

            bool isInterestedInFileDrag (const juce::StringArray& files)  override
            {
                jassert (isSupportedFile != nullptr);
                if (files.size () != 1 || ! isSupportedFile (files [0]))
                    return false;

                return true;
            }
            void fileDragEnter (const juce::StringArray& /*files*/, int /*x*/, int /*y*/) override
            {
                draggingFile = true;
                repaint ();
            }
            void fileDragExit (const juce::StringArray& /*files*/) override
            {
                draggingFile = false;
                repaint ();
            }
            void filesDropped (const juce::StringArray& files, int /*x*/, int /*y*/) override
            {
                DebugLog ("TabbedComponentWithDropTabs", "tab drop index: " + juce::String (getIndex ()));
                draggingFile = false;
                repaint ();
                if (! loadFile (files [0], getIndex ()))
                {
                    // TODO - indicate an error?
                }
            }

        private:
            bool draggingFile { false };
            std::function<bool (juce::String fileName)> isSupportedFile;
            std::function<bool (juce::String fileName, int channelIndex)> loadFile;
            void paintOverChildren (juce::Graphics& g) override
            {
                TabBarButton::paintOverChildren (g);
                if (draggingFile)
                    g.fillAll (juce::Colours::white.withAlpha (0.5f));

            }
        };
    };

    juce::Label bankNameLabel;
    juce::TextEditor bankNameEditor;
    juce::TextButton saveButton;
    juce::TextButton toolsButton;
    TabbedComponentWithDropTabs channelTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
    std::unique_ptr<juce::FileChooser> fileChooser;

    std::array<ChannelEditorComponent, 8> channelEditorComponents;

    void nameUiChanged (juce::String name);
    void nameDataChanged (juce::String name);

    void timerCallback () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};
