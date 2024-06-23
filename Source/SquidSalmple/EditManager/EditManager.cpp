#include "EditManager.h"
#include "../Bank/BankManagerProperties.h"
#include "../Metadata/SquidMetaDataReader.h"
#include "../Metadata/SquidMetaDataWriter.h"

constexpr auto kMaxSeconds { 11 };
constexpr auto kSupportedSampleRate { 44100 };

EditManager::EditManager ()
{
    audioFormatManager.registerBasicFormats ();
}

void EditManager::init (juce::ValueTree rootPropertiesVT)
{
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    BankManagerProperties bankManagerProperties (runtimeRootProperties.getValueTree (), BankManagerProperties::WrapperType::owner, BankManagerProperties::EnableCallbacks::no);
    uneditedSquidBankProperties.wrap (bankManagerProperties.getBank ("unedited"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.wrap (bankManagerProperties.getBank ("edit"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.forEachChannel ([this, &rootPropertiesVT] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        channelPropertiesList [channelIndex].wrap (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);
        return true;
    });
}

bool EditManager::isSupportedAudioFile (juce::File file)
{
    // 16
    // 44.1k
    // only mono (stereo will be converted to mono)
    if (file.isDirectory () || file.getFileExtension ().toLowerCase () != ".wav")
        return false;
    std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;
    // TODO - the module can only accept mono files, but if one loads a stereo is it converted to mono, this will need to be added as well
    // check for any format settings that are unsupported
    // NOTE - UNTIL I IMPLEMENT STEREO CONVERSION I AM ONLY ALLOW MONO TO BE LOADED
    if ((reader->usesFloatingPointData == true) || (reader->bitsPerSample != 16 && reader->bitsPerSample != 24) || (reader->numChannels != 1) || (reader->sampleRate != 44100))
        return false;

    return true;
}

void EditManager::loadChannel (juce::ValueTree squidChannelPropertiesVT, uint8_t channelIndex, juce::File sampleFile)
{
    SquidChannelProperties theSquidChannelProperties { squidChannelPropertiesVT,
                                                       SquidChannelProperties::WrapperType::owner,
                                                       SquidChannelProperties::EnableCallbacks::no };
    SquidChannelProperties newSquidChannelProperties { {}, SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no };
    if (sampleFile.exists ())
    {
        // TODO - check for import errors and handle accordingly
        addSampleToChannelProperties (newSquidChannelProperties.getValueTree (), sampleFile);
        SquidMetaDataReader squidMetaDataReader;
        squidMetaDataReader.read (newSquidChannelProperties.getValueTree (), sampleFile, channelIndex);
    }
    else
    {
        newSquidChannelProperties.setChannelIndex (channelIndex, false);
        newSquidChannelProperties.setFileName (sampleFile.getFileName (), false);
    }
    theSquidChannelProperties.copyFrom (newSquidChannelProperties.getValueTree ());
}

void EditManager::saveChannel (juce::ValueTree squidChannelPropertiesVT, uint8_t channelIndex, juce::File sampleFile)
{
    // create temp file
    // write the audio data
    // add the meta-data
#if 0
    auto tempFile { sampleFile.withFileExtension ("tmp") };
    SquidMetaDataWriter squidMetaDataWriter;
    SquidChannelProperties squidChannelPropertiesToSave (squidBankProperties.getChannelVT (channelIndex), SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);
    if (squidMetaDataWriter.write (squidChannelPropertiesToSave.getValueTree (), originalFile, tempFile))
    {
        if (originalFile.moveFileTo (originalFile.withFileExtension ("old")))
        {
            tempFile.moveFileTo (tempFile.withFileExtension ("wav"));
            originalFile.withFileExtension ("old").moveToTrash ();
            copyBank (squidBankProperties, uneditedSquidBankProperties);
        }
        else
        {
            // TODO - handle error
        }
    }
    else
    {
        // TODO - handle error
    }
#endif
}

void EditManager::saveBank ()
{
    jassert (bankDirectory.exists ());
    // update info.txt file in bank directory
    auto infoTxtFile { bankDirectory.getChildFile ("info.txt") };
    infoTxtFile.replaceWithText (squidBankProperties.getName());

    // update channel folders
    for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
    {
        auto channelDirectory { bankDirectory.getChildFile (juce::String (channelIndex + 1)) };
        SquidMetaDataWriter squidMetaDataWriter;
        SquidChannelProperties squidChannelPropertiesToSave (squidBankProperties.getChannelVT (channelIndex), SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);
        auto originalFile { channelDirectory.getChildFile (squidChannelPropertiesToSave.getFileName ()) };
        auto tempFile { originalFile.withFileExtension ("tmp") };
        if (squidMetaDataWriter.write (squidChannelPropertiesToSave.getValueTree (), originalFile, tempFile))
        {
            if (originalFile.moveFileTo (originalFile.withFileExtension ("old")))
            {
                tempFile.moveFileTo (tempFile.withFileExtension ("wav"));
                originalFile.withFileExtension ("old").moveToTrash ();
                copyBank (squidBankProperties, uneditedSquidBankProperties);
            }
            else
            {
                // TODO - handle error
            }
        }
        else
        {
            // TODO - handle error
        }
    }
}

void EditManager::loadBank (juce::File bankDirectoryToLoad)
{
    SquidBankProperties theSquidBankProperties ({}, SquidBankProperties::WrapperType::owner, SquidBankProperties::EnableCallbacks::no);

    // check for info.txt
    auto infoTxtFile { bankDirectoryToLoad.getChildFile ("info.txt") };
    // read bank name if file exists
    if (infoTxtFile.exists ())
    {
        auto infoTxtInputStream { infoTxtFile.createInputStream () };
        auto firstLine { infoTxtInputStream->readNextLine () };
        theSquidBankProperties.setName (firstLine.substring (0, 11), true);
    }
    else
    {
        theSquidBankProperties.setName (bankDirectoryToLoad.getFileName().substring (0, 11), true);
    }

    // iterate over the channel folders and load sample 
    for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
    {
        auto channelDirectory { bankDirectoryToLoad.getChildFile (juce::String (channelIndex + 1)) };
        juce::File sampleFile;
        // check for bankFolder/X (where X is the channel number)
        if (channelDirectory.exists () && channelDirectory.isDirectory ())
        {
            // TODO - what to do if there is already a wav file in the folder
            if (const auto& entry { juce::RangedDirectoryIterator (channelDirectory.getFullPathName (), false, "*.wav", juce::File::findFiles) }; entry != juce::RangedDirectoryIterator {})
                sampleFile = entry->getFile ();
        }
        else
        {
            // Channel folder does not exist, check for old style bank files "chan-00X.wav"
            auto oldStyleNamingSampleFile { bankDirectoryToLoad.getChildFile (juce::String ("chan-00") + juce::String (channelIndex + 1)).withFileExtension ("wav") };
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
        loadChannel (theSquidBankProperties.getChannelVT (channelIndex), static_cast<uint8_t>(channelIndex), sampleFile);
    }

    bankDirectory = bankDirectoryToLoad;
    copyBank (theSquidBankProperties, squidBankProperties);
    copyBank (squidBankProperties, uneditedSquidBankProperties);
}

// TODO - this is not complete. it takes a bankIndex, but I think that is incorrect, in that the EditManager only deals with the edit buffer
//        refer to client code to decide how to change things
void EditManager::loadBankDefaults (uint8_t bankIndex)
{
    SquidBankProperties defaultSquidBankProperties ({}, SquidBankProperties::WrapperType::owner, SquidBankProperties::EnableCallbacks::no);
    copyBank (defaultSquidBankProperties, squidBankProperties);
    copyBank (squidBankProperties, uneditedSquidBankProperties);
}

void EditManager::copyBank (SquidBankProperties& srcBankProperties, SquidBankProperties& destBankProperties)
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
}

void EditManager::addSampleToChannelProperties (juce::ValueTree channelPropertiesVT, juce::File sampleFile)
{
    jassert (sampleFile.exists ());
    SquidChannelProperties channelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
    if (std::unique_ptr<juce::AudioFormatReader> sampleFileReader { audioFormatManager.createReaderFor (sampleFile) }; sampleFileReader != nullptr)
    {
        const auto lengthInSamples { std::min (sampleFileReader->lengthInSamples, static_cast<juce::int64> (kMaxSeconds * kSupportedSampleRate)) };
        AudioBufferRefCounted::RefCountedPtr abrc { new AudioBufferRefCounted () };

        abrc->getAudioBuffer()->setSize (sampleFileReader->numChannels, static_cast<int> (lengthInSamples), false, true, false);
        sampleFileReader->read (abrc->getAudioBuffer (), 0, static_cast<int> (lengthInSamples), 0, true, false);

        channelProperties.setBitsPerSample (sampleFileReader->bitsPerSample, false);
        channelProperties.setSampleRate (sampleFileReader->sampleRate, false);
        channelProperties.setLengthInSamples (lengthInSamples, false);
        channelProperties.setNumChannels (sampleFileReader->numChannels, false);
        channelProperties.setAudioBuffer (abrc, false);
    }
    else
    {
        channelProperties.setBitsPerSample (0, false);
        channelProperties.setSampleRate (0.0, false);
        channelProperties.setLengthInSamples (0, false);
        channelProperties.setNumChannels (0, false);
        channelProperties.setAudioBuffer ({}, false);
    }
}
