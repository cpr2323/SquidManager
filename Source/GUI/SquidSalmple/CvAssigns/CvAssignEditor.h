#pragma once
#include <JuceHeader.h>
#include "CvAssignSection.h"
#include "../../Theme/SquidColourIds.h"
#include "../../Theme/UiComponents.h"
#include "../../../SquidSalmple/SquidChannelProperties.h"

class CvAssignEditor : public juce::Component
{
public:
    CvAssignEditor ();
    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT);
    void setEnableState (int cvParameterId, bool enabled);

    static constexpr int kNumCvAssigns { 7 };
    // the header row this editor draws for itself, so the owner can allow for it
    static constexpr int kHeaderHeight { 22 };

private:
    /*
        One tab per CV input, with a led that lights when that CV has any routing
        at all. Replaces the pair of arrows and an index, which said how to change
        the selection but nothing about what was assigned.
    */
    class CvTabButton : public juce::Button
    {
    public:
        explicit CvTabButton (int theCvIndex)
            : juce::Button ({}), cvIndex (theCvIndex)
        {
            setClickingTogglesState (false);
        }

        void setSelected (bool nowSelected) { selected = nowSelected; repaint (); }
        void setHasRouting (bool nowHasRouting)
        {
            if (hasRouting == nowHasRouting)
                return;
            hasRouting = nowHasRouting;
            repaint ();
        }

        void paintButton (juce::Graphics& g, bool isMouseOver, bool) override
        {
            auto bounds { getLocalBounds () };
            if (selected)
            {
                g.setColour (findColour (SquidColours::selectedRow));
                g.fillRect (bounds);
                g.setColour (findColour (SquidColours::accent));
                g.fillRect (bounds.removeFromBottom (2));
            }

            const auto ledBounds { getLocalBounds ().withTrimmedLeft (7).withWidth (7)
                                                    .withSizeKeepingCentre (7, 7).toFloat () };
            StatusLed::draw (g, ledBounds, hasRouting,
                             findColour (SquidColours::markerStart), findColour (SquidColours::outline));

            g.setFont (juce::Font (juce::FontOptions (12.0f)));
            g.setColour (findColour (selected ? SquidColours::text
                                              : (isMouseOver ? SquidColours::textDim : SquidColours::menuHeaderText)));
            g.drawText ("CV " + juce::String (cvIndex + 1), getLocalBounds ().withTrimmedLeft (20),
                        juce::Justification::centredLeft, false);

            g.setColour (findColour (SquidColours::outline));
            g.drawVerticalLine (getWidth () - 1, 3.0f, static_cast<float> (getHeight () - 3));
        }

    private:
        int cvIndex;
        bool selected { false };
        bool hasRouting { false };
    };

    SquidChannelProperties squidChannelProperties;
    juce::Label cvAssignHeaderLabel;
    std::array<std::unique_ptr<CvTabButton>, kNumCvAssigns> cvTabs;
    std::array<CvAssignSection, kNumCvAssigns> cvAssignSectionList;
    int curCvAssignIndex { 0 };

    void refreshRoutingLed (int cvIndex);
    void applyExplicitColours ();
    void paint (juce::Graphics& g) override;
    void resized () override;
    void lookAndFeelChanged () override;
    void selectCvAssigns (int cvSelectButtonIndex);
};
