#include "SquidLookAndFeel.h"
#include "SquidFonts.h"
#include "UiComponents.h"

namespace
{
    constexpr auto kFieldCornerSize { 2.0f };
    constexpr auto kFieldPadding { 7 };
    constexpr auto kComboCaretWidth { 7.0f };

    // A field shows the pointer the same way every other control does.
    bool isHovered (const juce::Component& component)
    {
        return component.isEnabled () && component.isMouseOver (true);
    }

    constexpr auto kTooltipMaxWidth { 400.0f };
    constexpr auto kTooltipPaddingX { 8 };
    constexpr auto kTooltipPaddingY { 5 };

    juce::TextLayout layoutTooltip (const juce::String& text, juce::Colour colour)
    {
        juce::AttributedString attributedText;
        attributedText.setJustification (juce::Justification::centredLeft);
        attributedText.append (text, SquidType::body (), colour);

        juce::TextLayout layout;
        layout.createLayoutWithBalancedLineLengths (attributedText, kTooltipMaxWidth);
        return layout;
    }
}

SquidLookAndFeel::SquidLookAndFeel ()
{
    applyPalette ();
    // anything that does not ask for a font of its own is set in Plex Sans
    setDefaultSansSerifTypeface (SquidFonts::getDefaultTypeface ());
}

void SquidLookAndFeel::setBackground (float newBackground)
{
    palette.setBackground (newBackground);
    applyPalette ();
}

void SquidLookAndFeel::applyPalette ()
{
    for (const auto& [colourId, colour] : palette.getColours ())
        setColour (colourId, colour);
}

//==============================================================================
void SquidLookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    // an unusable field sinks into the panel rather than staying recessed
    const auto background { textEditor.isEnabled () ? textEditor.findColour (juce::TextEditor::backgroundColourId)
                                                    : textEditor.findColour (SquidColours::listBackground) };
    g.setColour (background);
    g.fillRoundedRectangle (juce::Rectangle<int> { 0, 0, width, height }.toFloat (), kFieldCornerSize);
}

void SquidLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    const auto colourId { ! textEditor.isEnabled () ? SquidColours::outlineDim
                                                    : (textEditor.hasKeyboardFocus (true) || isHovered (textEditor)
                                                          ? SquidColours::accentDeep
                                                          : SquidColours::outline) };
    g.setColour (textEditor.findColour (colourId));
    g.drawRoundedRectangle (juce::Rectangle<int> { 0, 0, width, height }.toFloat ().reduced (0.5f), kFieldCornerSize, 1.0f);
}

void SquidLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                     int, int, int, int, juce::ComboBox& box)
{
    const auto area { juce::Rectangle<int> { 0, 0, width, height }.toFloat () };
    const auto enabled { box.isEnabled () };
    const auto hovered { isHovered (box) };

    g.setColour (box.findColour (enabled ? juce::ComboBox::backgroundColourId : SquidColours::listBackground));
    g.fillRoundedRectangle (area, kFieldCornerSize);

    const auto outlineId { ! enabled ? SquidColours::outlineDim
                                     : (hovered || box.isPopupActive () ? SquidColours::accentDeep
                                                                        : SquidColours::outline) };
    g.setColour (box.findColour (outlineId));
    g.drawRoundedRectangle (area.reduced (0.5f), kFieldCornerSize, 1.0f);

    // a unit, such as kHz, sits just after the value
    if (const auto unit { box.getProperties () [SquidLnFProperties::valueUnit].toString () }; unit.isNotEmpty ())
    {
        const auto valueWidth { SquidPaint::textWidth (getComboBoxFont (box), box.getText ()) };
        g.setFont (SquidType::unit ());
        g.setColour (box.findColour (SquidColours::textGhost));
        g.drawText (unit, juce::Rectangle<int> { kFieldPadding + valueWidth + 3, 0, width, height },
                    juce::Justification::centredLeft, false);
    }

    g.setColour (box.findColour (hovered ? SquidColours::accent : SquidColours::menuHeaderText));
    SquidPaint::caretDown (g, { static_cast<float> (width) - 5.0f - (kComboCaretWidth * 0.5f), (static_cast<float> (height) * 0.5f) + 0.5f },
                           kComboCaretWidth);
}

void SquidLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBorderSize ({ 0, 0, 0, 0 });
    label.setBounds (kFieldPadding, 0, box.getWidth () - kFieldPadding - 5 - static_cast<int> (kComboCaretWidth) - 2, box.getHeight ());
    label.setJustificationType (juce::Justification::centredLeft);
    label.setFont (getComboBoxFont (box));
}

juce::Font SquidLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return SquidType::value ();
}

//==============================================================================
juce::Font SquidLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return SquidType::button ();
}

void SquidLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    const auto area { button.getLocalBounds ().toFloat ().reduced (0.5f) };
    const auto hovered { button.isEnabled () && (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown) };

    g.setColour (backgroundColour);
    g.fillRoundedRectangle (area, kFieldCornerSize);
    g.setColour (button.findColour (hovered ? SquidColours::accentDeep : SquidColours::outline));
    g.drawRoundedRectangle (area, kFieldCornerSize, 1.0f);
}

//==============================================================================
int SquidLookAndFeel::getTabButtonBestWidth (juce::TabBarButton& button, int tabDepth)
{
    if (! button.getTabbedButtonBar ().getProperties ().contains (SquidLnFProperties::tabsHaveLeds))
        return juce::LookAndFeel_V4::getTabButtonBestWidth (button, tabDepth);

    // padding, led, gap, name, padding
    return 13 + static_cast<int> (StatusLed::kDiameter) + 7 + SquidPaint::textWidth (SquidType::channelTab (), button.getButtonText ()) + 13;
}

int SquidLookAndFeel::getTabButtonOverlap (int)
{
    // tabs sit edge to edge, divided by their own hairline
    return 0;
}

void SquidLookAndFeel::drawTabbedButtonBarBackground (juce::TabbedButtonBar& buttonBar, juce::Graphics& g)
{
    g.fillAll (buttonBar.findColour (SquidColours::tabBackground));
    g.setColour (buttonBar.findColour (SquidColours::outline));
    g.drawHorizontalLine (buttonBar.getHeight () - 1, 0.0f, static_cast<float> (buttonBar.getWidth ()));
}

void SquidLookAndFeel::drawTabAreaBehindFrontButton (juce::TabbedButtonBar&, juce::Graphics&, int, int)
{
    // the front tab marks itself with its own underline, so there is no shadow behind it
}

//==============================================================================
juce::Font SquidLookAndFeel::getPopupMenuFont ()
{
    return SquidType::body ();
}

void SquidLookAndFeel::drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& sectionName)
{
    g.setFont (SquidType::menuSectionHeader ());
    g.setColour (findColour (juce::PopupMenu::headerTextColourId));
    // sits slightly low in its row, so it reads as belonging to the items below it
    g.drawText (sectionName.toUpperCase (), area.reduced (11, 0).withTrimmedTop (3), juce::Justification::centredLeft, true);
}

void SquidLookAndFeel::getIdealPopupMenuSectionHeaderSizeWithOptions (const juce::String& text, int,
                                                                      int& idealWidth, int& idealHeight,
                                                                      const juce::PopupMenu::Options&)
{
    // the default is one and a half item heights, which leaves a header floating
    // in empty space; the header text plus a little room is enough
    const auto font { SquidType::menuSectionHeader () };
    idealHeight = juce::roundToInt (font.getHeight ()) + 9;
    idealWidth = SquidPaint::textWidth (font, text.toUpperCase ()) + 22;
}

//==============================================================================
juce::Rectangle<int> SquidLookAndFeel::getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea)
{
    const auto layout { layoutTooltip (tipText, juce::Colours::black) };
    const auto width { juce::roundToInt (std::ceil (layout.getWidth ())) + (kTooltipPaddingX * 2) };
    const auto height { juce::roundToInt (std::ceil (layout.getHeight ())) + (kTooltipPaddingY * 2) };

    // placed away from the pointer, on whichever side of it there is more room
    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX () ? screenPos.x - (width + 12) : screenPos.x + 24,
                                 screenPos.y > parentArea.getCentreY () ? screenPos.y - (height + 6) : screenPos.y + 6,
                                 width, height)
               .constrainedWithin (parentArea);
}

void SquidLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    const juce::Rectangle<int> bounds { 0, 0, width, height };
    g.fillAll (findColour (juce::TooltipWindow::backgroundColourId));
    g.setColour (findColour (juce::TooltipWindow::outlineColourId));
    g.drawRect (bounds, 1);

    layoutTooltip (text, findColour (juce::TooltipWindow::textColourId))
        .draw (g, bounds.reduced (kTooltipPaddingX, kTooltipPaddingY).toFloat ());
}

//==============================================================================
int SquidLookAndFeel::getDefaultScrollbarWidth ()
{
    return 8;
}

void SquidLookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height,
                                      bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                                      bool isMouseOver, bool isMouseDown)
{
    juce::Rectangle<int> thumbBounds { x, y, width, height };
    thumbBounds = isScrollbarVertical ? thumbBounds.withY (thumbStartPosition).withHeight (thumbSize)
                                      : thumbBounds.withX (thumbStartPosition).withWidth (thumbSize);
    const auto thumb { thumbBounds.toFloat ().reduced (1.0f) };

    g.setColour (scrollbar.findColour (isMouseOver || isMouseDown ? SquidColours::accentDeep : SquidColours::outlineStrong));
    g.fillRoundedRectangle (thumb, std::min (thumb.getWidth (), thumb.getHeight ()) * 0.5f);
}
