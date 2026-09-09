#pragma once

#include <JuceHeader.h>
#include "SquidColourIds.h"

/*
    Small pieces the app repeats often enough that they should only exist once.
    All of them resolve their colours at paint time, so they follow the palette
    without needing to be told a change happened.
*/

/*
    The module reports itself with LEDs, so the app does too: a hollow ring for
    "nothing here", a lit dot for "has content". Two states only - anything more
    would be inventing information the app does not have.
*/
class StatusLed : public juce::Component
{
public:
    StatusLed () { setInterceptsMouseClicks (false, false); }

    void setLit (bool shouldBeLit)
    {
        if (lit == shouldBeLit)
            return;
        lit = shouldBeLit;
        repaint ();
    }
    bool isLit () const noexcept { return lit; }

    void paint (juce::Graphics& g) override
    {
        const auto bounds { getLocalBounds ().toFloat ().reduced (0.5f) };
        if (lit)
        {
            g.setColour (findColour (SquidColours::markerStart));
            g.fillEllipse (bounds);
        }
        else
        {
            g.setColour (findColour (SquidColours::outline));
            g.drawEllipse (bounds, 1.0f);
        }
    }

    // for painting a led inline, where a child component would be awkward
    static void draw (juce::Graphics& g, juce::Rectangle<float> bounds, bool lit,
                      juce::Colour litColour, juce::Colour unlitColour)
    {
        if (lit)
        {
            g.setColour (litColour);
            g.fillEllipse (bounds);
        }
        else
        {
            g.setColour (unlitColour);
            g.drawEllipse (bounds, 1.0f);
        }
    }

private:
    bool lit { false };
};

/*
    The strip that names a pane - a title in the accent colour, an optional count
    on the right, and room for a few small buttons in between.
*/
class PaneHeader : public juce::Component
{
public:
    explicit PaneHeader (juce::String headerTitle)
        : title (std::move (headerTitle))
    {
    }

    void setCountText (juce::String newCountText)
    {
        if (countText == newCountText)
            return;
        countText = std::move (newCountText);
        repaint ();
    }

    // where buttons may be placed, between the title and the count
    juce::Rectangle<int> getFreeBounds () const
    {
        return getLocalBounds ().withTrimmedLeft (titleWidth)
                                .withTrimmedRight (countText.isEmpty () ? 6 : 46)
                                .reduced (0, 4);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (findColour (SquidColours::panelHeader));
        g.setColour (findColour (SquidColours::outline));
        g.drawHorizontalLine (getHeight () - 1, 0.0f, static_cast<float> (getWidth ()));

        g.setFont (juce::Font (juce::FontOptions (11.0f)));
        g.setColour (findColour (SquidColours::accentText));
        g.drawText (title, 8, 0, titleWidth, getHeight (), juce::Justification::centredLeft, false);

        if (countText.isNotEmpty ())
        {
            g.setColour (findColour (SquidColours::textDim));
            g.drawText (countText, getWidth () - 56, 0, 48, getHeight (), juce::Justification::centredRight, false);
        }
    }

private:
    juce::String title;
    juce::String countText;
    static constexpr int titleWidth { 56 };
};

/*
    A button that opens a menu, and says so. The background comes from the
    LookAndFeel so it matches every other button; only the arrow is extra.
*/
class MenuButton : public juce::TextButton
{
public:
    explicit MenuButton (juce::String text) { setButtonText (text); }

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
    {
        getLookAndFeel ().drawButtonBackground (g, *this, findColour (juce::TextButton::buttonColourId),
                                                isMouseOver, isMouseDown);
        auto bounds { getLocalBounds () };
        auto arrowArea { bounds.removeFromRight (16) };

        g.setColour (findColour (isEnabled () ? SquidColours::textDim : SquidColours::outlineDim));
        g.setFont (juce::Font (juce::FontOptions (11.0f)));
        g.drawText (getButtonText (), bounds.withTrimmedLeft (9), juce::Justification::centredLeft, false);

        juce::Path arrow;
        const auto centreX { static_cast<float> (arrowArea.getCentreX ()) };
        const auto centreY { static_cast<float> (arrowArea.getCentreY ()) };
        arrow.addTriangle (centreX - 3.5f, centreY - 2.0f, centreX + 3.5f, centreY - 2.0f, centreX, centreY + 2.5f);
        g.fillPath (arrow);
    }
};

/*
    A small outlined button for the chrome: the pane tools, and the chips in the
    path bar. TextButton with the app's own padding and no rounded fill.
*/
class ChromeButton : public juce::TextButton
{
public:
    explicit ChromeButton (juce::String text) { setButtonText (text); }

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
    {
        const auto bounds { getLocalBounds ().toFloat ().reduced (0.5f) };
        const auto toggledOn { getToggleState () };

        // an on state has to be visible without hovering it, so it takes the accent
        g.setColour (findColour (toggledOn ? SquidColours::selectedRow
                                      : (isMouseDown ? SquidColours::buttonBackground
                                                     : SquidColours::fieldBackground)));
        g.fillRoundedRectangle (bounds, 2.0f);

        g.setColour (findColour (toggledOn || isMouseOver ? SquidColours::accentDeep : SquidColours::outline));
        g.drawRoundedRectangle (bounds, 2.0f, 1.0f);

        g.setFont (juce::Font (juce::FontOptions (11.0f)));
        g.setColour (findColour (! isEnabled () ? SquidColours::outlineDim
                                                : (toggledOn ? SquidColours::accentText : SquidColours::textDim)));
        g.drawText (getButtonText (), getLocalBounds (), juce::Justification::centred, false);
    }
};
