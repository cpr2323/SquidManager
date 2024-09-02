#include "CurrentFolderComponent.h"
#include "../Utility/PersistentRootProperties.h"
#include "../Utility/RuntimeRootProperties.h"

CurrentFolderComponent::CurrentFolderComponent ()
{
    addAndMakeVisible (currentFolderAndProgressLabel);
}

void CurrentFolderComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFileChange = [this] (juce::String folderName)
    {
        currentFolderAndProgressLabel.setText (getFolderAndProgressString (folderName, ""), juce::NotificationType::dontSendNotification);
    };
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    currentFolderAndProgressLabel.setText (getFolderAndProgressString (appProperties.getRecentlyUsedFile (0), ""), juce::NotificationType::dontSendNotification);
}

juce::String CurrentFolderComponent::getFolderAndProgressString (juce::String folder, juce::String progress)
{
    if (! progress.isEmpty ())
        return folder + " (" + progress + ")";
    else
        return folder;
}

void CurrentFolderComponent::resized ()
{
    currentFolderAndProgressLabel.setBounds (getLocalBounds ());
}
