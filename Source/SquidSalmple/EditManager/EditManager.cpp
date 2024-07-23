#include "EditManager.h"
#include "../Bank/BankManagerProperties.h"
#include "../Metadata/SquidMetaDataReader.h"
#include "../Metadata/SquidMetaDataWriter.h"
#include "../../Utility/DebugLog.h"
#include "../../Utility/PersistentRootProperties.h"

constexpr auto kMaxSeconds { 11 };
constexpr auto kSupportedSampleRate { 44100 };
constexpr auto kMaxSampleLength { 524287 };

EditManager::EditManager ()
{
    audioFormatManager.registerBasicFormats ();
    defaultSquidBankProperties.wrap ({}, SquidBankProperties::WrapperType::owner, SquidBankProperties::EnableCallbacks::no);
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
    // check for any format settings that are unsupported
    if ((reader->usesFloatingPointData == true) || (reader->bitsPerSample != 16 && reader->bitsPerSample != 24) || (reader->numChannels > 2) || (reader->sampleRate != 44100))
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
        newSquidChannelProperties.setSampleFileName (sampleFile.getFullPathName (), false);
    }
    theSquidChannelProperties.copyFrom (newSquidChannelProperties.getValueTree ());
}

void EditManager::saveChannel (juce::ValueTree /*squidChannelPropertiesVT*/, uint8_t /*channelIndex*/, juce::File /*sampleFile*/)
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
    jassertfalse;
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
        auto sampleFileName { squidChannelPropertiesToSave.getSampleFileName () };
        if (sampleFileName.isEmpty ())
            continue;
        auto originalFile { juce::File (sampleFileName) };
        auto tempFile { originalFile.withFileExtension ("tmp") };
        // write out the file with the new metadata to a tmp file
        if (squidMetaDataWriter.write (squidChannelPropertiesToSave.getValueTree (), originalFile, tempFile))
        {
            // now move the old file out of the way with extension 'old'
            if (originalFile.moveFileTo (originalFile.withFileExtension ("old")))
            {
                const auto newFile { tempFile.withFileExtension ("wav") };
                // now move the new file to one with the extension of wav
                tempFile.moveFileTo (newFile);
                // and delete the original
                originalFile.withFileExtension ("old").moveToTrash ();
                // also delete any other wav or _wav files in directory
                for (const auto& entry : juce::RangedDirectoryIterator (tempFile.getParentDirectory (), false, "*", juce::File::findFiles))
                {
                    if (entry.getFile () == newFile)
                        continue;
                    const auto extension { entry.getFile ().getFileExtension ().toLowerCase ()};
                    if (extension == ".wav" || extension == "._wav")
                        entry.getFile ().moveToTrash ();
                }
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
    // finally, copy the new data to the unedited buffer
    copyBank (squidBankProperties, uneditedSquidBankProperties);
}

juce::ValueTree EditManager::getUneditedBankProperties ()
{
    return uneditedSquidBankProperties.getValueTree ();
}

juce::ValueTree EditManager::getDefaultBankProperties ()
{
    return defaultSquidBankProperties.getValueTree ();
}

juce::ValueTree EditManager::getUneditedChannelProperties (int channelIndex)
{
    return uneditedSquidBankProperties.getChannelVT (channelIndex);
}

juce::ValueTree EditManager::getDefaultChannelProperties (int channelIndex)
{
    return defaultSquidBankProperties.getChannelVT (channelIndex);
}

void EditManager::loadBank (juce::File bankDirectoryToLoad)
{
    SquidBankProperties theSquidBankProperties ({}, SquidBankProperties::WrapperType::owner, SquidBankProperties::EnableCallbacks::no);
    copyBank (theSquidBankProperties, squidBankProperties);
    copyBank (squidBankProperties, uneditedSquidBankProperties);

    // check for info.txt
    auto infoTxtFile { bankDirectoryToLoad.getChildFile ("info.txt") };
    auto newBankName { juce::String () };
    if (infoTxtFile.exists ())
    {
        // read bank name if file exists
        auto infoTxtInputStream { infoTxtFile.createInputStream () };
        auto firstLine { infoTxtInputStream->readNextLine () };
        theSquidBankProperties.setName (firstLine.substring (0, 11), true);
    }
    else
    {
         newBankName = bankDirectoryToLoad.getFileName().substring (0, 11);
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
                if (! channelDirectory.createDirectory ())
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
    if (! newBankName.isEmpty ())
        squidBankProperties.setName (newBankName, true);
}

// TODO - this is not complete. it takes a bankIndex, but I think that is incorrect, in that the EditManager only deals with the edit buffer
//        refer to client code to decide how to change things
void EditManager::loadBankDefaults (uint8_t /*bankIndex*/)
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
    juce::WavAudioFormat wavAudioFormat;
    auto inputStream { sampleFile.createInputStream () };
    // we have to use the WavAudioFormat::createReaderFor interface here, since the file may be our renamed ._wav type, which the AudioFormatManager will reject based on extension
    if (std::unique_ptr<juce::AudioFormatReader> sampleFileReader { wavAudioFormat.createReaderFor (inputStream.get (), true) }; sampleFileReader != nullptr)
    {
        inputStream.release ();
        const auto lengthInSamples { static_cast<uint32_t> (std::min (sampleFileReader->lengthInSamples, static_cast<juce::int64> (kMaxSampleLength))) };
        AudioBufferRefCounted::RefCountedPtr abrc { new AudioBufferRefCounted () };
        abrc->getAudioBuffer ()->setSize (1, static_cast<int> (lengthInSamples), false, true, false);

        if (sampleFileReader->numChannels == 1)
        {
            sampleFileReader->read (abrc->getAudioBuffer (), 0, static_cast<int> (lengthInSamples), 0, true, false);
        }
        else
        {
            // convert to mono
            juce::AudioBuffer<float> stereoAudioBuffer;
            stereoAudioBuffer.setSize (sampleFileReader->numChannels, static_cast<int> (lengthInSamples), false, true, false);
            sampleFileReader->read (&stereoAudioBuffer, 0, static_cast<int> (lengthInSamples), 0, true, true);

            constexpr float sqrRootOfTwo { 1.41421356237f };
            auto leftChannelReadPtr { stereoAudioBuffer.getReadPointer (0) };
            auto rightChannelReadPtr { stereoAudioBuffer.getReadPointer (1) };
            auto monoWritePtr { abrc->getAudioBuffer ()->getWritePointer (0) };
            for (uint32_t sampleCounter { 0 }; sampleCounter < lengthInSamples; ++sampleCounter)
            {
                *monoWritePtr = (*leftChannelReadPtr + *rightChannelReadPtr) / sqrRootOfTwo;
                ++monoWritePtr;
                ++leftChannelReadPtr;
                ++rightChannelReadPtr;
            }
        }

        channelProperties.setSampleDataBits (sampleFileReader->bitsPerSample, false);
        channelProperties.setSampleDataSampleRate (sampleFileReader->sampleRate, false);
        channelProperties.setSampleDataNumSamples (lengthInSamples, false);
        channelProperties.setSampleDataNumChannels (1, false);
        channelProperties.setSampleDataAudioBuffer (abrc, false);
    }
    else
    {
        inputStream.release ();
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
    if (! channelDirectory.exists ())
        channelDirectory.createDirectory ();
    const auto outputFile { channelDirectory.getChildFile ("new._wav") };
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
                const auto samplesToRead { static_cast<uint32_t> (curSampleOffset + reader->lengthInSamples < kMaxSampleLength ? reader->lengthInSamples : kMaxSampleLength - (curSampleOffset + reader->lengthInSamples)) };
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
        loadChannel (channelProperties.getValueTree (), static_cast<uint8_t> (channelIndex), outputFile);
        // set cue sets
        for (auto cueSetIndex { 0 }; cueSetIndex < cueSetList.size (); ++cueSetIndex)
            channelProperties.setCueSetPoints (cueSetIndex, SquidChannelProperties::sampleOffsetToByteOffset(cueSetList [cueSetIndex].offset),
                                               SquidChannelProperties::sampleOffsetToByteOffset(cueSetList [cueSetIndex].offset),
                                               SquidChannelProperties::sampleOffsetToByteOffset(cueSetList [cueSetIndex].offset + cueSetList [cueSetIndex].length));
    }
    else
    {
        outputFile.deleteFile ();
    }
}

void EditManager::cleanUpTempFiles (juce::File bankFolder)
{
    for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
    {
        auto channelFolder { bankFolder.getChildFile (juce::String (channelIndex + 1)) };
        for (const auto& entry : juce::RangedDirectoryIterator (channelFolder, false, "*._wav", juce::File::findFiles))
            entry.getFile ().moveToTrash ();
    }

}

void EditManager::forChannels (std::vector<int> channelIndexList, std::function<void (juce::ValueTree)> channelCallback)
{
    jassert (channelCallback != nullptr);
    for (const auto channelIndex : channelIndexList)
    {
        jassert (channelIndex >= 0 && channelIndex < 8);
        channelCallback (channelPropertiesList [channelIndex].getValueTree ());
    }
}
