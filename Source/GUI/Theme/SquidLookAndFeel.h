#pragma once

#include <JuceHeader.h>
#include "Palette.h"

/*
    Tags a component can set on itself to ask for a drawing variation, rather than
    being given a LookAndFeel of its own.

    A component with its own LookAndFeel resolves every colour through that object,
    so a second LookAndFeel is also a second palette - and one that nothing updates.
    Keeping the variations here means there is exactly one LookAndFeel in the app,
    and therefore exactly one set of colours.
*/
namespace SquidLnFProperties
{
    // combo box drawn without the drop-down arrow, text across the full width
    static inline const juce::Identifier noComboBoxArrow { "noComboBoxArrow" };
    // text button holding a single character, drawn without the usual side indent
    static inline const juce::Identifier singleGlyphButton { "singleGlyphButton" };
}

/*
    Installs a Palette into JUCE's colour lookup.

    ThemeController sets this as the default LookAndFeel, so every component in
    the app - including popup menus and dialogs, which have no parent to inherit
    from - resolves its colours through here.
*/
class SquidLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SquidLookAndFeel ();

    void setGround (float newGround);
    float getGround () const noexcept { return palette.getGround (); }

    const Palette& getPalette () const noexcept { return palette; }

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;
    juce::Font getComboBoxFont (juce::ComboBox& box) override;
    juce::Font getPopupMenuFont () override;
    int getTabButtonBestWidth (juce::TabBarButton& button, int tabDepth) override;
    juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override;
    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isMouseOver, bool isButtonDown) override;

private:
    void applyPalette ();

    Palette palette;
};
