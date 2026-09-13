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
    cvAssignHeaderLabel.setFont (SquidType::sectionHeader ());
    cvAssignHeaderLabel.setText ("CV ASSIGN", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (cvAssignHeaderLabel);

    for (auto cvIndex { 0 }; cvIndex < kNumCvAssigns; ++cvIndex)
    {
        auto tab { std::make_unique<CvTabButton> (cvIndex) };
        tab->onClick = [this, cvIndex] () { selectCvAssigns (cvIndex); };
        addAndMakeVisible (tab.get ());
        cvTabs [static_cast<size_t> (cvIndex)] = std::move (tab);
        cvTabWidths [static_cast<size_t> (cvIndex)] = CvTabButton::getIdealWidth (cvIndex);
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
    // the header band sits a step above the card, with a hairline under it
    g.setColour (findColour (SquidColours::panelHeader));
    g.fillRect (0, 0, getWidth (), kHeaderHeight - 1);
    g.setColour (findColour (SquidColours::outline));
    g.fillRect (0, kHeaderHeight - 1, getWidth (), 1);

    // the tabs are one joined control: an outline around the group, hairlines between
    g.fillRect (tabGroupBounds);
}

void CvAssignEditor::resized ()
{
    auto localBounds { getLocalBounds () };

    auto headerRow { localBounds.removeFromTop (kHeaderHeight).withTrimmedBottom (1).reduced (8, 5) };
    cvAssignHeaderLabel.setBounds (headerRow.removeFromLeft (SquidPaint::textWidth (SquidType::sectionHeader (), cvAssignHeaderLabel.getText ()) + 2));
    headerRow.removeFromLeft (9);

    // tabs are laid out one pixel apart inside a one pixel frame; the outline
    // painted behind them shows through as the frame and the dividers
    auto groupWidth { 1 };
    for (auto cvIndex { 0 }; cvIndex < kNumCvAssigns; ++cvIndex)
        groupWidth += cvTabWidths [static_cast<size_t> (cvIndex)] + 1;
    tabGroupBounds = headerRow.removeFromLeft (groupWidth);
    auto tabRow { tabGroupBounds.reduced (1) };
    for (auto cvIndex { 0 }; cvIndex < kNumCvAssigns; ++cvIndex)
    {
        cvTabs [static_cast<size_t> (cvIndex)]->setBounds (tabRow.removeFromLeft (cvTabWidths [static_cast<size_t> (cvIndex)]));
        tabRow.removeFromLeft (1);
    }

    sectionBounds = localBounds;
    cvAssignSectionList [static_cast<size_t> (curCvAssignIndex)].setBounds (sectionBounds);
}

void CvAssignEditor::selectCvAssigns (int newCvAssignIndex)
{
    curCvAssignIndex = newCvAssignIndex;
    for (auto cvIndex { 0 }; cvIndex < kNumCvAssigns; ++cvIndex)
    {
        cvTabs [static_cast<size_t> (cvIndex)]->setSelected (cvIndex == curCvAssignIndex);
        auto& cvAssignSection { cvAssignSectionList [static_cast<size_t> (cvIndex)] };
        if (cvIndex == curCvAssignIndex)
            cvAssignSection.setBounds (sectionBounds);
        cvAssignSection.setVisible (cvIndex == curCvAssignIndex);
    }
}
