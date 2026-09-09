#include "SquidLookAndFeel.h"

SquidLookAndFeel::SquidLookAndFeel ()
{
    applyPalette ();
}

void SquidLookAndFeel::setGround (float newGround)
{
    palette.setGround (newGround);
    applyPalette ();
}

void SquidLookAndFeel::applyPalette ()
{
    for (const auto& [colourId, colour] : palette.getColours ())
        setColour (colourId, colour);
}

void SquidLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int buttonX, int buttonY, int buttonW, int buttonH,
                                     juce::ComboBox& box)
{
    if (! box.getProperties ().contains (SquidLnFProperties::noComboBoxArrow))
    {
        juce::LookAndFeel_V4::drawComboBox (g, width, height, isButtonDown,
                                            buttonX, buttonY, buttonW, buttonH, box);
        return;
    }

    const auto cornerSize { box.findParentComponentOfClass<juce::ChoicePropertyComponent> () != nullptr ? 0.0f : 3.0f };
    const juce::Rectangle<int> boxBounds { 0, 0, width, height };

    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (boxBounds.toFloat (), cornerSize);

    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (boxBounds.toFloat ().reduced (0.5f, 0.5f), cornerSize, 1.0f);
}

void SquidLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    if (! box.getProperties ().contains (SquidLnFProperties::noComboBoxArrow))
    {
        juce::LookAndFeel_V4::positionComboBoxText (box, label);
        return;
    }

    label.setBounds (0, 0, box.getWidth (), box.getHeight ());
    label.setFont (getComboBoxFont (box));
}

juce::Font SquidLookAndFeel::getTextButtonFont (juce::TextButton& button, int buttonHeight)
{
    if (button.getProperties ().contains (SquidLnFProperties::singleGlyphButton))
        return juce::Font (juce::FontOptions (11.0f));

    return juce::LookAndFeel_V4::getTextButtonFont (button, buttonHeight);
}

void SquidLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                       bool isMouseOver, bool isButtonDown)
{
    if (! button.getProperties ().contains (SquidLnFProperties::singleGlyphButton))
    {
        juce::LookAndFeel_V4::drawButtonText (g, button, isMouseOver, isButtonDown);
        return;
    }

    // The default reserves an indent on each side of the text, which on these 15
    // pixel wide buttons leaves only 5 pixels for the glyph - narrower than the
    // '+' needs, so it gets squashed to the minimum horizontal scale and becomes
    // unreadable. These hold a single character, so use the full width.
    g.setFont (getTextButtonFont (button, button.getHeight ()));
    g.setColour (button.findColour (button.getToggleState () ? juce::TextButton::textColourOnId
                                                             : juce::TextButton::textColourOffId)
                       .withMultipliedAlpha (button.isEnabled () ? 1.0f : 0.5f));
    g.drawText (button.getButtonText (), button.getLocalBounds (), juce::Justification::centred, false);
}
