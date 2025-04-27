#include "CvAssignSection.h"
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
    const auto assignParameterWidth { static_cast<int> (getWidth () / getNumChildComponents ()) };
    for (auto curCvAssignParameter { 0 }; curCvAssignParameter < getNumChildComponents (); ++curCvAssignParameter)
    {
        auto curParameterComponent { dynamic_cast<CvAssignParameter*> (getChildComponent (curCvAssignParameter)) };
        curParameterComponent->setBounds (assignParameterWidth * curCvAssignParameter, 0, assignParameterWidth, getHeight ());
    }
}
