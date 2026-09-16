#include "CvAssignSection.h"
#include "../../Theme/SquidColourIds.h"
#include "../../../SquidSalmple/Metadata/SquidSalmpleDefs.h"

const auto kParameterDisplayOrderList { std::vector<int>
{
    CvParameterIndex::Level,
    CvParameterIndex::Attack,
    CvParameterIndex::Decay,
    CvParameterIndex::StartCue,
    CvParameterIndex::LoopCue,
    CvParameterIndex::EndCue,
    CvParameterIndex::FiltFreq,
    CvParameterIndex::FiltRes,
    CvParameterIndex::Reverse,
    CvParameterIndex::Speed,
    CvParameterIndex::PitchShift,
    CvParameterIndex::Bits,
    CvParameterIndex::Rate,
    CvParameterIndex::LoopMode,
    CvParameterIndex::ETrig,
    CvParameterIndex::CueSet,
} };

CvAssignSection::CvAssignSection ()
{
    // create and add CvAssignParameter components for each parameters listed
    for ([[maybe_unused]] auto curParameterId : kParameterDisplayOrderList)
    {
        auto cvAssignParameter { std::make_unique<CvAssignParameter> () };
        addAndMakeVisible (cvAssignParameter.release ());
    }
}

CvAssignSection::~CvAssignSection ()
{
    for (auto curComponentIndex { getNumChildComponents () - 1 }; curComponentIndex >= 0; --curComponentIndex)
    {
        auto curParameterComponent { getChildComponent (curComponentIndex) };
        removeChildComponent (curParameterComponent);
        delete curParameterComponent;
    }
}

void CvAssignSection::init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT, int theCvIndex)
{
    for (auto curParameterComponentIndex { 0 }; curParameterComponentIndex < getNumChildComponents (); ++curParameterComponentIndex)
    {
        auto curParameterComponent { dynamic_cast<CvAssignParameter*> ( getChildComponent (curParameterComponentIndex)) };
        curParameterComponent->init (rootPropertiesVT, channelPropertiesVT, theCvIndex, kParameterDisplayOrderList [curParameterComponentIndex]);
    }
}

void CvAssignSection::setEnableState (int cvParameterId, bool enabled)
{
    for (auto curParameterComponentIndex { 0 }; curParameterComponentIndex < getNumChildComponents (); ++curParameterComponentIndex)
    {
        auto curParameterComponent { dynamic_cast<CvAssignParameter*> (getChildComponent (curParameterComponentIndex)) };
        if (curParameterComponent->getParameterId () == cvParameterId)
        {
            curParameterComponent->setEnabled (enabled);
            break;
        }
    }
}

void CvAssignSection::resized ()
{
    // the leftover pixels are shared out, so the last column meets the edge
    const auto assignParameterWidth { static_cast<float> (getWidth ()) / static_cast<float> (getNumChildComponents ()) };
    for (auto curCvAssignParameter { 0 }; curCvAssignParameter < getNumChildComponents (); ++curCvAssignParameter)
    {
        auto curParameterComponent { dynamic_cast<CvAssignParameter*> (getChildComponent (curCvAssignParameter)) };
        const auto left { juce::roundToInt (assignParameterWidth * static_cast<float> (curCvAssignParameter)) };
        const auto right { juce::roundToInt (assignParameterWidth * static_cast<float> (curCvAssignParameter + 1)) };
        curParameterComponent->setBounds (left, 0, right - left, getHeight ());
    }
}

void CvAssignSection::paintOverChildren (juce::Graphics& g)
{
    // quiet dividers between the columns, none after the last
    g.setColour (findColour (SquidColours::outlineDim));
    for (auto curCvAssignParameter { 1 }; curCvAssignParameter < getNumChildComponents (); ++curCvAssignParameter)
        g.fillRect (getChildComponent (curCvAssignParameter)->getX () - 1, 0, 1, getHeight ());
}
