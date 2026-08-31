cmake_minimum_required(VERSION 3.22)

project(SquidManager VERSION 1.4.0)

# Submodules must be initialised before building:
#   git submodule update --init --recursive
add_subdirectory(submodules/JUCE builds/JUCE)

juce_add_gui_app(SquidManager
    PRODUCT_NAME "SquidManager"
    COMPANY_NAME "OmOhmProductions"
    ICON_BIG     "Source/GUI/sqedit2_2.png"
    ICON_SMALL   "Source/GUI/sqedit2_2.png")

# The sources include <JuceHeader.h> rather than the individual module headers,
# so the header the Projucer used to generate is still needed.
juce_generate_juce_header(SquidManager)

# Without this, Visual Studio opens with ALL_BUILD selected and F5 fails with
# "Unable to start program ... ALL_BUILD. Access is denied." — ALL_BUILD is a
# utility target with nothing to launch.
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT SquidManager)

set(SOURCE_FILES
    Source/AppProperties.cpp
    Source/AppProperties.h
    Source/Main.cpp
    Source/SystemServices.cpp
    Source/SystemServices.h)

set(GUI_FILES
    Source/GUI/BottomStatusWindow.cpp
    Source/GUI/BottomStatusWindow.h
    Source/GUI/CurrentFolderComponent.cpp
    Source/GUI/CurrentFolderComponent.h
    Source/GUI/GuiProperties.cpp
    Source/GUI/GuiProperties.h
    Source/GUI/MainComponent.cpp
    Source/GUI/MainComponent.h
    Source/GUI/sqedit2_2.png)

set(GUI_SQUIDSALMPLE_FILES
    Source/GUI/SquidSalmple/ChannelEditorComponent.cpp
    Source/GUI/SquidSalmple/ChannelEditorComponent.h
    Source/GUI/SquidSalmple/SquidEditor.cpp
    Source/GUI/SquidSalmple/SquidEditor.h)

set(GUI_BANKLIST_FILES
    Source/GUI/SquidSalmple/BankList/BankListComponent.cpp
    Source/GUI/SquidSalmple/BankList/BankListComponent.h
    Source/GUI/SquidSalmple/BankList/BankListProperties.cpp
    Source/GUI/SquidSalmple/BankList/BankListProperties.h)

set(GUI_CUESETS_FILES
    Source/GUI/SquidSalmple/CueSets/WaveformDisplay.cpp
    Source/GUI/SquidSalmple/CueSets/WaveformDisplay.h)

set(GUI_CVASSIGNS_FILES
    Source/GUI/SquidSalmple/CvAssigns/CvAssignEditor.cpp
    Source/GUI/SquidSalmple/CvAssigns/CvAssignEditor.h
    Source/GUI/SquidSalmple/CvAssigns/CvAssignParameter.cpp
    Source/GUI/SquidSalmple/CvAssigns/CvAssignParameter.h
    Source/GUI/SquidSalmple/CvAssigns/CvAssignSection.cpp
    Source/GUI/SquidSalmple/CvAssigns/CvAssignSection.h)

set(GUI_FILEVIEW_FILES
    Source/GUI/SquidSalmple/FileView/FileViewComponent.cpp
    Source/GUI/SquidSalmple/FileView/FileViewComponent.h)

set(GUI_LOOPPOINTS_FILES
    Source/GUI/SquidSalmple/LoopPoints/LoopPointsView.cpp
    Source/GUI/SquidSalmple/LoopPoints/LoopPointsView.h)

set(SQUIDSALMPLE_FILES
    Source/SquidSalmple/CvParameterProperties.cpp
    Source/SquidSalmple/CvParameterProperties.h
    Source/SquidSalmple/SquidBankProperties.cpp
    Source/SquidSalmple/SquidBankProperties.h
    Source/SquidSalmple/SquidChannelProperties.cpp
    Source/SquidSalmple/SquidChannelProperties.h)

set(SQUIDSALMPLE_AUDIO_FILES
    Source/SquidSalmple/Audio/AudioPlayer.cpp
    Source/SquidSalmple/Audio/AudioPlayer.h
    Source/SquidSalmple/Audio/AudioPlayerProperties.cpp
    Source/SquidSalmple/Audio/AudioPlayerProperties.h
    Source/SquidSalmple/Audio/AudioSettingsProperties.cpp
    Source/SquidSalmple/Audio/AudioSettingsProperties.h)

set(SQUIDSALMPLE_BANK_FILES
    Source/SquidSalmple/Bank/BankHelpers.cpp
    Source/SquidSalmple/Bank/BankHelpers.h
    Source/SquidSalmple/Bank/BankManagerProperties.cpp
    Source/SquidSalmple/Bank/BankManagerProperties.h
    Source/SquidSalmple/Bank/SquidBankLoader.cpp
    Source/SquidSalmple/Bank/SquidBankLoader.h)

set(SQUIDSALMPLE_DATA_FILES
    Source/SquidSalmple/Data/DefaultMetaData.xml
    Source/SquidSalmple/Data/MaxMetaData.xml
    Source/SquidSalmple/Data/MinMetaData.xml)

set(SQUIDSALMPLE_EDITMANAGER_FILES
    Source/SquidSalmple/EditManager/EditManager.cpp
    Source/SquidSalmple/EditManager/EditManager.h
    Source/SquidSalmple/EditManager/EditManagerProperties.cpp
    Source/SquidSalmple/EditManager/EditManagerProperties.h)

set(SQUIDSALMPLE_METADATA_FILES
    Source/SquidSalmple/Metadata/BusyChunkReader.cpp
    Source/SquidSalmple/Metadata/BusyChunkReader.h
    Source/SquidSalmple/Metadata/BusyChunkWriter.cpp
    Source/SquidSalmple/Metadata/BusyChunkWriter.h
    Source/SquidSalmple/Metadata/SquidMetaDataReader.cpp
    Source/SquidSalmple/Metadata/SquidMetaDataReader.h
    Source/SquidSalmple/Metadata/SquidMetaDataWriter.cpp
    Source/SquidSalmple/Metadata/SquidMetaDataWriter.h
    Source/SquidSalmple/Metadata/SquidSalmpleDefs.cpp
    Source/SquidSalmple/Metadata/SquidSalmpleDefs.h)

# libsamplerate.c is a single translation unit that #includes the library's own
# .c files, so it is the only file from that tree that gets compiled.
set(SRC_FILES
    Source/SRC/libsamplerate.c
    Source/SRC/libsamplerate-0.1.9/src/samplerate.h)

set(UTILITY_FILES
    Source/Utility/Crc.h
    Source/Utility/CustomComboBox.cpp
    Source/Utility/CustomComboBox.h
    Source/Utility/CustomComponentMouseHandler.cpp
    Source/Utility/CustomComponentMouseHandler.h
    Source/Utility/CustomTextEditor.h
    Source/Utility/DebugLog.cpp
    Source/Utility/DebugLog.h
    Source/Utility/DebugLogImplementation.h
    Source/Utility/DirectoryDataProperties.cpp
    Source/Utility/DirectoryDataProperties.h
    Source/Utility/DirectoryValueTree.cpp
    Source/Utility/DirectoryValueTree.h
    Source/Utility/DumpStack.cpp
    Source/Utility/DumpStack.h
    Source/Utility/ErrorHelpers.cpp
    Source/Utility/ErrorHelpers.h
    Source/Utility/FileSelectLabel.cpp
    Source/Utility/FileSelectLabel.h
    Source/Utility/LambdaThread.h
    Source/Utility/NoArrowComboBoxLnF.h
    Source/Utility/PersistentRootProperties.cpp
    Source/Utility/PersistentRootProperties.h
    Source/Utility/RootProperties.cpp
    Source/Utility/RootProperties.h
    Source/Utility/RoundedSlideSwitch.cpp
    Source/Utility/RoundedSlideSwitch.h
    Source/Utility/RuntimeRootProperties.cpp
    Source/Utility/RuntimeRootProperties.h
    Source/Utility/SinglePoleFilter.h
    Source/Utility/SplitWindowComponent.cpp
    Source/Utility/SplitWindowComponent.h
    Source/Utility/ValueTreeFile.cpp
    Source/Utility/ValueTreeFile.h
    Source/Utility/ValueTreeHelpers.cpp
    Source/Utility/ValueTreeHelpers.h
    Source/Utility/ValueTreeMonitor.cpp
    Source/Utility/ValueTreeMonitor.h
    Source/Utility/ValueTreeWrapper.cpp
    Source/Utility/ValueTreeWrapper.h
    Source/Utility/WatchDogTimer.h)

# The files the Projucer marked resource="1". They are built into a separate
# binary data target, but stay listed in the groups above so the IDE tree still
# shows them next to the code they belong with.
set(RESOURCE_FILES
    Source/GUI/sqedit2_2.png
    ${SQUIDSALMPLE_DATA_FILES})

set_source_files_properties(${RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

juce_add_binary_data(SquidManagerBinaryData SOURCES ${RESOURCE_FILES})

source_group("Source"                             FILES ${SOURCE_FILES})
source_group("Source/GUI"                         FILES ${GUI_FILES})
source_group("Source/GUI/SquidSalmple"            FILES ${GUI_SQUIDSALMPLE_FILES})
source_group("Source/GUI/SquidSalmple/BankList"   FILES ${GUI_BANKLIST_FILES})
source_group("Source/GUI/SquidSalmple/CueSets"    FILES ${GUI_CUESETS_FILES})
source_group("Source/GUI/SquidSalmple/CvAssigns"  FILES ${GUI_CVASSIGNS_FILES})
source_group("Source/GUI/SquidSalmple/FileView"   FILES ${GUI_FILEVIEW_FILES})
source_group("Source/GUI/SquidSalmple/LoopPoints" FILES ${GUI_LOOPPOINTS_FILES})
source_group("Source/SquidSalmple"                FILES ${SQUIDSALMPLE_FILES})
source_group("Source/SquidSalmple/Audio"          FILES ${SQUIDSALMPLE_AUDIO_FILES})
source_group("Source/SquidSalmple/Bank"           FILES ${SQUIDSALMPLE_BANK_FILES})
source_group("Source/SquidSalmple/Data"           FILES ${SQUIDSALMPLE_DATA_FILES})
source_group("Source/SquidSalmple/EditManager"    FILES ${SQUIDSALMPLE_EDITMANAGER_FILES})
source_group("Source/SquidSalmple/Metadata"       FILES ${SQUIDSALMPLE_METADATA_FILES})
source_group("Source/SRC"                         FILES ${SRC_FILES})
source_group("Source/Utility"                     FILES ${UTILITY_FILES})

target_sources(SquidManager
    PRIVATE
        ${SOURCE_FILES}
        ${GUI_FILES}
        ${GUI_SQUIDSALMPLE_FILES}
        ${GUI_BANKLIST_FILES}
        ${GUI_CUESETS_FILES}
        ${GUI_CVASSIGNS_FILES}
        ${GUI_FILEVIEW_FILES}
        ${GUI_LOOPPOINTS_FILES}
        ${SQUIDSALMPLE_FILES}
        ${SQUIDSALMPLE_AUDIO_FILES}
        ${SQUIDSALMPLE_BANK_FILES}
        ${SQUIDSALMPLE_DATA_FILES}
        ${SQUIDSALMPLE_EDITMANAGER_FILES}
        ${SQUIDSALMPLE_METADATA_FILES}
        ${SRC_FILES}
        ${UTILITY_FILES})

target_compile_definitions(SquidManager
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        DONT_SET_USING_JUCE_NAMESPACE=1)

target_link_libraries(SquidManager
    PRIVATE
        SquidManagerBinaryData
        juce::juce_audio_processors_headless
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
