#include "EditManager.h"
#include "../CvParameterProperties.h"
#include "../Bank/BankManagerProperties.h"
#include "../Metadata/SquidSalmpleDefs.h"
#include "../Metadata/SquidMetaDataReader.h"
#include "../Metadata/SquidMetaDataWriter.h"
#include "../../Utility/DebugLog.h"
#include "../../Utility/PersistentRootProperties.h"
#include "../../SRC//libsamplerate-0.1.9/src/samplerate.h"

#define LOG_EDIT_MANAGER 0
#if LOG_EDIT_MANAGER
#define LogEditManager(text) DebugLog ("EditManager", text);
#else
#define LogEditManager(text) ;
#endif

constexpr float epsilon { 1e-6f };

constexpr auto kMaxSeconds { 11 };
constexpr auto kSupportedSampleRate { 44100 };
constexpr auto kMaxSampleLength { 524287 };

EditManager::EditManager ()
{
    audioFormatManager.registerBasicFormats ();
    for (auto formatIndex { 0 }; formatIndex < audioFormatManager.getNumKnownFormats (); ++formatIndex)
    {
        const auto* format { audioFormatManager.getKnownFormat (formatIndex) };
        //DebugLog ("EditManager", "Format Name: " + format->getFormatName ());
        //DebugLog ("EditManager", "Format Extensions: " + format->getFileExtensions ().joinIntoString (", "));
        audioFileExtensions.addArray (format->getFileExtensions ());
    }

    defaultSquidBankProperties.wrap ({}, SquidBankProperties::WrapperType::owner, SquidBankProperties::EnableCallbacks::no);
    defaultSquidBankProperties.forEachChannel ([this] (juce::ValueTree channelPropertiesVT, [[maybe_unused]]int channelIndex)
    {
        // fully initialize the channel properties
        SquidChannelProperties channelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);
        return true;
    });
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

bool EditManager::isAltOutput (int channelIndex)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    return isAltOutput (channelPropertiesList [channelIndex]);
}

bool EditManager::isAltOutput (juce::ValueTree channelPropertiesVT)
{
    SquidChannelProperties channelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);
    return isAltOutput (channelProperties);
}

bool EditManager::isAltOutput (SquidChannelProperties& channelProperties)
{
    return (channelProperties.getChannelFlags () & ChannelFlags::kNeighborOutput) != 0;
}

void EditManager::setAltOutput (SquidChannelProperties& channelProperties, bool useAltOutput)
{
    if (useAltOutput)
    {
        const auto newFlags { static_cast<uint16_t> (channelProperties.getChannelFlags () | ChannelFlags::kNeighborOutput) };
        channelProperties.setChannelFlags (newFlags, false);
    }
    else
    {
        const auto offFlag { ~ChannelFlags::kNeighborOutput };
        const auto newFlags { static_cast<uint16_t> (channelProperties.getChannelFlags () & offFlag) };
        channelProperties.setChannelFlags (newFlags, false);
    }
}

void EditManager::setAltOutput (int channelIndex, bool useAltOutput)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    setAltOutput (channelPropertiesList [channelIndex], useAltOutput);
}

void EditManager::setAltOutput (juce::ValueTree channelPropertiesVT, bool useAltOutput)
{
    SquidChannelProperties channelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);
    setAltOutput (channelProperties, useAltOutput);
}

void EditManager::swapChannels (int firstChannelIndex, int secondChannelIndex)
{
    auto& firstChannelProperties { channelPropertiesList [firstChannelIndex] };
    auto& secondChannelProperties { channelPropertiesList [secondChannelIndex] };
    // copy files into respective folders
    auto copyFileAsTemp = [this] (juce::File srcFile, int dstChannelIndex)
    {
        if (srcFile == juce::File ())
            return juce::File ();
        const auto destChannelDirectory { juce::File (appProperties.getRecentlyUsedFile (0)).getChildFile (juce::String (dstChannelIndex + 1)) };
        if (! destChannelDirectory.exists ())
            destChannelDirectory.createDirectory ();
        const auto destFile { destChannelDirectory.getChildFile (srcFile.getFileNameWithoutExtension ()).withFileExtension ("._tmp") };
        srcFile.copyFileTo (destFile);
        if (srcFile.getFileExtension () == "._wav")
            srcFile.deleteFile ();
        return destFile;
    };
    auto renameTempFileToActualFile = [] (juce::File tempFileToRename)
        {
            auto newNameFile { tempFileToRename.withFileExtension ("._wav") };
            if (newNameFile.exists ())
                newNameFile.deleteFile ();
            tempFileToRename.moveFileTo (newNameFile);
            return newNameFile.getFullPathName ();
        };
    // we need to copy the files to temp names, in case the file names are the same (ie. the first copy would have overwritten the second file)
    // before we could copy it.
    auto tempSecondFile { copyFileAsTemp (firstChannelProperties.getSampleFileName (), secondChannelProperties.getChannelIndex ()) };
    auto tempFirstFile { copyFileAsTemp (secondChannelProperties.getSampleFileName (), firstChannelProperties.getChannelIndex ()) };
    // after both files are copied, we can give them their intended names
    auto newSecondFileName { renameTempFileToActualFile (tempSecondFile) };
    auto newFirstFileName { renameTempFileToActualFile (tempFirstFile) };
    // swap SquidChannelProperties
    auto getNewChannelProperties = [] (juce::ValueTree channelProperties, int newChannelIndex, juce::String newFileName)
    {
        SquidChannelProperties newChannelProperties (channelProperties.createCopy (), SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);
        newChannelProperties.setSampleFileName (newFileName, false);
        auto oldChannelIndex { newChannelProperties.getChannelIndex () };
        newChannelProperties.setChannelIndex (static_cast<uint8_t> (newChannelIndex), false);
        if (newChannelProperties.getChannelSource () == oldChannelIndex)
            newChannelProperties.setChannelSource (static_cast<uint8_t> (newChannelIndex), false);
        if (newChannelProperties.getChoke () == oldChannelIndex)
            newChannelProperties.setChoke (newChannelIndex, false);
        if (newChannelProperties.getRecDest () == oldChannelIndex)
            newChannelProperties.setRecDest (newChannelIndex, false);
        return newChannelProperties.getValueTree ();
    };
    auto newSecondChannelProperties { getNewChannelProperties (firstChannelProperties.getValueTree (), secondChannelProperties.getChannelIndex (), newSecondFileName) };
    auto newFirstChannelProperties { getNewChannelProperties (secondChannelProperties.getValueTree (), firstChannelProperties.getChannelIndex (), newFirstFileName) };
    firstChannelProperties.copyFrom (newFirstChannelProperties, SquidChannelProperties::CopyType::all, SquidChannelProperties::CheckIndex::no);
    secondChannelProperties.copyFrom (newSecondChannelProperties, SquidChannelProperties::CopyType::all, SquidChannelProperties::CheckIndex::no);
}

FileInfo EditManager::getFileInfo (juce::File file)
{
    // 16
    // 44.1k
    // only mono (stereo will be converted to mono)
    if (file.isDirectory () || file.getFileExtension ().toLowerCase () != ".wav")
        return {};
    std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
    if (reader == nullptr)
        return {};
    // check for any format settings that are unsupported
    if ((reader->usesFloatingPointData == true) || (reader->bitsPerSample != 16 && reader->bitsPerSample != 24) || (reader->numChannels > 2) || (reader->sampleRate != 44100))
        return { false, reader->sampleRate, reader->bitsPerSample, reader->lengthInSamples, reader->numChannels, reader->usesFloatingPointData };

    return { true, reader->sampleRate, reader->bitsPerSample, reader->lengthInSamples, reader->numChannels, reader->usesFloatingPointData };
}

bool EditManager::isCueRandomOn (int channelIndex)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    return isCueRandomOn (channelPropertiesList [channelIndex]);
}

bool EditManager::isCueRandomOn (juce::ValueTree channelPropertiesVT)
{
    SquidChannelProperties channelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);
    return isCueRandomOn (channelProperties);
}

bool EditManager::isCueRandomOn (SquidChannelProperties& channelProperties)
{
    return channelProperties.getChannelFlags () & ChannelFlags::kCueRandom;
}

bool EditManager::isCueStepOn (int channelIndex)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    return isCueStepOn (channelPropertiesList [channelIndex]);
}

bool EditManager::isCueStepOn (juce::ValueTree channelPropertiesVT)
{
    SquidChannelProperties channelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);
    return isCueStepOn (channelProperties);
}

bool EditManager::isCueStepOn (SquidChannelProperties& channelProperties)
{
    return channelProperties.getChannelFlags () & ChannelFlags::kCueStepped;
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
    theSquidChannelProperties.copyFrom (newSquidChannelProperties.getValueTree (), SquidChannelProperties::CopyType::all, SquidChannelProperties::CheckIndex::no);
}

void EditManager::renameSample (int channelIndex, juce::String newSampleName)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    auto currentFile { juce::File (channelPropertiesList[channelIndex].getSampleFileName ()) };
    auto newFile { currentFile.getParentDirectory ().getChildFile (newSampleName).withFileExtension ("._wav") };
    LogEditManager ("Current File: " + currentFile.getFullPathName ());
    LogEditManager ("New File: " + newFile.getFullPathName ());
    if (currentFile.getFileExtension () == ".wav")
    {
        LogEditManager ("Copying");
        auto copySuccess { currentFile.copyFileTo (newFile) };
        jassert (copySuccess == true);
    }
    else // assume files ends with ._wav
    {
        LogEditManager ("Renaming");
        auto renameSuccess { currentFile.moveFileTo (newFile) };
        jassert (renameSuccess == true);
    }
    channelPropertiesList [channelIndex].setSampleFileName (newFile.getFullPathName (), false);
}

void EditManager::saveChannel (juce::ValueTree /*squidChannelPropertiesVT*/, uint8_t /*channelIndex*/, juce::File /*sampleFile*/)
{
    // create temp file
    // write the audio data
    // add the metadata
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
    infoTxtFile.replaceWithText (squidBankProperties.getName ());

    // update channel folders
    for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
    {
        auto channelDirectory { bankDirectory.getChildFile (juce::String (channelIndex + 1)) };
        SquidMetaDataWriter squidMetaDataWriter;
        SquidChannelProperties squidChannelPropertiesToSave (squidBankProperties.getChannelVT (channelIndex), SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);
        auto sampleFileName { squidChannelPropertiesToSave.getSampleFileName () };
        if (sampleFileName.isEmpty ())
        {
            for (const auto& entry : juce::RangedDirectoryIterator (channelDirectory, false, "*", juce::File::findFiles))
            {
                const auto extension { entry.getFile ().getFileExtension ().toLowerCase () };
                if (extension == ".wav" || extension == "._wav")
                    entry.getFile ().moveToTrash ();
            }
            continue;
        }
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
                squidChannelPropertiesToSave.setSampleFileName (newFile.getFullPathName (), false);
                // and delete the original
                originalFile.withFileExtension ("old").moveToTrash ();
                // also delete any other wav or _wav files in directory
                for (const auto& entry : juce::RangedDirectoryIterator (tempFile.getParentDirectory (), false, "*", juce::File::findFiles))
                {
                    if (entry.getFile () == newFile)
                        continue;
                    const auto extension { entry.getFile ().getFileExtension ().toLowerCase () };
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

void EditManager::cleanupChannelTempFiles ()
{
    for (auto& channelProperties : channelPropertiesList)
    {
        auto currentSampleFile { juce::File (channelProperties.getSampleFileName ()) };
        if (currentSampleFile.getFileExtension () == "._wav")
            currentSampleFile.moveToTrash ();
    }
}
void EditManager::setBankDefaults ()
{
    cleanupChannelTempFiles ();
    squidBankProperties.copyFrom (defaultSquidBankProperties.getValueTree ());
}

void EditManager::setBankUnedited ()
{
    cleanupChannelTempFiles ();
    squidBankProperties.copyFrom (uneditedSquidBankProperties.getValueTree ());
}

void EditManager::clearChannel (int channelIndex)
{
    SquidChannelProperties clearedChannelProperties (defaultSquidBankProperties.getChannelVT (channelIndex), SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);

    auto& channelProperties { channelPropertiesList [channelIndex] };
    clearedChannelProperties.setChannelIndex (static_cast<uint8_t> (channelIndex), false);
    clearedChannelProperties.setChannelSource (static_cast<uint8_t> (channelIndex), false);
    clearedChannelProperties.setChoke (channelIndex, false);
    clearedChannelProperties.setRecDest (channelIndex, false);
    clearedChannelProperties.setSampleDataBits (0, false);
    clearedChannelProperties.setSampleDataSampleRate (0.0, false);
    clearedChannelProperties.setSampleDataNumSamples (0, false);
    clearedChannelProperties.setSampleDataNumChannels (0, false);
    clearedChannelProperties.setSampleDataAudioBuffer ({}, false);

    channelProperties.copyFrom (clearedChannelProperties.getValueTree (), SquidChannelProperties::CopyType::all, SquidChannelProperties::CheckIndex::no);
}

void EditManager::setChannelDefaults (int channelIndex)
{
    SquidChannelProperties defaultChannelProperties (defaultSquidBankProperties.getChannelVT (channelIndex), SquidChannelProperties::WrapperType::owner, SquidChannelProperties::EnableCallbacks::no);

    auto& channelProperties { channelPropertiesList[channelIndex] };
    defaultChannelProperties.setChannelIndex (static_cast<uint8_t> (channelIndex), false);
    defaultChannelProperties.setChannelSource (static_cast<uint8_t> (channelIndex), false);
    defaultChannelProperties.setChoke (channelIndex, false);
    defaultChannelProperties.setEndOfData (channelProperties.getEndOfData (), false);
    defaultChannelProperties.setRecDest (channelIndex, false);
    defaultChannelProperties.setSampleFileName (channelProperties.getSampleFileName (), false);
    addSampleToChannelProperties (defaultChannelProperties.getValueTree (), channelProperties.getSampleFileName ());
    const auto endOffset { SquidChannelProperties::sampleOffsetToByteOffset (defaultChannelProperties.getSampleDataNumSamples ()) };
    defaultChannelProperties.setEndCue (endOffset, false);
    defaultChannelProperties.setCueSetPoints (0, 0, 0, endOffset);

    channelProperties.copyFrom (defaultChannelProperties.getValueTree (), SquidChannelProperties::CopyType::all, SquidChannelProperties::CheckIndex::no);
}

void EditManager::setChannelUnedited (int channelIndex)
{
    auto& channelProperties { channelPropertiesList [channelIndex] };
    auto currentSampleFile { juce::File (channelProperties.getSampleFileName ()) };
    if (currentSampleFile.getFileExtension () == "._wav")
        currentSampleFile.moveToTrash ();
    channelProperties.copyFrom (uneditedSquidBankProperties.getChannelVT (channelIndex), SquidChannelProperties::CopyType::all, SquidChannelProperties::CheckIndex::no);
}

void EditManager::setCueRandom (int channelIndex, bool on)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    auto& channelProperties { channelPropertiesList [channelIndex] };
    if (on)
    {
        // turn cue random on
        channelProperties.setChannelFlags (channelProperties.getChannelFlags () | ChannelFlags::kCueRandom, false);
        // turn cue stepped off (in case it is on)
        channelProperties.setChannelFlags (channelProperties.getChannelFlags () & ~ChannelFlags::kCueStepped, false);
    }
    else
    {
        // turn cue random off
        channelProperties.setChannelFlags (channelProperties.getChannelFlags () & ~ChannelFlags::kCueRandom, false);
    }
}

void EditManager::setCueStep (int channelIndex, bool on)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    auto& channelProperties { channelPropertiesList [channelIndex] };
    if (on)
    {
        // turn cue step on
        channelProperties.setChannelFlags (channelProperties.getChannelFlags () | ChannelFlags::kCueStepped, false);
        // turn cue random off (in case it is on)
        channelProperties.setChannelFlags (channelProperties.getChannelFlags () & ~ChannelFlags::kCueRandom, false);
    }
    else
    {
        // turn cue step off
        channelProperties.setChannelFlags (channelProperties.getChannelFlags () & ~ChannelFlags::kCueStepped, false);
    }
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
    if (infoTxtFile.exists ())
    {
        // read bank name if file exists
        auto infoTxtInputStream { infoTxtFile.createInputStream () };
        auto firstLine { infoTxtInputStream->readNextLine () };
        theSquidBankProperties.setName (firstLine.substring (0, 11), true);
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
            if (oldStyleNamingSampleFile.exists () && ! oldStyleNamingSampleFile.isDirectory ())
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
        loadChannel (theSquidBankProperties.getChannelVT (channelIndex), static_cast<uint8_t> (channelIndex), sampleFile);
    }

    bankDirectory = bankDirectoryToLoad;
    copyBank (theSquidBankProperties, squidBankProperties);
    copyBank (squidBankProperties, uneditedSquidBankProperties);
}

// TODO - this is not complete. it takes a bankIndex, but I think that is incorrect, in that the EditManager only deals with the edit buffer
//        refer to client code to decide how to change things
void EditManager::loadBankDefaults (uint8_t /*bankIndex*/)
{
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
        destChannelProperties.copyFrom (srcBankProperties.getChannelVT (channelIndex), SquidChannelProperties::CopyType::all, SquidChannelProperties::CheckIndex::no);
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

void EditManager::sampleConvert (juce::AudioFormatReader* reader, juce::AudioBuffer<float>& outputBuffer)
{
    juce::AudioBuffer<float> inputBuffer;
    const auto numChannels { reader->numChannels };
    const auto numSamples { reader->lengthInSamples };
    inputBuffer.setSize (numChannels, static_cast<int> (numSamples), false, true, false);
    reader->read (&inputBuffer, 0, static_cast<int> (numSamples), 0, true, true);

    const double ratio { 44100. / reader->sampleRate };
    const int outputNumSamples { static_cast<int> (numSamples * ratio) };
    outputBuffer.setSize (numChannels, outputNumSamples, false, true, false);
    SRC_STATE* srcState = src_new (SRC_SINC_BEST_QUALITY, numChannels, nullptr);
    if (srcState == nullptr)
    {
        // TODO - handle error
        jassertfalse;
    }

    SRC_DATA srcData;
    srcData.data_in = inputBuffer.getReadPointer (0);
    srcData.input_frames = static_cast<int> (numSamples);
    srcData.data_out = outputBuffer.getWritePointer (0);
    srcData.output_frames = outputNumSamples;
    srcData.src_ratio = ratio;
    srcData.end_of_input = 0;

    int error { src_process (srcState, &srcData) };
    src_delete (srcState);

    if (error != 0)
    {
        // TODO - handle error
        jassertfalse;
    }
}

void EditManager::concatenateAndBuildCueSets (const juce::StringArray& files, int channelIndex, juce::String outputFileName, juce::ValueTree cueSetListVT)
{
    LogEditManager ("concatenateAndBuildCueSets");
    const auto channelDirectory { juce::File (appProperties.getRecentlyUsedFile (0)).getChildFile (juce::String (channelIndex + 1)) };
    if (! channelDirectory.exists ())
        channelDirectory.createDirectory ();
    const auto outputFile { channelDirectory.getChildFile (outputFileName) };
    if (outputFile.exists ())
        outputFile.deleteFile ();
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

            // build list of cue sets from file list
            // concatenate files into one file
            uint32_t curSampleOffset { 0 };
            int numFilesProcessed { 0 };
            for (auto& inputFileName : files)
            {
                ++numFilesProcessed;
                auto inputFile { juce::File (inputFileName) };
                std::unique_ptr<juce::AudioFormatReader> reader;
                if (inputFile.getFileExtension () == "._wav")
                {
                    // we have to use the WavAudioFormat::createReaderFor interface here, since the file may be our renamed ._wav type, which the AudioFormatManager will reject based on extension
                    auto inputStream { inputFile.createInputStream () };
                    reader.reset (wavAudioFormat.createReaderFor (inputStream.get (), true));
                    if (reader != nullptr)
                        inputStream.release ();
                }
                else
                {
                    reader.reset (audioFormatManager.createReaderFor (inputFile));
                }
                if (reader != nullptr)
                {
                    jassert (reader != nullptr);
                    LogEditManager ("opened input file [" + juce::String (numFilesProcessed) + "]: " + inputFileName);
                    if (reader->bitsPerSample == 44100)
                    {
                        if (curSampleOffset + reader->lengthInSamples < kMaxSampleLength)
                        {
                            const auto samplesToRead { static_cast<uint32_t> (reader->lengthInSamples) };
                            if (writer->writeFromAudioReader (*reader.get (), 0, samplesToRead) == true)
                            {
                                LogEditManager ("successful file write [" + juce::String (numFilesProcessed) + "]: offset: " + juce::String (curSampleOffset) + ", numSamples: " + juce::String (samplesToRead));
                                if (! cueSetListVT.isValid () || numFilesProcessed > 1)
                                    cueSetList.emplace_back (CueSet { curSampleOffset, static_cast<uint32_t> (samplesToRead) });
                            }
                            else
                            {
                                // handle error
                                LogEditManager ("ERROR - when writing file");
                                hadError = true;
                            }
                            curSampleOffset += samplesToRead;
                        }
                        else
                        {
                            LogEditManager ("ERROR - file too long");
                        }
                    }
                    else // needs to be sample rate converted
                    {
                        const double ratio { 44100. / reader->sampleRate };
                        const int samplesToRead { static_cast<int> (reader->lengthInSamples * ratio) };
                        if (curSampleOffset + samplesToRead < kMaxSampleLength)
                        {
                            juce::AudioBuffer<float> outputBuffer;
                            sampleConvert (reader.get (), outputBuffer);
                            if (writer->writeFromAudioSampleBuffer (outputBuffer, 0, outputBuffer.getNumSamples ()) == true)
                            {
                                LogEditManager ("successful file write [" + juce::String (numFilesProcessed) + "]: offset: " + juce::String (curSampleOffset) + ", numSamples: " + juce::String (outputBuffer.getNumSamples ()));
                                if (! cueSetListVT.isValid () || numFilesProcessed > 1)
                                    cueSetList.emplace_back (CueSet { curSampleOffset, static_cast<uint32_t> (outputBuffer.getNumSamples ()) });
                            }
                            else
                            {
                                // handle error
                                LogEditManager ("ERROR - when writing file");
                                hadError = true;
                            }
                            curSampleOffset += outputBuffer.getNumSamples ();
                        }
                        else
                        {
                            LogEditManager ("ERROR - file too long");
                        }
                    }
                }
                else
                {
                    hadError = true;
                }
                if (curSampleOffset >= kMaxSampleLength || hadError)
                    break;
            }
        }
        else
        {
            LogEditManager ("ERROR - unable to open output file: " + outputFile.getFullPathName ());
            hadError = true;
        }
    }
    if (! hadError)
    {
        auto& channelProperties { channelPropertiesList [channelIndex] };
        // load file
        channelProperties.triggerLoadBegin (false);
        loadChannel (channelProperties.getValueTree (), static_cast<uint8_t> (channelIndex), outputFile);
        // remove any cue sets created by embedded markers
        channelProperties.setCurCueSet (0, false);
        for (auto cueSetCount { channelProperties.getNumCueSets () }; cueSetCount > 1; --cueSetCount)
            channelProperties.removeCueSet (cueSetCount - 1);
        channelProperties.triggerLoadComplete (false);
        // add original cue sets
        auto curCueSetIndex { 0 };
        if (cueSetListVT.isValid ())
        {
            ValueTreeHelpers::forEachChildOfType (cueSetListVT, SquidChannelProperties::CueSetTypeId, [this, &channelProperties, &curCueSetIndex] (juce::ValueTree cueSetVT)
            {
                channelProperties.setCueSetPoints (curCueSetIndex,
                                                   static_cast<int> (cueSetVT.getProperty (SquidChannelProperties::CueSetStartPropertyId)),
                                                   static_cast<int> (cueSetVT.getProperty (SquidChannelProperties::CueSetLoopPropertyId)),
                                                   static_cast<int> (cueSetVT.getProperty (SquidChannelProperties::CueSetEndPropertyId)));
                ++curCueSetIndex;
                return true;
            });
        }

        // add new cue sets
        for (auto cueSetIndex { 0 }; cueSetIndex < cueSetList.size (); ++cueSetIndex)
        {
            auto& cueSetListEntry { cueSetList [cueSetIndex] };
            channelProperties.setCueSetPoints (curCueSetIndex + cueSetIndex,
                                               SquidChannelProperties::sampleOffsetToByteOffset (cueSetListEntry.offset),
                                               SquidChannelProperties::sampleOffsetToByteOffset (cueSetListEntry.offset),
                                               SquidChannelProperties::sampleOffsetToByteOffset (cueSetListEntry.offset + cueSetListEntry.length));
        }
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

void EditManager::cloneCvAssigns (int srcChannelIndex, int srcCvAssignIndex, int destChannelIndex, int destCvAssignIndex)
{
    jassert (srcChannelIndex >= 0 && srcChannelIndex < 8);
    jassert (srcCvAssignIndex >= 0 && srcCvAssignIndex < 8);
    jassert (destChannelIndex >= 0 && destChannelIndex < 8);
    jassert (destCvAssignIndex >= 0 && destCvAssignIndex < 8);
    auto& srcChannelProperties { channelPropertiesList [srcChannelIndex] };
    auto& destChannelProperties { channelPropertiesList [destChannelIndex] };
    srcChannelProperties.forEachCvParameter (srcCvAssignIndex, [this, &destChannelProperties, destCvAssignIndex] (juce::ValueTree srcParameterVT)
    {
        CvParameterProperties srcCvParameterProperties { srcParameterVT, CvParameterProperties::WrapperType::client, CvParameterProperties::EnableCallbacks::no };
        const auto cvParameterId { srcCvParameterProperties.getId () };
        CvParameterProperties dstCvParameterProperties { destChannelProperties.getCvParameterVT (destCvAssignIndex, cvParameterId), CvParameterProperties::WrapperType::client, CvParameterProperties::EnableCallbacks::no };
        dstCvParameterProperties.setEnabled (srcCvParameterProperties.getEnabled (), false);
        dstCvParameterProperties.setAttenuation (srcCvParameterProperties.getAttenuation (), false);
        dstCvParameterProperties.setOffset (srcCvParameterProperties.getOffset (), false);
        return true;
    });
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

juce::ValueTree EditManager::getChannelPropertiesVT (int channelIndex)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    return channelPropertiesList [channelIndex].getValueTree ();
}

juce::PopupMenu EditManager::createChannelInteractionMenu (int channelIndex, juce::String interactionArticle,
                                                           std::function <void (SquidChannelProperties&)> setter,
                                                           std::function <bool (SquidChannelProperties&)> canCloneCallback,
                                                           std::function <bool (SquidChannelProperties&)> canCloneToAllCallback)
{
    jassert (setter != nullptr);
    jassert (canCloneCallback != nullptr);
    jassert (canCloneToAllCallback != nullptr);
    //const auto channelIndex { squidChannelProperties.getChannelIndex () };
    juce::PopupMenu cloneMenu;
    for (auto destChannelIndex { 0 }; destChannelIndex < 8; ++destChannelIndex)
    {
        if (destChannelIndex != channelIndex)
        {
            auto canCloneToDestChannel { true };
            forChannels ({ destChannelIndex }, [this, canCloneCallback, &canCloneToDestChannel] (juce::ValueTree channelPropertiesVT)
            {
                SquidChannelProperties destChannelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                canCloneToDestChannel = canCloneCallback (destChannelProperties);
            });
            if (canCloneToDestChannel)
            {
                cloneMenu.addItem (interactionArticle + " Channel " + juce::String (destChannelIndex + 1), true, false, [this, destChannelIndex, setter] ()
                {
                    forChannels ({ destChannelIndex }, [this, setter] (juce::ValueTree channelPropertiesVT)
                    {
                        SquidChannelProperties destChannelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                        setter (destChannelProperties);
                    });
                });
            }
        }
    }
    std::vector<int> channelIndexList;
    // build list of other channels
    for (auto destChannelIndex { 0 }; destChannelIndex < 8; ++destChannelIndex)
        if (destChannelIndex != channelIndex)
            channelIndexList.emplace_back (destChannelIndex);
    auto canCloneDestChannelCount { 0 };
    forChannels ({ channelIndexList }, [this, canCloneToAllCallback, &canCloneDestChannelCount] (juce::ValueTree channelPropertiesVT)
    {
        SquidChannelProperties destChannelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
        if (canCloneToAllCallback (destChannelProperties))
            ++canCloneDestChannelCount;
    });
    if (canCloneDestChannelCount > 0)
    {
        cloneMenu.addItem (interactionArticle + " All", true, false, [this, setter, channelIndexList] ()
        {
            // clone to other channels
            forChannels (channelIndexList, [this, setter] (juce::ValueTree channelPropertiesVT)
            {
                SquidChannelProperties destChannelProperties (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::no);
                setter (destChannelProperties);
            });
        });
    }
    return cloneMenu;
}

juce::PopupMenu EditManager::createChannelEditMenu (juce::PopupMenu existingPopupMenu, int channelIndex, std::function <void (SquidChannelProperties&)> setter, std::function <void ()> resetter, std::function <void ()> reverter)
{
    juce::PopupMenu editMenu (existingPopupMenu);
    editMenu.addSubMenu ("Clone", createChannelInteractionMenu (channelIndex, "To", setter, [this] (SquidChannelProperties&) { return true; }, [this] (SquidChannelProperties&) { return true; }), true);
    if (resetter != nullptr)
        editMenu.addItem ("Default", true, false, [this, resetter] () { resetter (); });
    if (reverter != nullptr)
        editMenu.addItem ("Revert", true, false, [this, reverter] () { reverter (); });

    return editMenu;
};

std::unique_ptr<juce::AudioFormatReader> EditManager::getReaderFor (const juce::File file)
{
    return std::unique_ptr<juce::AudioFormatReader> (audioFormatManager.createReaderFor (file));
}

juce::String EditManager::getFileTypesList ()
{
    juce::String fileTypesList;
    for (auto fileExtension : audioFileExtensions)
        fileTypesList += juce::String (fileTypesList.length () == 0 ? "" : ";") + "*" + fileExtension;
    return fileTypesList;
}

bool EditManager::isSquidSalmpleSupportedAudioFile (const juce::File file)
{
    if (file.getFileExtension ().toLowerCase () != ".wav")
        return false;
    if (auto reader (getReaderFor (file)); reader != nullptr)
    {
        return reader->usesFloatingPointData == false &&
               (reader->bitsPerSample >= 16 && reader->bitsPerSample <= 24) &&
               (reader->numChannels >= 1 && reader->numChannels <= 2) &&
               reader->sampleRate == 44100;
    }
    return false;
}

bool EditManager::isSquidManagerSupportedAudioFile (const juce::File file)
{
    return audioFileExtensions.contains (file.getFileExtension (), true);
}

bool EditManager::copySampleToChannel (juce::File srcFile, juce::File destFile)
{
    if (isSquidSalmpleSupportedAudioFile (srcFile))
    {
        // TODO handle case where file of same name already exists
        // TODO should copy be moved to a thread?
        // since we are copying the file from elsewhere, we will save it to a file with a "magic" extension
        // this is so we can undo things if the Bank is not saved, and we don't use the normal 'wav' extension
        // in case the app crashes, or something, and the extra file would confuse the module
        srcFile.copyFileTo (destFile);
        // TODO handle failure
    }
    else
    {
        // convert file from current location to channel directory
        auto destinationFileStream { std::make_unique<juce::FileOutputStream> (destFile) };
        destinationFileStream->setPosition (0);
        destinationFileStream->truncate ();

        if (auto reader { getReaderFor (srcFile) }; reader != nullptr)
        {
            auto numChannels { reader->numChannels };
            auto bitsPerSample { reader->bitsPerSample };

            if (bitsPerSample < 16)
                bitsPerSample = 16;
            else if (bitsPerSample > 24)
                bitsPerSample = 24;
            jassert (numChannels != 0);
            numChannels = 1;

            // TODO - if the srcFile is a wav file, we should check to see if we need to copy the markers, and then add those to the destination file
            juce::WavAudioFormat wavAudioFormat;
            if (std::unique_ptr<juce::AudioFormatWriter> writer { wavAudioFormat.createWriterFor (destinationFileStream.get (),
                                                                  44100, numChannels, bitsPerSample, {}, 0) }; writer != nullptr)
            {
                // audioFormatWriter will delete the file stream when done
                destinationFileStream.release ();

                if (reader->bitsPerSample == 44100)
                {
                    // copy the whole thing
                    // TODO - two things
                    //   a) this needs to be done in a thread
                    //   b) we should locally read into a buffer and then write that, so we can display progress if needed
                    if (writer->writeFromAudioReader (*reader.get (), 0, -1) == true)
                    {
                        // close the writer and reader, so that we can manipulate the files
                        writer.reset ();
                        reader.reset ();
                    }
                    else
                    {
                        // failure to convert
                        jassertfalse;
                        return false;
                    }
                }
                else
                {
                    juce::AudioBuffer<float> outputBuffer;
                    sampleConvert (reader.get (), outputBuffer);
                    if (writer->writeFromAudioSampleBuffer (outputBuffer, 0, outputBuffer.getNumSamples ()) == true)
                    {
                        // close the writer and reader, so that we can manipulate the files
                        writer.reset ();
                        reader.reset ();
                    }
                    else
                    {
                        // failure to convert
                        jassertfalse;
                        return false;
                    }
                }
            }
            else
            {
                //failure to create writer
                jassertfalse;
                return false;
            }
        }
        else
        {
            // failure to create reader
            jassertfalse;
            return false;
        }
    }

    return true;
}

int EditManager::findNextZeroCrossing (int startSampleOffset, int maxSampleOffset, juce::AudioBuffer<float>& buffer)
{
    if (startSampleOffset < 0 || startSampleOffset >= maxSampleOffset - 1)
        return -1; // Invalid start position

    auto readPtr { buffer.getReadPointer (0) };
    for (auto i = startSampleOffset + 1; i < maxSampleOffset - 1; ++i)
    {
        if ((readPtr [i] > epsilon && readPtr [i + 1] <= epsilon) || (readPtr [i] < epsilon && readPtr [i + 1] >= epsilon))
        {
            return i; // Return the index of the zero crossing
        }
    }
    return -1; // No zero crossing found
}

int EditManager::findPreviousZeroCrossing (int startSampleOffset, int minSampleOffset, juce::AudioBuffer<float>& buffer)
{
    if (startSampleOffset <= minSampleOffset || startSampleOffset > buffer.getNumSamples ())
        return -1; // Invalid start position

    auto readPtr { buffer.getReadPointer (0) };
    for (auto i = startSampleOffset - 1; i > minSampleOffset; --i)
    {
        if ((readPtr [i] > epsilon && readPtr [i - 1] <= epsilon) || (readPtr [i] < epsilon && readPtr [i - 1] >= epsilon))
        {
            return i - 1; // Return the index of the zero crossing
        }
    }
    return -1; // No zero crossing found
}
