#pragma once

#include <JuceHeader.h>
#include "DirectoryDataProperties.h"
#include "../Utility/LambdaThread.h"
#include "../Utility/ValueTreeMonitor.h"
#include "../Utility/WatchDogTimer.h"

using FileTypeIdentifierCallback = std::function<int (juce::File)>;

class DirectoryValueTree : public juce::Thread,
                           private juce::Timer,
                           private juce::AsyncUpdater
{
public:
    DirectoryValueTree ();
    ~DirectoryValueTree ();

    void init (juce::ValueTree rootPropertiesVT);
    juce::ValueTree getDirectoryDataPropertiesVT ();
    void setFileTypeIdentifier (FileTypeIdentifierCallback theFileTypeIdentifierCallback);

private:
    enum class ScanType
    {
        checkForUpdate,
        fullScan
    };
    enum class TaskManagementState
    {
        idle,
        startScan,
        scanning,
        startCheck,
        checking,
    };
    WatchdogTimer timer; // TODO - remove when not needed, ie. when done measuring things
    DirectoryDataProperties directoryDataProperties;
    juce::AudioFormatManager audioFormatManager;
    LambdaThread scanThread { "ScanThread", 1000 };
    LambdaThread checkThread { "CheckThread", 1000 };
    FileTypeIdentifierCallback fileTypeIdentifierCallback;

    // a summary of one directory entry, holding just the fields the change check compares.
    // the scan/check threads work with these instead of reading the shared ValueTree
    struct DirectoryEntrySummary
    {
        juce::String name;
        juce::int64 createTime {};
        juce::int64 modificationTime {};
        bool isFolder {};

        bool operator== (const DirectoryEntrySummary& other) const
        {
            return isFolder == other.isFolder && createTime == other.createTime &&
                   modificationTime == other.modificationTime && name == other.name;
        }
        bool operator!= (const DirectoryEntrySummary& other) const { return ! operator== (other); }
    };

    int scanDepth { -1 };
    juce::int64 lastScanInProgressUpdate {};
    std::atomic<bool> cancelScan { false };
    std::atomic<bool> cancelCheck { false };
    juce::CriticalSection taskManagementCS;
    TaskManagementState requestedTaskManagementState { TaskManagementState::idle };
    TaskManagementState currentTaskManagementState { TaskManagementState::idle };
    std::atomic<ScanType> scanType { ScanType::fullScan };

    // the shared (published) ValueTree is only ever touched on the message thread. these members
    // carry everything the scan/check threads need, so that they never have to read it
    juce::CriticalSection scanStateCS;
    juce::String rootFolderNameForScan;
    std::vector<DirectoryEntrySummary> lastScanSummary;

    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    void doProgressUpdate (juce::String progressString);
    void getContentsOfFolder (juce::ValueTree folderVT, int curDepth, std::function<bool ()> shouldCancelFunc);
    juce::String getPathFromCurrentRoot (juce::String fullPath);
    TaskManagementState getCurrentTaskManagementState ();
    juce::String getTaskManagementStateString (TaskManagementState theThreadState);
    TaskManagementState getRequestedTaskManagementState ();
    bool hasFolderChanged ();
    juce::ValueTree makeFileEntry (juce::File file, juce::int64 createTime, juce::int64 modificationTime, DirectoryDataProperties::TypeIndex fileType);
    void publishScanResults (juce::ValueTree scanResultsVT);
    void scanDirectory ();
    void sendStatusUpdate (DirectoryDataProperties::ScanStatus scanStatus);
    void setCurrentTaskManagementState (DirectoryValueTree::TaskManagementState newThreadState);
    void setScanDepth (int theScanDepth);
    bool setRequestedTaskManagementState (DirectoryValueTree::TaskManagementState newThreadState);
    bool shouldCancelOperation (LambdaThread& whichTaskThread, std::atomic<bool>& whichTaskCancelToCheck);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT, std::function<bool ()> shouldCancelFunc);
    void startScan ();
    std::vector<DirectoryEntrySummary> summarizeFolder (juce::ValueTree folderVT);
    std::vector<DirectoryEntrySummary> getLastScanSummary ();
    void setLastScanSummary (std::vector<DirectoryEntrySummary> summary);
    juce::String getRootFolderNameForScan ();
    void cacheRootFolderNameForScan ();
    void wakeUpTaskManagmentThread ();

    ValueTreeMonitor ddpMonitor;
    ValueTreeMonitor rootFolderMonitor;

    void run () override;
    void timerCallback () override;
    void handleAsyncUpdate () override;
};
