#include <JuceHeader.h>
#include "AppProperties.h"
#include "SystemServices.h"
#include "GUI/GuiProperties.h"
#include "GUI/MainComponent.h"
#include "SquidSalmple/Audio/AudioPlayer.h"
#include "SquidSalmple/Bank/BankManagerProperties.h"
#include "SquidSalmple/SquidBankProperties.h"
#include "oolib/Debug/DebugLog.h"
#include "oolib/Debug/ValueTreeMonitor.h"
#include "oolib/Directory/DirectoryValueTree.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"
#include "oolib/ValueTree/ValueTreeFile.h"

constexpr const char* kVersionDecorator { "" };

// this requires the third party Melatonin Inspector be installed and added to the project
// https://github.com/sudara/melatonin_inspector
#define ENABLE_MELATONIN_INSPECTOR 0

// set to 1 to log how long the message thread spends unresponsive. useful when moving work on to,
// or off of, the message thread, to check that the UI still redraws and handles input promptly
#define ENABLE_MESSAGE_THREAD_STALL_MONITOR 0

#if ENABLE_MESSAGE_THREAD_STALL_MONITOR
// times the gaps between the callbacks of a fast timer. anything beyond the timer interval is time
// the message thread was busy, and therefore time the UI was frozen
class MessageThreadStallMonitor : private juce::Timer
{
public:
    MessageThreadStallMonitor ()
    {
        lastCallbackTime = juce::Time::getMillisecondCounterHiRes ();
        startTimer (kIntervalMs);
    }
    ~MessageThreadStallMonitor ()
    {
        stopTimer ();
        report ();
    }

    void report ()
    {
        juce::Logger::writeToLog ("[stall monitor] callbacks: " + juce::String (numCallbacks) +
                                  ", worst stall: " + juce::String (worstStallMs, 1) + "ms" +
                                  ", stalls > 16ms: " + juce::String (numStallsOver16) +
                                  ", stalls > 50ms: " + juce::String (numStallsOver50) +
                                  ", stalls > 100ms: " + juce::String (numStallsOver100));
    }

private:
    static constexpr int kIntervalMs { 10 };
    double lastCallbackTime {};
    double worstStallMs {};
    int numCallbacks {};
    int numStallsOver16 {};
    int numStallsOver50 {};
    int numStallsOver100 {};

    void timerCallback () override
    {
        const auto now { juce::Time::getMillisecondCounterHiRes () };
        const auto stallMs { now - lastCallbackTime - kIntervalMs };
        lastCallbackTime = now;
        ++numCallbacks;
        if (stallMs > worstStallMs)
        {
            worstStallMs = stallMs;
            juce::Logger::writeToLog ("[stall monitor] new worst stall: " + juce::String (stallMs, 1) + "ms");
        }
        if (stallMs > 100.0)
            ++numStallsOver100;
        else if (stallMs > 50.0)
            ++numStallsOver50;
        else if (stallMs > 16.0)
            ++numStallsOver16;
    }
};
#endif

const juce::String PropertiesFileExtension { ".properties" };

void crashHandler (void* /*data*/)
{
    FlushDebugLog ();
    juce::Logger::writeToLog (juce::SystemStats::getStackBacktrace ());
    FlushDebugLog ();
}

class SquidManagerApplication : public juce::JUCEApplication, public juce::Timer
{
public:
    SquidManagerApplication () {}
    const juce::String getApplicationName () override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion () override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed () override { return true; }

    void initialise ([[maybe_unused]] const juce::String& commandLine) override
    {
        initAppDirectory ();
        initLogger ();
        initCrashHandler ();
        initPropertyRoots ();
        initSquidSalmple ();
        initAudio ();
        initSystemServices ();

        initUi ();

        //ValueTreeHelpers::dumpValueTreeContent (rootProperties.getValueTree (), false, [] (juce::String text) { DebugLog ("main", text); });

#if ENABLE_MESSAGE_THREAD_STALL_MONITOR
        messageThreadStallMonitor = std::make_unique<MessageThreadStallMonitor> ();
#endif

        // async quit timer
        startTimer (125);
    }

    void shutdown () override
    {
#if ENABLE_MESSAGE_THREAD_STALL_MONITOR
        messageThreadStallMonitor.reset ();
#endif
        persitentPropertiesFile.save ();
        mainWindow = nullptr; // (deletes our window)
        juce::Logger::setCurrentLogger (nullptr);
    }

    void anotherInstanceStarted ([[maybe_unused]] const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    void suspended () override
    {
        runtimeRootProperties.triggerAppSuspended (false);
    }

    void resumed () override
    {
        runtimeRootProperties.triggerAppResumed (false);
    }

    void systemRequestedQuit () override
    {
        // reset preferred quit state
        runtimeRootProperties.setPreferredQuitState (RuntimeRootProperties::QuitState::now, false);
        // listeners for 'onSystemRequestedQuit' can do runtimeRootProperties.setPreferredQuitState (RuntimeRootProperties::QuitState::idle);
        // if they need to do something, which also makes them responsible for calling runtimeRootProperties.setQuitState (RuntimeRootProperties::QuitState::now); when they are done...
        runtimeRootProperties.triggerSystemRequestedQuit (false);
        localQuitState.store (runtimeRootProperties.getPreferredQuitState ());
    }

    void timerCallback () override
    {
        if (localQuitState.load () == RuntimeRootProperties::QuitState::now)
            quit ();
    }

    void initSquidSalmple ()
    {
        SquidBankProperties squidBankProperties ({}, SquidBankProperties::WrapperType::owner, SquidBankProperties::EnableCallbacks::no);

        BankManagerProperties bankManagerProperties (runtimeRootProperties.getValueTree (), BankManagerProperties::WrapperType::owner, BankManagerProperties::EnableCallbacks::no);
        bankManagerProperties.addBank ("edit", squidBankProperties.getValueTree ());
        bankManagerProperties.addBank ("unedited", squidBankProperties.getValueTree ().createCopy ());

        // add the Bank Manager to the Runtime Root
        runtimeRootProperties.getValueTree ().addChild (bankManagerProperties.getValueTree (), -1, nullptr);

        // setup the directory scanner
        directoryValueTree.init (runtimeRootProperties.getValueTree ());
        directoryDataProperties.wrap (directoryValueTree.getDirectoryDataPropertiesVT (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::no);
        directoryDataProperties.setScanDepth (0, false);
        // a Squid Salmple drive holds folders named "Bank <n>", which have to be ordered by that number
        // rather than by name, so that Bank 2 comes before Bank 10
        directoryValueTree.setComparatorForType (DirectoryValueTree::folderTypeId, [] (juce::String firstName, juce::String secondName)
        {
            // the names are full paths, and the bank number is only meaningful in the file name
            auto asBankSortKey = [] (juce::String fullPath)
            {
                auto folderName { juce::File (fullPath).getFileName ().toLowerCase () };
                auto bankId { 0 };
                if (folderName.substring (0, 5) == "bank ")
                {
                    bankId = folderName.substring (5).getIntValue ();
                    // every "Bank <n>" folder collapses to the same name, so the number does the ordering
                    if (bankId > 0)
                        folderName = "bank ";
                }
                return std::tuple<juce::String, int> { folderName, bankId };
            };
            const auto [firstFolderName, firstBankId] { asBankSortKey (firstName) };
            const auto [secondFolderName, secondBankId] { asBankSortKey (secondName) };
            return firstFolderName < secondFolderName || (firstFolderName == secondFolderName && firstBankId < secondBankId);
        });
        // debug tool for watching changes on the Directory Data Properties Value Tree
        //directoryDataMonitor.assign (directoryDataProperties.getValueTreeRef ());

        // when the folder being viewed changes, signal the directory scanner to rescan
        appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
        {
            directoryDataProperties.setRootFolder (folderName, false);
            directoryDataProperties.triggerStartScan (false);
        };
    }

    void initUi ()
    {
        guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::owner, GuiProperties::EnableCallbacks::no);
        mainWindow.reset (new MainWindow (getApplicationName () + " - " + getVersionDisplayString (), rootProperties.getValueTree ()));
    }

    void initPropertyRoots ()
    {
        persistentRootProperties.wrap (rootProperties.getValueTree (), PersistentRootProperties::WrapperType::owner, PersistentRootProperties::EnableCallbacks::no);
        // connect the Properties file and the AppProperties ValueTree with the propertiesFile (ValueTreeFile with auto-save)
        persitentPropertiesFile.init (persistentRootProperties.getValueTree (), appDirectory.getChildFile ("app" + PropertiesFileExtension), true);
        appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::owner, AppProperties::EnableCallbacks::yes);
        appProperties.setMaxMruEntries (1);
        runtimeRootProperties.wrap (rootProperties.getValueTree (), RuntimeRootProperties::WrapperType::owner, RuntimeRootProperties::EnableCallbacks::yes);
        runtimeRootProperties.setAppVersion (getApplicationVersion (), false);
        runtimeRootProperties.setAppDataPath (appDirectory.getFullPathName (), false);
        runtimeRootProperties.onQuitStateChanged = [this] (RuntimeRootProperties::QuitState quitState) { localQuitState.store (quitState); };

        if (appProperties.getMostRecentFolder ().isEmpty ())
            appProperties.setMostRecentFolder (appDirectory.getFullPathName ());
    }

    void initAudio ()
    {
        audioPlayer.init (rootProperties.getValueTree ());
    }

    void initAppDirectory ()
    {
        // locate the appProperties file in the User Application Data Directory

        const juce::String propertiesFilePath { juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getFullPathName () };
        appDirectory = juce::File (propertiesFilePath).getChildFile (ProjectInfo::companyName).getChildFile (getApplicationName ());
        if (! appDirectory.exists ())
        {
            const auto result { appDirectory.createDirectory () };
            if (! result.wasOk ())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Application Startup Error",
                    "Unable to create " + getApplicationName () + " preferences directory, '" + appDirectory.getFullPathName () + "'", {}, nullptr,
                    juce::ModalCallbackFunction::create ([this] (int) { quit (); }));
                return;
            }
        }
    }

    juce::String getVersionDisplayString ()
    {
        return "v" + getApplicationVersion () + juce::String (kVersionDecorator);
    }

    void initLogger ()
    {
        auto getSessionTextForLogFile = [this] ()
        {
            auto resultOrNa = [] (juce::String result)
            {
                if (result.isEmpty ())
                    return juce::String ("n/a");
                else
                    return result;
            };
            const auto nl { juce::String ("\n") };
            auto welcomeText { juce::String (getApplicationName () + " - " + getVersionDisplayString () + " Log File" + nl) };
            welcomeText += " OS: " + resultOrNa (juce::SystemStats::getOperatingSystemName ()) + nl;
            welcomeText += " Device Description: " + resultOrNa (juce::SystemStats::getDeviceDescription ()) + nl;
            welcomeText += " Device Manufacturer: " + resultOrNa (juce::SystemStats::getDeviceManufacturer ()) + nl;
            welcomeText += " CPU Vendor: " + resultOrNa (juce::SystemStats::getCpuVendor ()) + nl;
            welcomeText += " CPU Model: " + resultOrNa (juce::SystemStats::getCpuModel ()) + nl;
            welcomeText += " CPU Speed: " + resultOrNa (juce::String (juce::SystemStats::getCpuSpeedInMegahertz ())) + nl;
            welcomeText += " Logical/Physicals CPUs: " + resultOrNa (juce::String (juce::SystemStats::getNumCpus ())) + "/" + resultOrNa (juce::String (juce::SystemStats::getNumPhysicalCpus ())) + nl;
            welcomeText += " Memory: " + resultOrNa (juce::String (juce::SystemStats::getMemorySizeInMegabytes ())) + "mb" + nl;
            return welcomeText;
        };
        fileLogger = std::make_unique<juce::FileLogger> (appDirectory.getChildFile ("DebugLog"), getSessionTextForLogFile ());
        juce::Logger::setCurrentLogger (fileLogger.get ());
    }

    void initCrashHandler ()
    {
        juce::SystemStats::setApplicationCrashHandler (crashHandler);
    }

    void initSystemServices ()
    {
        // initialize services
        editManager.init (rootProperties.getValueTree ());

        // connect services to the SystemServices VTW
        SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::owner, SystemServices::EnableCallbacks::no);
        systemServices.setEditManager (&editManager);

        // the directory scanner knows nothing about Squid Salmple samples, so the audio file type is
        // registered here, where the EditManager that decides what counts as one is available.
        // both callbacks run on the scan thread
        directoryValueTree.registerFileType (kAudioFileTypeName,
            [this] (juce::File file) { return editManager.getFileInfo (file).supported; },
            [this] (juce::ValueTree entryVT, juce::File file)
            {
                const auto fileInfo { editManager.getFileInfo (file) };
                if (! fileInfo.supported)
                {
                    // the file changed between being identified and being described
                    entryVT.setProperty ("error", "invalid format", nullptr);
                    return;
                }
                entryVT.setProperty ("dataType", (fileInfo.usesFloatingPointData == true ? "floating point" : "integer"), nullptr);
                entryVT.setProperty ("bitDepth", static_cast<int> (fileInfo.bitsPerSample), nullptr);
                entryVT.setProperty ("numChannels", static_cast<int> (fileInfo.numChannels), nullptr);
                entryVT.setProperty ("sampleRate", static_cast<int> (fileInfo.sampleRate), nullptr);
                entryVT.setProperty ("lengthSamples", static_cast<juce::int64> (fileInfo.lengthInSamples), nullptr);
            });

        // start the initial directory scan, based on the last accessed folder stored in the app properties
        directoryDataProperties.setRootFolder (appProperties.getMostRecentFolder (), false);
        directoryDataProperties.triggerStartScan (false);
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name, juce::ValueTree rootPropertiesVT)
            : DocumentWindow (name,
                              juce::Desktop::getInstance ().getDefaultLookAndFeel ().findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (rootPropertiesVT), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
           #endif

            PersistentRootProperties prp (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
            guiProperties.wrap (prp.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);
            auto [width, height] { guiProperties.getSize () };
            auto [x, y] { guiProperties.getPosition () };
            setResizeLimits (120, 523, 1475, 65000);
            if (x == -1 || y == -1)
                centreWithSize (width, height);
            else
                setBounds (x, y, width, height);

            setVisible (true);

#if ENABLE_MELATONIN_INSPECTOR
            inspector.setVisible (true);
#endif
        }

#if (! JUCE_IOS) && (! JUCE_ANDROID)
        void moved () override
        {
            guiProperties.setPosition (getBounds ().getX (), getBounds ().getY (), false);
            DocumentWindow::moved ();
        }

        void resized () override
        {
            guiProperties.setSize (getBounds ().getWidth (), getBounds ().getHeight (), false);
            DocumentWindow::resized ();
        }
#endif // ! JUCE_IOS && ! JUCE_ANDROID

        void closeButtonPressed () override
        {
            JUCEApplication::getInstance ()->systemRequestedQuit ();
        }

    private:
        GuiProperties guiProperties;
#if ENABLE_MELATONIN_INSPECTOR
        melatonin::Inspector inspector { *this, false };
#endif
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    juce::File appDirectory;
    RootProperties rootProperties;
    ValueTreeFile persitentPropertiesFile;
    PersistentRootProperties persistentRootProperties;
    AppProperties appProperties;
    GuiProperties guiProperties;
    RuntimeRootProperties runtimeRootProperties;
    DirectoryValueTree directoryValueTree;
    DirectoryDataProperties directoryDataProperties;
    std::unique_ptr<juce::FileLogger> fileLogger;
    std::atomic<RuntimeRootProperties::QuitState> localQuitState { RuntimeRootProperties::QuitState::idle };
    std::unique_ptr<MainWindow> mainWindow;
    AudioPlayer audioPlayer;
#if ENABLE_MESSAGE_THREAD_STALL_MONITOR
    std::unique_ptr<MessageThreadStallMonitor> messageThreadStallMonitor;
#endif

    // System Services
    EditManager editManager;

#if JUCE_DEBUG
    ValueTreeMonitor audioConfigPropertiesMonitor;
    ValueTreeMonitor directoryDataMonitor;
    ValueTreeMonitor presetPropertiesMonitor;
#endif
};

// This macro generates the main () routine that launches the app.
START_JUCE_APPLICATION (SquidManagerApplication)
