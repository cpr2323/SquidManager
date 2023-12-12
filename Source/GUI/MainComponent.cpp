#include "MainComponent.h"
#include "../Utility/PersistentRootProperties.h"
#include "../Utility/RuntimeRootProperties.h"

const auto toolWindowHeight { 30 };

MainComponent::MainComponent (juce::ValueTree rootPropertiesVT)
{
    setSize (1117, 609);

    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);

    restoreLayout ();
}

void MainComponent::restoreLayout ()
{
}

void MainComponent::saveLayoutChanges ()
{
}

void MainComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.setColour (juce::Colours::red);
}

void MainComponent::resized ()
{
}
