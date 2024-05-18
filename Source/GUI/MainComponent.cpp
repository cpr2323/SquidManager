#include "MainComponent.h"
#include "../SystemServices.h"
#include "../Utility/PersistentRootProperties.h"
#include "../Utility/RuntimeRootProperties.h"

const auto toolWindowHeight { 30 };

MainComponent::MainComponent (juce::ValueTree rootPropertiesVT)
{
    setSize (1117, 609);

    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    addAndMakeVisible (squidMetaDataEditorComponent);
    squidMetaDataEditorComponent.init (rootPropertiesVT);

    // TODO - this is just some test code. remove when done
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    auto& sampleManager { systemServices.getSampleManager () };
    sampleManager.isSupportedAudioFile ({});

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
    squidMetaDataEditorComponent.setBounds (getLocalBounds ());
}
