#pragma once
#include <JuceHeader.h>
#include "CvAssignSection.h"
#include "../../Theme/SquidColourIds.h"

class CvAssignEditor : public juce::Component
{
public:
    CvAssignEditor ();
    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT);
    void setEnableState (int cvParameterId, bool enabled);

private:
    /*
        juce::ArrowButton bakes its colour in at construction and offers no
        setter, so re-colouring one means building a new one - which on a theme
        change also means a fresh layout pass over everything below it. This
        draws the same arrow but resolves the colour when it paints, so a palette
        change costs nothing more than a repaint.
    */
    class ArrowSelectButton : public juce::Button
    {
    public:
        explicit ArrowSelectButton (float arrowDirectionInRadians)
            : juce::Button ({})
        {
            path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
            path.applyTransform (juce::AffineTransform::rotation (juce::MathConstants<float>::twoPi * arrowDirectionInRadians,
                                                                  0.5f, 0.5f));
        }

        void paintButton (juce::Graphics& g, bool, bool shouldDrawButtonAsDown) override
        {
            juce::Path drawnPath { path };
            const auto offset { shouldDrawButtonAsDown ? 1.0f : 0.0f };
            drawnPath.applyTransform (path.getTransformToScaleToFit (offset, offset,
                                                                     static_cast<float> (getWidth ()) - 3.0f,
                                                                     static_cast<float> (getHeight ()) - 3.0f, false));
            juce::DropShadow (juce::Colours::black.withAlpha (0.3f), shouldDrawButtonAsDown ? 2 : 4, {}).drawForPath (g, drawnPath);
            g.setColour (findColour (SquidColours::text).withAlpha (isEnabled () ? 1.0f : 0.5f));
            g.fillPath (drawnPath);
        }

    private:
        juce::Path path;
    };

    juce::Label curCvAssignIndexLabel;
    ArrowSelectButton upButton { 0.75f };
    ArrowSelectButton downButton { 0.25f };
    std::array<CvAssignSection, 7> cvAssignSectionList;
    int curCvAssignIndex { 0 };

    void paint (juce::Graphics& g) override;
    void resized () override;
    void selectCvAssigns (int cvSelectButtonIndex);
};
