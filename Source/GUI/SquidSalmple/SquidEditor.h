#pragma once

#include <JuceHeader.h>
#include "ChannelEditorComponent.h"
#include "../Theme/UiComponents.h"
#include "../../AppProperties.h"
#include "../../SquidSalmple/Audio/AudioPlayerProperties.h"
#include "../../SquidSalmple/SquidBankProperties.h"
#include "../../SquidSalmple/EditManager/EditManager.h"
#include "oolib/Properties/RuntimeRootProperties.h"

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

        // lights the tab's led when that channel holds a sample
        void setChannelHasContent (int tabIndex, bool channelHasContent)
        {
            if (auto* tabButton { dynamic_cast<FileDropTargetTabBarButton*> (getTabbedButtonBar ().getTabButton (tabIndex)) })
                tabButton->setHasContent (channelHasContent);
        }

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
                //DebugLog ("TabbedComponentWithDropTabs", "tab drop index: " + juce::String (getIndex ()));
                draggingFile = false;
                repaint ();
                if (! loadFile (files [0], getIndex ()))
                {
                    // TODO - indicate an error?
                }
            }

            void setHasContent (bool channelHasContent)
            {
                if (hasContent == channelHasContent)
                    return;
                hasContent = channelHasContent;
                repaint ();
            }

        private:
            bool draggingFile { false };
            bool hasContent { false };
            std::function<bool (juce::String fileName)> isSupportedFile;
            std::function<bool (juce::String fileName, int channelIndex)> loadFile;
            void paintButton (juce::Graphics& g, bool isMouseOver, bool /*isMouseDown*/) override
            {
                const auto bounds { getLocalBounds () };
                const auto selected { getToggleState () };
                const auto hovered { isMouseOver && ! selected };

                // the front tab takes the colour of the page under it, so the two
                // read as one surface; the others sit on the strip
                if (selected)
                    g.fillAll (findColour (SquidColours::windowBackground));
                else if (hovered)
                    g.fillAll (findColour (SquidColours::panelHeader));

                g.setColour (findColour (SquidColours::outlineDim));
                g.fillRect (bounds.withLeft (bounds.getRight () - 1));

                // says at a glance which channels are in use
                auto content { bounds.withTrimmedLeft (13) };
                const auto ledBounds { content.removeFromLeft (static_cast<int> (StatusLed::kDiameter)).toFloat ()
                                              .withSizeKeepingCentre (StatusLed::kDiameter, StatusLed::kDiameter) };
                StatusLed::draw (g, ledBounds, hasContent, *this);
                content.removeFromLeft (7);

                g.setFont (SquidType::channelTab ());
                g.setColour (findColour (selected ? SquidColours::text
                                                  : (hovered ? SquidColours::textDim : SquidColours::menuHeaderText)));
                g.drawText (getButtonText (), content, juce::Justification::centredLeft, false);

                if (selected)
                {
                    g.setColour (findColour (SquidColours::accent));
                    g.fillRect (bounds.withTop (bounds.getBottom () - 2));
                }
                else if (hovered)
                {
                    g.setColour (findColour (SquidColours::accentDeep));
                    g.drawRect (bounds.withTrimmedBottom (1), 1);
                }

                if (draggingFile)
                    g.fillAll (findColour (SquidColours::dropOverlay));
            }
        };
    };

    juce::Label bankNameLabel;
    juce::TextEditor bankNameEditor;
    ActionButton saveButton { "SAVE BANK" };
    MenuButton toolsButton { "BANK TOOLS" };
    bool bankHasUnsavedEdits { false };

    static constexpr int kHeaderHeight { 38 };
    static constexpr int kTabBarHeight { 31 };
    static constexpr int kFieldHeight { 23 };
    static inline const juce::String kUnsavedEditsText { "UNSAVED EDITS" };

    // set in resized, drawn in paint
    juce::Rectangle<int> headerBounds;
    juce::Rectangle<int> unsavedEditsBounds;
    TabbedComponentWithDropTabs channelTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
    std::unique_ptr<juce::FileChooser> fileChooser;

    std::array<ChannelEditorComponent, 8> channelEditorComponents;

    void nameUiChanged (juce::String name);
    void nameDataChanged (juce::String name);

    void timerCallback () override;
    void resized () override;
    void applyExplicitColours ();
    void lookAndFeelChanged () override;
    void paint (juce::Graphics& g) override;
};
