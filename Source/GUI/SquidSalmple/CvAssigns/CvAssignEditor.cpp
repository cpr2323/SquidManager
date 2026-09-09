#include "CvAssignEditor.h"
#include "../../../SquidSalmple/Metadata/SquidSalmpleDefs.h"

namespace
{
    // the parameters a CV can be routed to, in the order the columns are drawn
    const auto kRoutableParameterIds { std::vector<int>
    {
        CvParameterIndex::Level,    CvParameterIndex::Attack,     CvParameterIndex::Decay,
        CvParameterIndex::StartCue, CvParameterIndex::LoopCue,    CvParameterIndex::EndCue,
        CvParameterIndex::FiltFreq, CvParameterIndex::FiltRes,    CvParameterIndex::Reverse,
        CvParameterIndex::Speed,    CvParameterIndex::PitchShift, CvParameterIndex::Bits,
        CvParameterIndex::Rate,     CvParameterIndex::LoopMode,   CvParameterIndex::ETrig,
        CvParameterIndex::CueSet
    } };
}

CvAssignEditor::CvAssignEditor ()
{
    cvAssignHeaderLabel.setBorderSize ({ 0, 0, 0, 0 });
    cvAssignHeaderLabel.setJustificationType (juce::Justification::centredLeft);
    cvAssignHeaderLabel.setFont (cvAssignHeaderLabel.getFont ().withPointHeight (11.0f));
    cvAssignHeaderLabel.setText ("CV ASSIGN", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (cvAssignHeaderLabel);

    for (auto cvIndex { 0 }; cvIndex < kNumCvAssigns; ++cvIndex)
    {
        auto tab { std::make_unique<CvTabButton> (cvIndex) };
        tab->onClick = [this, cvIndex] () { selectCvAssigns (cvIndex); };
        addAndMakeVisible (tab.get ());
        cvTabs [static_cast<size_t> (cvIndex)] = std::move (tab);
    }

    for (auto& cvAssignSection : cvAssignSectionList)
        addChildComponent (cvAssignSection);

    applyExplicitColours ();
    selectCvAssigns (curCvAssignIndex);
}

void CvAssignEditor::init (juce::ValueTree rootPropertiesVT, juce::ValueTree channelPropertiesVT)
{
    for (auto cvIndex { 0 }; cvIndex < static_cast<int> (cvAssignSectionList.size ()); ++cvIndex)
        cvAssignSectionList [static_cast<size_t> (cvIndex)].init (rootPropertiesVT, channelPropertiesVT, cvIndex);

    // The tab leds follow the data rather than the controls, so they are right
    // however an assignment was changed.
    squidChannelProperties.wrap (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);
    squidChannelProperties.onCvAssignEnabledChange = [this] (int cvIndex, int, bool)
    {
        refreshRoutingLed (cvIndex);
    };

    for (auto cvIndex { 0 }; cvIndex < kNumCvAssigns; ++cvIndex)
        refreshRoutingLed (cvIndex);
}

void CvAssignEditor::refreshRoutingLed (int cvIndex)
{
    if (cvIndex < 0 || cvIndex >= kNumCvAssigns || ! squidChannelProperties.isValid ())
        return;

    auto hasRouting { false };
    for (const auto parameterId : kRoutableParameterIds)
    {
        if (squidChannelProperties.getCvAssignEnabled (cvIndex, parameterId))
        {
            hasRouting = true;
            break;
        }
    }
    cvTabs [static_cast<size_t> (cvIndex)]->setHasRouting (hasRouting);
}

void CvAssignEditor::setEnableState (int cvParameterId, bool enabled)
{
    for (auto& cvAssignSection : cvAssignSectionList)
        cvAssignSection.setEnableState (cvParameterId, enabled);
}

void CvAssignEditor::applyExplicitColours ()
{
    cvAssignHeaderLabel.setColour (juce::Label::ColourIds::textColourId, findColour (SquidColours::accentText));
}

void CvAssignEditor::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    applyExplicitColours ();
}

void CvAssignEditor::paint (juce::Graphics& g)
{
    // a hairline under the tab row, so the tabs read as belonging to the matrix
    g.setColour (findColour (SquidColours::outline));
    g.drawHorizontalLine (kHeaderHeight - 1, 0.0f, static_cast<float> (getWidth ()));
}

void CvAssignEditor::resized ()
{
    auto localBounds { getLocalBounds () };

    auto headerRow { localBounds.removeFromTop (kHeaderHeight) };
    cvAssignHeaderLabel.setBounds (headerRow.removeFromLeft (80));
    headerRow.removeFromLeft (6);
    for (auto& tab : cvTabs)
        tab->setBounds (headerRow.removeFromLeft (64));

    localBounds.removeFromTop (2);
    for (auto& cvAssignSection : cvAssignSectionList)
        cvAssignSection.setBounds (localBounds);
}

void CvAssignEditor::selectCvAssigns (int newCvAssignIndex)
{
    curCvAssignIndex = newCvAssignIndex;
    for (auto cvIndex { 0 }; cvIndex < kNumCvAssigns; ++cvIndex)
    {
        cvTabs [static_cast<size_t> (cvIndex)]->setSelected (cvIndex == curCvAssignIndex);
        cvAssignSectionList [static_cast<size_t> (cvIndex)].setVisible (cvIndex == curCvAssignIndex);
    }
}
