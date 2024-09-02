#include "FileSelectLabel.h"

FileSelectLabel::FileSelectLabel ()
{
    mouseEavesDropper.onMouseDown = [this] (const juce::MouseEvent& mouseEvent)
    {
        if (mouseEvent.mods.isPopupMenu ())
        {
            if (onPopupMenuCallback != nullptr)
                onPopupMenuCallback ();
        }
        else
        {
            browseForSample ();
        }
    };
    addMouseListener (&mouseEavesDropper, true);
}

FileSelectLabel::~FileSelectLabel ()
{
    removeMouseListener (&mouseEavesDropper);
}

void FileSelectLabel::setOutline (juce::Colour colour)
{
    outlineColor = colour;
}

void FileSelectLabel::canMultiSelect (bool canDoMultiSelect)
{
    fileChooserOptions = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | (canDoMultiSelect ? juce::FileBrowserComponent::canSelectMultipleItems : 0);
}

void FileSelectLabel::browseForSample ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the Squid Salmple file you want to load...", {}, "*.wav"));
    fileChooser->launchAsync (fileChooserOptions, [this] (const juce::FileChooser& fc) mutable
    {
        if (fc.getURLResults ().size () > 0 && fc.getURLResults () [0].isLocalFile () && onFilesSelected != nullptr)
        {
            for (auto urlResult : fc.getURLResults ())
            {
                if (! urlResult.isLocalFile ())
                    return;
                juce::File fileToLoad (urlResult.getLocalFile ().getFullPathName ());
                if (fileToLoad.isDirectory ())
                    return;
            }

            juce::StringArray files;
            for (auto urlResult : fc.getURLResults ())
                files.add (urlResult.getLocalFile ().getFullPathName ());
            onFilesSelected (files);
        }
    }, nullptr);
}

void FileSelectLabel::paintOverChildren (juce::Graphics& g)
{
    juce::Label::paintOverChildren (g);
    g.setColour (outlineColor);
    g.drawRect (getLocalBounds ());
}
