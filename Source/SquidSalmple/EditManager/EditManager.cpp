#include "EditManager.h"
#include "../Bank/BankManagerProperties.h"
#include "../Metadata/SquidMetaDataReader.h"
#include "../Metadata/SquidMetaDataWriter.h"
#include "../../Utility/DebugLog.h"
#include "../../Utility/PersistentRootProperties.h"

constexpr auto kMaxSeconds { 11 };
constexpr auto kSupportedSampleRate { 44100 };
constexpr auto kMaxSampleLength { 524287 };

static uint32_t byteOffsetToSampleOffset (uint32_t byteOffset)
{
    return byteOffset / 2;
}

static uint32_t sampleOffsetToByteOffset (uint32_t sampleOffset)
{
    return sampleOffset * 2;
}

EditManager::EditManager ()
{
    audioFormatManager.registerBasicFormats ();
}

void EditManager::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties { rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no };
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
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
    copyBank (theSquidBankProperties, squidBankProperties);
    copyBank (squidBankProperties, uneditedSquidBankProperties);

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
        const auto lengthInSamples { std::min (sampleFileReader->lengthInSamples, static_cast<juce::int64> (kMaxSampleLength)) };
        AudioBufferRefCounted::RefCountedPtr abrc { new AudioBufferRefCounted () };

        abrc->getAudioBuffer()->setSize (sampleFileReader->numChannels, static_cast<int> (lengthInSamples), false, true, false);
        sampleFileReader->read (abrc->getAudioBuffer (), 0, static_cast<int> (lengthInSamples), 0, true, false);

        channelProperties.setSampleDataBits (sampleFileReader->bitsPerSample, false);
        channelProperties.setSampleDataSampleRate (sampleFileReader->sampleRate, false);
        channelProperties.setSampleDataNumSamples (lengthInSamples, false);
        channelProperties.setSampleDataNumChannels (sampleFileReader->numChannels, false);
        channelProperties.setSampleDataAudioBuffer (abrc, false);
    }
    else
    {
        channelProperties.setSampleDataBits (0, false);
        channelProperties.setSampleDataSampleRate (0.0, false);
        channelProperties.setSampleDataNumSamples (0, false);
        channelProperties.setSampleDataNumChannels (0, false);
        channelProperties.setSampleDataAudioBuffer ({}, false);
    }
}

void EditManager::concatenateAndBuildCueSets (const juce::StringArray& files, int channelIndex)
{
    auto debugLog = [this] (const juce::String& text) { DebugLog ("EditManager", text); };
    debugLog ("concatenateAndBuildCueSets");
    const auto channelDirectory { juce::File (appProperties.getRecentlyUsedFile (0)).getChildFile (juce::String (channelIndex + 1)) };
    const auto outputFile { channelDirectory.getChildFile ("temp.wav") };
    struct CueSet
    {
        uint32_t offset;
        uint32_t length;
    };
    std::vector<CueSet> cueSetList;
    auto hadError { false };
    {

        auto outputStream { outputFile.createOutputStream () };
        juce::WavAudioFormat wavAudioFormat;
        if (std::unique_ptr<juce::AudioFormatWriter> writer { wavAudioFormat.createWriterFor (outputStream.get (), 44100.0, 1, 16, {}, 0) }; writer != nullptr)
        {
            // audioFormatWriter will delete the file stream when done
            outputStream.release ();

            // build list of cur sets from file list
            // concatenate files into one file
            uint32_t curSampleOffset { 0 };
            for (auto& file : files)
            {
                std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
                jassert (reader != nullptr);
                debugLog ("opened input file: " + file);
                const auto samplesToRead { curSampleOffset + reader->lengthInSamples < kMaxSampleLength ? reader->lengthInSamples : kMaxSampleLength - (curSampleOffset + reader->lengthInSamples) };
                if (writer->writeFromAudioReader (*reader.get (), 0, samplesToRead) == true)
                {
                    debugLog ("successful file write: offset: " + juce::String (curSampleOffset) + ", numSamples: " + juce::String (samplesToRead));
                    cueSetList.emplace_back (CueSet { curSampleOffset, static_cast<uint32_t>(samplesToRead) });
                }
                else
                {
                    // handle error
                    debugLog ("ERROR - when writing file");
                    hadError = true;
                }
                curSampleOffset += samplesToRead;
                if (curSampleOffset > kMaxSampleLength)
                    break;
            }
        }
        else
        {
            debugLog ("ERROR - unable to open output file: " + outputFile.getFullPathName ());
            hadError = true;
        }
    }
    if (! hadError)
    {
        auto& channelProperties { channelPropertiesList [channelIndex] };
        // load file
        loadChannel (channelProperties.getValueTree (), channelIndex, outputFile);
        // set cue sets
        for (auto cueSetIndex { 0 }; cueSetIndex < cueSetList.size (); ++cueSetIndex)
            channelProperties.setCueSetPoints (cueSetIndex, sampleOffsetToByteOffset(cueSetList [cueSetIndex].offset),
                                               sampleOffsetToByteOffset(cueSetList [cueSetIndex].offset),
                                               sampleOffsetToByteOffset(cueSetList [cueSetIndex].offset + cueSetList [cueSetIndex].length));
    }
    else
    {
        outputFile.deleteFile ();
    }
}
