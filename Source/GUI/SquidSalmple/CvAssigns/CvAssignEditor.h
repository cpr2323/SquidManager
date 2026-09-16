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
    // the header band this editor draws for itself, and the matrix under it, so
    // the owner can size the card around them
    static constexpr int kHeaderHeight { 34 };
    static constexpr int kMatrixHeight { 91 };

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

        static juce::String getLabel (int cvIndex) { return "CV " + juce::String (cvIndex + 1); }
        static int getIdealWidth (int cvIndex)
        {
            return kPadding + static_cast<int> (StatusLed::kDiameter) + kLedGap + SquidPaint::textWidth (SquidType::cvTab (), getLabel (cvIndex)) + kPadding;
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
            const auto bounds { getLocalBounds () };
            const auto hovered { isMouseOver && ! selected };

            g.fillAll (findColour (selected ? SquidColours::windowBackground : SquidColours::buttonBackground));

            auto content { bounds.withTrimmedLeft (kPadding) };
            const auto ledBounds { content.removeFromLeft (static_cast<int> (StatusLed::kDiameter)).toFloat ()
                                          .withSizeKeepingCentre (StatusLed::kDiameter, StatusLed::kDiameter) };
            StatusLed::draw (g, ledBounds, hasRouting, *this);
            content.removeFromLeft (kLedGap);

            g.setFont (SquidType::cvTab ());
            g.setColour (findColour (selected ? SquidColours::text
                                              : (hovered ? SquidColours::textDim : SquidColours::menuHeaderText)));
            g.drawText (getLabel (cvIndex), content, juce::Justification::centredLeft, false);

            if (selected)
            {
                g.setColour (findColour (SquidColours::accent));
                g.fillRect (bounds.withTop (bounds.getBottom () - 2));
            }
            else if (hovered)
            {
                g.setColour (findColour (SquidColours::accentDeep));
                g.drawRect (bounds, 1);
            }
        }

    private:
        static constexpr int kPadding { 10 };
        static constexpr int kLedGap { 6 };

        int cvIndex;
        bool selected { false };
        bool hasRouting { false };
    };

    SquidChannelProperties squidChannelProperties;
    juce::Label cvAssignHeaderLabel;
    std::array<std::unique_ptr<CvTabButton>, kNumCvAssigns> cvTabs;
    // tab labels never change, so they are measured once rather than on every layout
    std::array<int, kNumCvAssigns> cvTabWidths {};
    std::array<CvAssignSection, kNumCvAssigns> cvAssignSectionList;
    // Where a section goes. Only the section on screen is given it in resized; the
    // others are given it when their tab is picked. Each section holds 16 columns of
    // controls, so laying out the six hidden ones on every resize was most of the cost
    // of resizing the editor.
    juce::Rectangle<int> sectionBounds;
    int curCvAssignIndex { 0 };

    // set in resized, drawn in paint: the outline around the joined tab group
    juce::Rectangle<int> tabGroupBounds;

    void refreshRoutingLed (int cvIndex);
    void applyExplicitColours ();
    void paint (juce::Graphics& g) override;
    void resized () override;
    void lookAndFeelChanged () override;
    void selectCvAssigns (int cvSelectButtonIndex);
};
