#include "SquidFonts.h"
#include <BinaryData.h>

namespace
{
    juce::Typeface::Ptr loadTypeface (const char* data, int size)
    {
        return juce::Typeface::createSystemTypefaceFor (data, static_cast<size_t> (size));
    }
}

SquidFonts::SquidFonts ()
    : sansRegular (loadTypeface (BinaryData::IBMPlexSansRegular_ttf, BinaryData::IBMPlexSansRegular_ttfSize)),
      condensedRegular (loadTypeface (BinaryData::IBMPlexSansCondensedRegular_ttf, BinaryData::IBMPlexSansCondensedRegular_ttfSize)),
      condensedSemiBold (loadTypeface (BinaryData::IBMPlexSansCondensedSemiBold_ttf, BinaryData::IBMPlexSansCondensedSemiBold_ttfSize)),
      monoRegular (loadTypeface (BinaryData::IBMPlexMonoRegular_ttf, BinaryData::IBMPlexMonoRegular_ttfSize))
{
}

SquidFonts::~SquidFonts ()
{
    clearSingletonInstance ();
}

juce::Typeface::Ptr SquidFonts::getTypeface (Face face) const
{
    switch (face)
    {
        // The mockup sets a few values in the 500 weight of Sans and Mono. Those
        // cuts are not embedded, so they fall back to the regular weight; the
        // colour those values are drawn in already sets them apart.
        case Face::sans:
        case Face::sansMedium:        return sansRegular;
        case Face::condensed:         return condensedRegular;
        case Face::condensedSemiBold: return condensedSemiBold;
        case Face::mono:
        case Face::monoMedium:        return monoRegular;
    }
    jassertfalse;
    return sansRegular;
}

juce::Typeface::Ptr SquidFonts::getDefaultTypeface ()
{
    return getInstance ()->sansRegular;
}

juce::Font SquidFonts::make (Face face, float emSize, float letterSpacingEm)
{
    juce::Font font { juce::FontOptions (getInstance ()->getTypeface (face)).withPointHeight (emSize) };
    if (juce::approximatelyEqual (letterSpacingEm, 0.0f))
        return font;

    // tracking is applied as a fraction of the JUCE height, so scale the em
    // spacing by how tall this face is per em
    const auto heightPerEm { font.getHeight () / emSize };
    font.setExtraKerningFactor (letterSpacingEm / heightPerEm);
    return font;
}

namespace SquidType
{
    using Face = SquidFonts::Face;

    juce::Font paneTitle ()         { return SquidFonts::make (Face::condensedSemiBold, 10.5f, 0.13f); }
    juce::Font sectionHeader ()     { return SquidFonts::make (Face::condensedSemiBold, 9.5f, 0.14f); }
    juce::Font parameterLabel ()    { return SquidFonts::make (Face::condensedSemiBold, 10.0f, 0.10f); }
    juce::Font cuePointLabel ()     { return SquidFonts::make (Face::condensedSemiBold, 9.5f, 0.10f); }
    juce::Font button ()            { return SquidFonts::make (Face::condensedSemiBold, 10.5f, 0.10f); }
    juce::Font buttonSmall ()       { return SquidFonts::make (Face::condensedSemiBold, 10.0f, 0.10f); }
    juce::Font channelTab ()        { return SquidFonts::make (Face::condensedSemiBold, 11.5f, 0.09f); }
    juce::Font cvTab ()             { return SquidFonts::make (Face::condensedSemiBold, 10.0f, 0.10f); }
    juce::Font transport ()         { return SquidFonts::make (Face::condensedSemiBold, 10.0f, 0.11f); }
    juce::Font cvFieldLabel ()      { return SquidFonts::make (Face::condensedSemiBold, 10.0f, 0.10f); }

    juce::Font chip ()              { return SquidFonts::make (Face::condensed, 10.5f, 0.08f); }
    juce::Font mini ()              { return SquidFonts::make (Face::condensed, 10.0f, 0.07f); }
    juce::Font statusTag ()         { return SquidFonts::make (Face::condensed, 10.0f, 0.09f); }
    juce::Font caption ()           { return SquidFonts::make (Face::condensed, 8.5f, 0.11f); }
    juce::Font markerLabel ()       { return SquidFonts::make (Face::condensed, 9.0f, 0.10f); }
    juce::Font menuSectionHeader () { return SquidFonts::make (Face::condensed, 9.5f, 0.13f); }

    juce::Font value ()             { return SquidFonts::make (Face::mono, 12.0f); }
    juce::Font nameField ()         { return SquidFonts::make (Face::mono, 12.5f, 0.02f); }
    juce::Font fileName ()          { return SquidFonts::make (Face::mono, 12.5f); }
    juce::Font meta ()              { return SquidFonts::make (Face::mono, 11.0f); }
    juce::Font cvValue ()           { return SquidFonts::make (Face::mono, 11.0f); }
    juce::Font cueChip ()           { return SquidFonts::make (Face::mono, 9.5f); }
    juce::Font cueChipActive ()     { return SquidFonts::make (Face::monoMedium, 9.5f); }
    juce::Font ruler ()             { return SquidFonts::make (Face::mono, 9.0f); }
    juce::Font bankNumber ()        { return SquidFonts::make (Face::mono, 10.5f); }
    juce::Font count ()             { return SquidFonts::make (Face::mono, 10.0f); }
    juce::Font chipValue ()         { return SquidFonts::make (Face::monoMedium, 10.5f); }
    juce::Font unit ()              { return SquidFonts::make (Face::mono, 10.0f); }
    juce::Font glyph ()             { return SquidFonts::make (Face::mono, 13.0f); }

    juce::Font body ()              { return SquidFonts::make (Face::sans, 12.0f); }
    juce::Font bodyStrong ()        { return SquidFonts::make (Face::sansMedium, 12.0f); }
    juce::Font statusMessage ()     { return SquidFonts::make (Face::sans, 11.5f); }
}
