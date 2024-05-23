#include "EditManager.h"
#include "../Metadata/SquidMetaDataReader.h"

void EditManager::init (juce::ValueTree rootPropertiesVT)
{
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    squidBankProperties.wrap (runtimeRootProperties.getValueTree (), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.forEachChannel ([this, &rootPropertiesVT] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        squidChannelProperties [channelIndex].wrap (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);
        return true;
    });
}


void EditManager::loadBank (juce::File bankDirectory)
{
    squidBankProperties.triggerLoadBegin (false);
    // check for info.txt
    auto infoTxtFile { bankDirectory.getChildFile ("info.txt") };
    // read bank name if file exists
    if (infoTxtFile.exists ())
    {
        auto infoTxtInputStream { infoTxtFile.createInputStream () };
        auto firstLine { infoTxtInputStream->readNextLine () };
        squidBankProperties.setName (firstLine.substring (0, 11), true);
    }
    // iterate over the channel folders and load sample 
    for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
    {
        auto channelDirectory { bankDirectory.getChildFile (juce::String (channelIndex + 1)) };
        juce::File sampleFile;
        // check for bankFolder/X (where X is the channel number)
        if (channelDirectory.exists () && channelDirectory.isDirectory ())
        {
            // TODO - what to do if there is already a wav file in the folder
            for (const auto& entry : juce::RangedDirectoryIterator (channelDirectory.getFullPathName (), false, "*.wav", juce::File::findFiles))
            {
                sampleFile = entry.getFile ();
                break;
            }
        }
        else
        {
            // Channel folder does not exist, check for old style bank files "chan-00X.wav"
            auto oldStyleNamingSampleFile { bankDirectory.getChildFile (juce::String ("chan-00") + juce::String (channelIndex + 1)).withFileExtension ("wav") };
            if (oldStyleNamingSampleFile.exists () && !oldStyleNamingSampleFile.isDirectory ())
            {
                // create folder
                if (!channelDirectory.createDirectory ())
                {
                    // TODO - report error in creating directory
                }
                else
                {
                    // copy file
                    auto destFile { channelDirectory.getChildFile (oldStyleNamingSampleFile.getFileName ()) };
                    // TODO - handle copy failure
                    oldStyleNamingSampleFile.copyFileTo (destFile);
                    sampleFile = destFile;
                }
            }
        }
        SquidChannelProperties squidChannelProperties { squidBankProperties.getChannelVT (channelIndex),
                                                        SquidChannelProperties::WrapperType::client,
                                                        SquidChannelProperties::EnableCallbacks::no };
        squidChannelProperties.triggerLoadBegin (false);
        if (sampleFile.exists ())
        {
            // TODO - check for import errors and handle accordingly
            SquidMetaDataReader squidMetaDataReader;
            SquidChannelProperties loadedSquidMetaDataProperties { squidMetaDataReader.read (sampleFile),
                                                                    SquidChannelProperties::WrapperType::owner,
                                                                    SquidChannelProperties::EnableCallbacks::no };
            squidChannelProperties.copyFrom (loadedSquidMetaDataProperties.getValueTree ());
        }
        else
        {
            // TODO - load default. report and error?
        }
        squidChannelProperties.triggerLoadComplete (false);
    }
    squidBankProperties.triggerLoadComplete (false);
}