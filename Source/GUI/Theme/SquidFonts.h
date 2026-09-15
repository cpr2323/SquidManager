#pragma once

#include <JuceHeader.h>

/*
    The app's type: IBM Plex, in the three families the mockup uses. Plex was
    drawn for technical interfaces, and its condensed cut sets the small caps
    labels at a width the parameter grid can afford.

    Every size here is the mockup's CSS font-size, which is the em size, so it
    goes through withPointHeight. withHeight would read it as ascent + descent,
    which for Plex is 1.3 em, and draw everything about a quarter too small.

    Letter spacing is also given in em, as CSS gives it. JUCE's tracking is a
    fraction of the JUCE height rather than of the em, so it is converted using
    the typeface's own metrics rather than passed straight through.
*/
class SquidFonts : private juce::DeletedAtShutdown
{
public:
    enum class Face
    {
        sans,
        sansMedium,
        condensed,
        condensedSemiBold,
        mono,
        monoMedium
    };

    static juce::Font make (Face face, float emSize, float letterSpacingEm = 0.0f);

    // the regular sans face, which the LookAndFeel installs as the default for
    // anything that does not ask for a font of its own
    static juce::Typeface::Ptr getDefaultTypeface ();

    ~SquidFonts () override;

    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (SquidFonts)

private:
    SquidFonts ();
    juce::Typeface::Ptr getTypeface (Face face) const;

    // Held here rather than in function statics, so they are released with the
    // rest of JUCE at shutdown instead of after it.
    juce::Typeface::Ptr sansRegular;
    juce::Typeface::Ptr condensedRegular;
    juce::Typeface::Ptr condensedSemiBold;
    juce::Typeface::Ptr monoRegular;
};

/*
    One function per role, so a size or spacing is decided in one place. The
    names say where a style is used, not what it looks like.
*/
namespace SquidType
{
    // condensed semi bold, upper case
    juce::Font paneTitle ();        // FOLDERS, BANKS
    juce::Font sectionHeader ();    // BANK, SAMPLE, LEVEL & ENV, CUE POINTS, CV ASSIGN
    juce::Font parameterLabel ();   // LEVEL, ATTACK ...
    juce::Font cuePointLabel ();    // START, LOOP, END beside their fields
    juce::Font button ();           // BANK TOOLS, SAVE BANK
    juce::Font buttonSmall ();      // CHANNEL TOOLS
    juce::Font channelTab ();       // CH 1 ... CH 8
    juce::Font cvTab ();            // CV 1 ... CV 7
    juce::Font transport ();        // ONCE, LOOP
    juce::Font cvFieldLabel ();     // ATN, OFS

    // condensed regular
    juce::Font chip ();             // OUT, SETTINGS
    juce::Font mini ();             // OPEN, NEW, ALL
    juce::Font statusTag ();        // UNSAVED EDITS
    juce::Font caption ();          // the loop tuner's END / START
    juce::Font markerLabel ();      // the waveform marker labels
    juce::Font menuSectionHeader ();

    // mono
    juce::Font value ();            // parameter fields and combo boxes
    juce::Font nameField ();        // the bank name
    juce::Font fileName ();         // the sample file chip
    juce::Font meta ();             // sample length and sample count
    juce::Font cvValue ();          // ATN / OFS values
    juce::Font cueChip ();
    juce::Font cueChipActive ();
    juce::Font ruler ();            // the timeline over the waveform
    juce::Font bankNumber ();
    juce::Font count ();            // 31/32 in the banks header
    juce::Font chipValue ();        // the device name in the OUT chip
    juce::Font unit ();             // kHz after a value
    juce::Font glyph ();            // + and - on the cue set tools

    // sans
    juce::Font body ();             // list rows, breadcrumbs, popup menus
    juce::Font bodyStrong ();       // the current folder in the breadcrumbs
    juce::Font statusMessage ();    // the bottom status bar
}
