cmake_minimum_required(VERSION 3.22)

# macOS builds are universal binaries, so the one .app runs natively on both Apple silicon
# and Intel Macs. This has to be set before project(), because that is where CMake inspects
# the compiler and decides which architectures the build - including JUCE and every other
# subdirectory - is configured for. It is ignored by the Windows and Linux generators.
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Architectures to build for on macOS")

# arm64 did not exist before macOS 11, and the slices of a universal binary cannot have
# different deployment targets. Without this CMake would default to the version of macOS
# doing the building, which makes the app refuse to launch on anything older.
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version to build for")

# The line above only supplies a default: it does nothing if the cache already holds a value,
# such as the empty one left in a build folder configured before it was added, or an older
# version passed with -D. Treat 11.0 as a floor rather than a suggestion - a newer target is
# still allowed.
if(CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "" OR CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "11.0")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version to build for" FORCE)
endif()

# CMake gives the Xcode Debug configuration ONLY_ACTIVE_ARCH=YES, which builds just the
# architecture of the machine doing the building - so a Debug build on an Apple silicon Mac
# silently produces an arm64-only app. Forcing it off keeps every configuration universal,
# whether the Xcode destination is set to "My Mac" or "Any Mac".
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")

project(SquidManager VERSION 1.4.2)

# Submodules must be initialised before building:
#   git submodule update --init --recursive
add_subdirectory(submodules/JUCE builds/JUCE)

# oolib is a header/source collection rather than a library: its files include <JuceHeader.h>,
# so they have to be compiled as part of a JUCE target. oolib.cmake defines OOLIB_SOURCES and
# OOLIB_INCLUDE_DIR, and nothing else about the checkout leaks into this file.
include(submodules/oolib/oolib.cmake)

juce_add_gui_app(SquidManager
    PRODUCT_NAME    "SquidManager"
    COMPANY_NAME    "OmOhmProductions"
    COMPANY_WEBSITE "https://cpr2323.github.io/"
    ICON_BIG        "Source/GUI/sqedit2_2.png"
    ICON_SMALL      "Source/GUI/sqedit2_2.png")

# The sources include <JuceHeader.h> rather than the individual module headers,
# so the header the Projucer used to generate is still needed.
juce_generate_juce_header(SquidManager)

# Without this, Visual Studio opens with ALL_BUILD selected and F5 fails with
# "Unable to start program ... ALL_BUILD. Access is denied." — ALL_BUILD is a
# utility target with nothing to launch.
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT SquidManager)

# The Xcode equivalent of the line above, as near as Xcode allows. Xcode has no "startup
# project" setting - the selected scheme is per-user state CMake never writes - but with
# scheme generation left off everywhere else, SquidManager is the only target CMake writes
# a scheme for, so Xcode has nothing else to land on. Turn off "Autocreate schemes" in
# Xcode (Product > Scheme > Manage Schemes) or it will keep inventing one per target,
# SquidManagerBinaryData and ZERO_CHECK included.
set_target_properties(SquidManager PROPERTIES XCODE_GENERATE_SCHEME ON)

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
    Source/GUI/SettingsDialogComponent.cpp
    Source/GUI/SettingsDialogComponent.h
    Source/GUI/Theme/Palette.cpp
    Source/GUI/Theme/Palette.h
    Source/GUI/Theme/SquidColourIds.h
    Source/GUI/Theme/SquidFonts.cpp
    Source/GUI/Theme/SquidFonts.h
    Source/GUI/Theme/UiComponents.h
    Source/GUI/Theme/SquidLookAndFeel.cpp
    Source/GUI/Theme/SquidLookAndFeel.h
    Source/GUI/Theme/ThemeController.cpp
    Source/GUI/Theme/ThemeController.h
    Source/GUI/sqedit2_2.png)

# The app's type. IBM Plex is embedded rather than asked of the system, so the
# app looks the same on every machine it runs on.
set(FONT_FILES
    Assets/Fonts/IBM-Plex-Mono-Regular.ttf
    Assets/Fonts/IBM-Plex-Sans-Regular.ttf
    Assets/Fonts/IBM-Plex-Sans-Condensed-Regular.ttf
    Assets/Fonts/IBM-Plex-Sans-Condensed-SemiBold.ttf)

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

# The files the Projucer marked resource="1". They are built into a separate
# binary data target, but stay listed in the groups above so the IDE tree still
# shows them next to the code they belong with.
set(RESOURCE_FILES
    Source/GUI/sqedit2_2.png
    ${SQUIDSALMPLE_DATA_FILES}
    ${FONT_FILES})

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
source_group("Assets/Fonts"                       FILES ${FONT_FILES})
# oolib keeps its own Category folders, so let the IDE mirror them rather than flattening the list
source_group(TREE ${OOLIB_INCLUDE_DIR}/oolib PREFIX "oolib" FILES ${OOLIB_SOURCES})

target_sources(SquidManager
    PRIVATE
        ${SOURCE_FILES}
        ${GUI_FILES}
        ${FONT_FILES}
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
        ${OOLIB_SOURCES})

target_include_directories(SquidManager
    PRIVATE
        ${OOLIB_INCLUDE_DIR})

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

# The third party licenses have to travel with the app, so they are copied into the
# folder JUCE builds it into - SquidManager_artefacts/<config> - beside the .exe, or
# beside the .app bundle on macOS. That folder can then be distributed as it is.
#
# The copy is a target of its own that SquidManager depends on, rather than a post
# build step: a post build step only runs when the app relinks, so an added or
# changed license would not arrive until some code changed too. Depending on it
# means any build of SquidManager - including building just that project in Visual
# Studio - copies the licenses.
add_custom_target(SquidManagerLicenses
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Licenses"
                                               "$<GENEX_EVAL:$<TARGET_PROPERTY:SquidManager,RUNTIME_OUTPUT_DIRECTORY>>/Licenses"
    COMMENT "Copying Licenses next to SquidManager"
    VERBATIM)
add_dependencies(SquidManager SquidManagerLicenses)
