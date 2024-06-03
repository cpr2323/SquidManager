#include "EditManager.h"
#include "../Bank/BankManagerProperties.h"
#include "../Metadata/SquidMetaDataReader.h"

void EditManager::init (juce::ValueTree rootPropertiesVT)
{
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    BankManagerProperties bankManagerProperties (runtimeRootProperties.getValueTree (), BankManagerProperties::WrapperType::owner, BankManagerProperties::EnableCallbacks::no);
    uneditedSquidBankProperties.wrap (bankManagerProperties.getBank ("unedited"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.wrap (bankManagerProperties.getBank ("edit"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.forEachChannel ([this, &rootPropertiesVT] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        squidChannelPropertiesList [channelIndex].wrap (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);
        return true;
    });
}

void EditManager::loadChannel (juce::ValueTree squidChannelPropertiesVT, int channelIndex, juce::File sampleFile)
{
    SquidChannelProperties theSquidChannelProperties { squidChannelPropertiesVT,
                                                       SquidChannelProperties::WrapperType::owner,
                                                       SquidChannelProperties::EnableCallbacks::no };
    if (sampleFile.exists ())
    {
        // TODO - check for import errors and handle accordingly
        SquidMetaDataReader squidMetaDataReader;
        SquidChannelProperties loadedSquidChannelProperties { squidMetaDataReader.read (sampleFile),
                                                                SquidChannelProperties::WrapperType::owner,
                                                                SquidChannelProperties::EnableCallbacks::no };
        theSquidChannelProperties.copyFrom (loadedSquidChannelProperties.getValueTree ());
    }
    else
    {
        // TODO - load default. report and error?
        SquidChannelProperties defaultSquidChannelProperties { {}, SquidChannelProperties::WrapperType::owner,
                                                                    SquidChannelProperties::EnableCallbacks::no };
        defaultSquidChannelProperties.setChannelIndex (channelIndex, false);
        defaultSquidChannelProperties.setFileName (sampleFile.getFileName (), false);
        theSquidChannelProperties.copyFrom (defaultSquidChannelProperties.getValueTree ());
    }
}

void EditManager::loadBank (juce::File bankDirectory)
{
    SquidBankProperties theSquidBankProperties ({}, SquidBankProperties::WrapperType::owner, SquidBankProperties::EnableCallbacks::no);

    // check for info.txt
    auto infoTxtFile { bankDirectory.getChildFile ("info.txt") };
    // read bank name if file exists
    if (infoTxtFile.exists ())
    {
        auto infoTxtInputStream { infoTxtFile.createInputStream () };
        auto firstLine { infoTxtInputStream->readNextLine () };
        theSquidBankProperties.setName (firstLine.substring (0, 11), true);
    }
    else
    {
        theSquidBankProperties.setName (bankDirectory.getFileName().substring (0, 11), true);
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
        loadChannel (theSquidBankProperties.getChannelVT (channelIndex), channelIndex, sampleFile);
    }

    auto copyBank = [] (SquidBankProperties& srcBankProperties, SquidBankProperties& destBankProperties)
    {
        destBankProperties.triggerLoadBegin (false);
        destBankProperties.setName (srcBankProperties.getName (), false);
        for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
        {
            SquidChannelProperties destChannelProperties { destBankProperties.getChannelVT (channelIndex),
                                                           SquidChannelProperties::WrapperType::owner,
                                                           SquidChannelProperties::EnableCallbacks::no };
            destChannelProperties.triggerLoadBegin (false);
            destChannelProperties.copyFrom (srcBankProperties.getChannelVT (channelIndex));
            destChannelProperties.triggerLoadComplete (false);
        }
        destBankProperties.triggerLoadComplete (false);
    };
    copyBank (theSquidBankProperties, squidBankProperties);
    copyBank (theSquidBankProperties, uneditedSquidBankProperties);
}
