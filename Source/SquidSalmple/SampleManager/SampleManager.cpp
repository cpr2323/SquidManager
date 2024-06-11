#include "SampleManager.h"
#include "../SquidBankProperties.h"
#include "../Bank/BankManagerProperties.h"
#include "../../Utility/DebugLog.h"
#include "../../Utility/PersistentRootProperties.h"

#define LOG_SAMPLE_POOL 1
#if LOG_SAMPLE_POOL
#define LogSamplePool(text) DebugLog ("SampleManager", text);
#else
#define LogSamplePool(text) ;
#endif

SampleManager::SampleManager ()
{
    audioFormatManager.registerBasicFormats ();
}

void SampleManager::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);
    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::owner, SampleManagerProperties::EnableCallbacks::no);

    auto closeAllOpenSamples = [this] ()
    {
        // reset all samples being used
        for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
            handleSampleChange (channelIndex, "");
    };
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    currentFolder = juce::File (appProperties.getMostRecentFolder ());
    appProperties.onMostRecentFolderChange = [this, closeAllOpenSamples] (juce::String folderName)
    {
        closeAllOpenSamples ();
        currentFolder = { juce::File (folderName) };
    };

    BankManagerProperties bankManagerProperties (runtimeRootProperties.getValueTree (), BankManagerProperties::WrapperType::owner, BankManagerProperties::EnableCallbacks::no);
    SquidBankProperties squidBankProperties (bankManagerProperties.getBank ("edit"), SquidBankProperties::WrapperType::client, SquidBankProperties::EnableCallbacks::yes);
    squidBankProperties.forEachChannel ([this] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        channelAndSamplePropertiesList [channelIndex].squidChannelProperties.wrap (channelPropertiesVT, SquidChannelProperties::WrapperType::client, SquidChannelProperties::EnableCallbacks::yes);
        channelAndSamplePropertiesList [channelIndex].squidChannelProperties.onFileNameChange = [this, channelIndex] (juce::String fileName) { handleSampleChange (channelIndex, fileName); };
        channelAndSamplePropertiesList [channelIndex].sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (channelIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::yes);
        return true;
    });
}

juce::ValueTree SampleManager::getSampleProperties (int channelIndex)
{
    return channelAndSamplePropertiesList [channelIndex].sampleProperties.getValueTree ();
}

bool SampleManager::isSupportedAudioFile (juce::File file)
{
    // 16 or 24 bit
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

void SampleManager::handleSampleChange (int channelIndex, juce::String sampleName)
{
    // NOTE: at first I was going to have and assert if the sampleName was the same as the currently loaded sample, but then I realized that one could load a new version of a sample (from an external folder)
    //       and we would need to re-open it to get all of the pertinent data.
    // TODO: Does this also means we need to make sure, when loading a sample that we bypass the ValueTree feature of not executing callbacks for same data, or do we set the name to empty before loading, which
    //       would enable the callbacks to happen correctly?
    //       ** Currently I think we need to reset the sampleName to "" before setting it to a new value, so it will force a reloading of the sample data
    auto& sampleProperties { channelAndSamplePropertiesList [channelIndex].sampleProperties };

    // if there is an already open sample, we want to close it
    if (sampleProperties.getName ().isNotEmpty ())
    {
        LogSamplePool ("handleSampleChangeclosing sample '" + sampleProperties.getName () + " 'for c" + juce::String (channelIndex));
        sampleProperties.setStatus (SampleStatus::uninitialized, false); // this should inform clients to stop using the sample, before we reset everything else
        close (sampleProperties.getName ());
        sampleProperties.setAudioBufferPtr (nullptr, false);
        sampleProperties.setBitsPerSample (0, false);
        sampleProperties.setSampleRate (0.0, false);
        sampleProperties.setLengthInSamples (0, false);
        sampleProperties.setName ("", false);
        sampleProperties.setNumChannels (0, false);
    }

    // if there is a new sample coming in (vs sample being reset) we want to open it
    if (sampleName.isNotEmpty ())
    {
        LogSamplePool ("handleSampleChange: opening sample '" + sampleName + " 'for c" + juce::String (channelIndex) + "/z");
        auto& sampleData { open (sampleName) };
        sampleProperties.setName (sampleName, false);
        sampleProperties.setBitsPerSample (sampleData.bitsPerSample, false);
        sampleProperties.setSampleRate (sampleData.sampleRate, false);
        sampleProperties.setLengthInSamples (sampleData.lengthInSamples, false);
        sampleProperties.setNumChannels (sampleData.numChannels, false);
        sampleProperties.setAudioBufferPtr (&sampleData.audioBuffer, false);
        sampleProperties.setStatus (sampleData.status, false);
    }
}

SampleManager::SampleData& SampleManager::open (juce::String fileName)
{
    LogSamplePool ("open: " + fileName);
    jassert (fileName.isNotEmpty ());
    jassert (currentFolder.exists ());
    const auto sampleDataIter { sampleList.find (fileName) };
    if (sampleDataIter == sampleList.end ())
        return loadSample (fileName);

    sampleDataIter->second.useCount++;
    LogSamplePool ("open: useCount:" + juce::String (sampleList [fileName].useCount));
    return (*sampleDataIter).second;
}

SampleManager::SampleData& SampleManager::loadSample (juce::String fileName)
{
    LogSamplePool ("loadSample: " + fileName);
    SampleData newSampleData;
    newSampleData.useCount = 1;
    updateSample (fileName, newSampleData);
    LogSamplePool ("loadSample: useCount: 1");

    sampleList [fileName] = std::move (newSampleData);
    return sampleList [fileName];
}

void SampleManager::updateSampleProperties (juce::String fileName, SampleData& sampleData)
{
    for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
    {
        auto& sampleProperties { channelAndSamplePropertiesList [channelIndex].sampleProperties };
        if (sampleProperties.getName () == fileName)
        {
            sampleProperties.setBitsPerSample (sampleData.bitsPerSample, false);
            sampleProperties.setSampleRate (sampleData.sampleRate, false);
            sampleProperties.setLengthInSamples (sampleData.lengthInSamples, false);
            sampleProperties.setNumChannels (sampleData.numChannels, false);
            sampleProperties.setAudioBufferPtr (&sampleData.audioBuffer, false);
            sampleProperties.setStatus (sampleData.status, false);
        }
    }
}

void SampleManager::updateSample (juce::String fileName, SampleData& sampleData)
{
    juce::File fullPath { currentFolder.getChildFile (fileName) };
    if (fullPath.exists ())
    {
        LogSamplePool ("updateSample: file exists");
        if (std::unique_ptr<juce::AudioFormatReader> sampleFileReader { audioFormatManager.createReaderFor (fullPath) }; sampleFileReader != nullptr)
        {
            // cache sample attributes
            sampleData.status = SampleStatus::exists;
            sampleData.bitsPerSample = sampleFileReader->bitsPerSample;
            sampleData.sampleRate = sampleFileReader->sampleRate;
            sampleData.numChannels = sampleFileReader->numChannels;
            sampleData.lengthInSamples = sampleFileReader->lengthInSamples;

            // read in audio data
            sampleData.audioBuffer.setSize (sampleData.numChannels, static_cast<int> (sampleData.lengthInSamples), false, true, false);
            sampleFileReader->read (&sampleData.audioBuffer, 0, static_cast<int> (sampleData.lengthInSamples), 0, true, false);
            updateSampleProperties (fileName, sampleData);
        }
        else
        {
            LogSamplePool ("updateSample: wrong format");
            sampleData.status = SampleStatus::wrongFormat;
        }
    }
    else
    {
        LogSamplePool ("updateSample: does not exist");
        sampleData.status = SampleStatus::doesNotExist;
    }
}

void SampleManager::clear ()
{
    sampleList.clear ();
}

void SampleManager::update ()
{
    for (auto& [fileName, SampleDataInternal] : sampleList)
        updateSample (fileName, SampleDataInternal);
}

void SampleManager::close (juce::String fileName)
{
    LogSamplePool ("close: " + fileName);
    //dumpStacktrace (-1, [this] (juce::String logLine) { DebugLog ("SamplePool", logLine); });
    jassert (fileName.isNotEmpty ());
    [[maybe_unused]] const auto sampleDataIter { sampleList.find (fileName) };
    jassert (sampleDataIter != sampleList.end ());
    jassert (sampleList [fileName].useCount != 0);
    --sampleList [fileName].useCount;
    LogSamplePool ("close: useCount:" + juce::String (sampleList [fileName].useCount));
    if (sampleList [fileName].useCount == 0)
    {
        sampleList.erase (fileName);
        LogSamplePool ("close: deleting");
    }
}
