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
        void paintOverChildren (juce::Graphics& g) override
        {
            juce::TabbedComponent::paintOverChildren (g);
            g.setColour (findColour (SquidColours::outline));
            g.drawRect (getLocalBounds (), 1);
        }

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
            void paintOverChildren (juce::Graphics& g) override
            {
                TabBarButton::paintOverChildren (g);

                // says at a glance which channels are in use
                const auto ledBounds { getLocalBounds ().withTrimmedLeft (7).withWidth (7)
                                                        .withSizeKeepingCentre (7, 7).toFloat () };
                StatusLed::draw (g, ledBounds, hasContent,
                                 findColour (SquidColours::markerStart), findColour (SquidColours::outline));

                if (draggingFile)
                    g.fillAll (findColour (SquidColours::dropOverlay));
            }

            void paintButton (juce::Graphics& g, bool isMouseOver, bool /*isMouseDown*/) override
            {
                // Draw a compact tab: small left LED, tight label plate and a thin
                // accent underline when selected to match the facelift mockup.
                const auto full = getLocalBounds();

                // LED to the left
                const auto ledBounds = full.withTrimmedLeft (6).withWidth (7)
                                            .withSizeKeepingCentre (7, 7).toFloat ();
                StatusLed::draw (g, ledBounds, hasContent,
                                 findColour (SquidColours::markerStart), findColour (SquidColours::outline));

                // text area sits to the right of the LED with a small inset
                auto textArea = full.withTrimmedLeft (16).reduced (6, 4);

                // If selected, draw a subtle filled plate behind the entire
                // tab (including the LED) and a thin accent rail at its bottom
                // edge so the LED reads as part of the highlighted area.
                if (getToggleState())
                {
                    g.setColour (findColour (SquidColours::selectedRow));
                    g.fillRoundedRectangle (full.toFloat(), 3.0f);
                    g.setColour (findColour (SquidColours::accent));
                    auto railArea = full;
                    railArea.removeFromBottom (2);
                    g.fillRect (railArea.removeFromBottom (2).toFloat());
                }

                // Text
                g.setFont (SquidFonts::condensed (11.5f, "SemiBold"));
                g.setColour (findColour (getToggleState () ? SquidColours::accentText
                                                          : (isMouseOver ? SquidColours::textDim : SquidColours::menuHeaderText)));
                g.drawText (getButtonText (), full.withTrimmedLeft (20), juce::Justification::centredLeft, false);

                // show the drop overlay if a file is being dragged (overlay the
                // tab content area only)
                if (draggingFile)
                {
                    g.setColour (findColour (SquidColours::dropOverlay));
                    g.fillRoundedRectangle (full.toFloat(), 3.0f);
                }

                // draw a hairline separator on the right edge so tabs read as
                // distinct items
                g.setColour (findColour (SquidColours::outline));
                const int x = full.getRight () - 1;
                g.drawVerticalLine (x, static_cast<float> (full.getY () + 3),
                                    static_cast<float> (full.getBottom () - 3));
            }
        };
    };

    juce::Label bankNameLabel;
    juce::TextEditor bankNameEditor;
    juce::Label unsavedEditsLabel;
    juce::TextButton saveButton;
    MenuButton toolsButton { "BANK TOOLS" };
    bool bankHasUnsavedEdits { false };
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
