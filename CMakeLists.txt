cmake_minimum_required(VERSION 4.0.1 FATAL_ERROR)
#------------------------------------------------------------------------------#
# Policies; add to this list as required:
#
#  - CMP0177: DESTINATION paths are normalized.
#------------------------------------------------------------------------------#
foreach(policy CMP0177)
    if(POLICY ${policy})
        cmake_policy(SET ${policy} NEW)
    endif()
endforeach()

#------------------------------------------------------------------------------#
# Configuration defaults that we allow to be overridden on the cmake
# command line
#------------------------------------------------------------------------------#
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/libraries)
endif()

#------------------------------------------------------------------------------#
# Project settings for name and version of this build
#------------------------------------------------------------------------------#
project(
    wfview
    VERSION   2.21
    LANGUAGES C CXX
)

option(USB_CONTROLLER "Enable USB controller support" ON)

#------------------------------------------------------------------------------#
# Add our own CMake modules
#------------------------------------------------------------------------------#
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/Modules")

#------------------------------------------------------------------------------#
# CMake Script Includes
#------------------------------------------------------------------------------#
include(CheckTypeSize)
include(CheckSymbolExists)
include(CMakeDependentOption)
include(GenerateExportHeader)
include(GNUInstallDirs)

#------------------------------------------------------------------------------#
# Ensure we have required library dependencies and use the qt standard setup
#------------------------------------------------------------------------------#
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (LINUX)
    find_package(Libudev)
endif()

find_package(Qt6    6.8 REQUIRED COMPONENTS Gui Multimedia Network PrintSupport SerialPort WebSockets Widgets Xml)
find_package(PortAudio REQUIRED)
find_package(Opus REQUIRED)
find_package(hidapi REQUIRED)

qt_standard_project_setup()

#------------------------------------------------------------------------------#
# Create a target that's the same as our project name; that'll be our
# created executable
#------------------------------------------------------------------------------#
set(TARGET ${PROJECT_NAME})

#------------------------------------------------------------------------------#
# MacOS requires a number of settings in the Info.plist and an icon file
#------------------------------------------------------------------------------#
if (APPLE)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources/wfview.icns
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    qt_add_executable(${TARGET} MACOSX_BUNDLE ${CMAKE_CURRENT_SOURCE_DIR}/resources/wfview.icns)

set_target_properties(
    ${TARGET} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST    "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist"
    MACOSX_BUNDLE_ICON_FILE     "${CMAKE_CURRENT_SOURCE_DIR}/resources/wfview.icns"
)
endif()

#------------------------------------------------------------------------------#
# Source and header files common to all platforms
#------------------------------------------------------------------------------#
target_sources (${TARGET} PRIVATE
    include/aboutbox.h
    include/audioconverter.h
    include/audiodevices.h
    include/audiohandler.h
    include/audiohandlerbase.h
    include/audiohandlerpainput.h
    include/audiohandlerpaoutput.h
    include/audiohandlerqtinput.h
    include/audiohandlerqtoutput.h
    include/audiohandlerrtinput.h
    include/audiohandlerrtoutput.h
    include/audiohandlertciinput.h
    include/audiohandlertcioutput.h
    include/audioprocessingwidget.h
    include/audiotaper.h
    include/bandbuttons.h
    include/bytering.h
    include/cachingqueue.h
    include/calibrationwindow.h
    include/cluster.h
    include/colorprefs.h
    include/commhandler.h
    include/controllersetup.h
    include/cwsender.h
    include/cwsidetone.h
    include/database.h
    include/debugwindow.h
    include/firsttimesetup.h
    include/freqctrl.h
    include/freqmemory.h
    include/frequencyinputwidget.h
    include/ft4222handler.h
    include/icomcommander.h
    include/icomserver.h
    include/icomudpaudio.h
    include/icomudpbase.h
    include/icomudpcivdata.h
    include/icomudphandler.h
    include/kenwoodcommander.h
    include/kenwoodserver.h
    include/keyboard.h
    include/logcategories.h
    include/loggingwindow.h
    include/memories.h
    include/meter.h
    include/packettypes.h
    include/prefs.h
    include/printhex.h
    include/pttyhandler.h
    include/qledlabel.h
    include/receiverwidget.h
    include/repeaterattributes.h
    include/repeatersetup.h
    include/rigcommander.h
    include/rigcreator.h
    include/rigctld.h
    include/rigidentities.h
    include/rigserver.h
    include/rigstate.h
    include/rtpaudio.h
    include/satellitesetup.h
    include/scrolltest.h
    include/selectradio.h
    include/settingswidget.h
    include/sidebandchooser.h
    include/spectrumwidget.h
    include/tablewidget.h
    include/tciserver.h
    include/tcpserver.h
    include/txaudioprocessor.h
    include/usbcontroller.h
    include/wfmain.h
    include/wfviewtypes.h
    include/yaesucommander.h
    include/yaesuserver.h
    include/yaesuudpaudio.h
    include/yaesuudpbase.h
    include/yaesuudpcat.h
    include/yaesuudpcontrol.h
    include/yaesuudpscope.h
    qcustomplot/qcustomplot.cpp
    src/aboutbox.cpp src/aboutbox.ui
    src/audio/adpcm/adpcm-dns.c
    src/audio/adpcm/adpcm-lib.c src/audio/adpcm/adpcm-lib.h
    src/audio/audioconverter.cpp
    src/audio/audiodevices.cpp
    src/audio/audiohandlerbase.cpp
    src/audio/audiohandlerpainput.cpp
    src/audio/audiohandlerpaoutput.cpp
    src/audio/audiohandlerqtinput.cpp
    src/audio/audiohandlerqtoutput.cpp
    src/audio/audiohandlerrtinput.cpp
    src/audio/audiohandlerrtoutput.cpp
    src/audio/audiohandlertciinput.cpp
    src/audio/audiohandlertcioutput.cpp
    src/audio/plugins/dyson_compress.cpp
    src/audio/plugins/dysoncompress.h
    src/audio/plugins/mbeq.cpp src/audio/plugins/mbeq.h
    src/audio/plugins/noisegate.cpp src/audio/plugins/noisegate.h
    src/audio/pocketfft/pocketfft.c
    src/audio/resampler/arch.h
    src/audio/resampler/resample.c
    src/audio/resampler/resample_neon.h
    src/audio/resampler/resample_sse.h
    src/audio/resampler/speex_resampler.h
    src/audio/spectrumwidget.cpp
    src/audio/txaudioprocessor.cpp
    src/audioprocessingwidget.cpp
    src/bandbuttons.cpp src/bandbuttons.ui
    src/cachingqueue.cpp
    src/calibrationwindow.cpp src/calibrationwindow.ui
    src/cluster.cpp
    src/commhandler.cpp
    src/controllersetup.cpp src/controllersetup.ui
    src/cwsender.cpp src/cwsender.ui
    src/cwsidetone.cpp
    src/database.cpp
    src/debugwindow.cpp src/debugwindow.ui
    src/firsttimesetup.cpp src/firsttimesetup.ui
    src/freqctrl.cpp
    src/freqmemory.cpp
    src/frequencyinputwidget.cpp src/frequencyinputwidget.ui
    src/ft4222handler.cpp
    src/keyboard.cpp
    src/logcategories.cpp
    src/loggingwindow.cpp src/loggingwindow.ui
    src/main.cpp
    src/memories.cpp src/memories.ui
    src/meter.cpp
    src/pttyhandler.cpp
    src/qledlabel.cpp
    src/radio/icomcommander.cpp
    src/radio/icomserver.cpp
    src/radio/icomudpaudio.cpp
    src/radio/icomudpbase.cpp
    src/radio/icomudpcivdata.cpp
    src/radio/icomudphandler.cpp
    src/radio/kenwoodcommander.cpp
    src/radio/kenwoodserver.cpp
    src/radio/yaesucommander.cpp
    src/radio/yaesuserver.cpp
    src/radio/yaesuudpaudio.cpp
    src/radio/yaesuudpbase.cpp
    src/radio/yaesuudpcat.cpp
    src/radio/yaesuudpcontrol.cpp
    src/radio/yaesuudpscope.cpp
    src/receiverwidget.cpp
    src/repeatersetup.cpp src/repeatersetup.ui
    src/rigcommander.cpp
    src/rigcreator.cpp src/rigcreator.ui
    src/rigctld.cpp
    src/rigidentities.cpp
    src/rigserver.cpp
    src/rtpaudio.cpp
    src/satellitesetup.cpp src/satellitesetup.ui
    src/scrolltest.cpp
    src/selectradio.cpp src/selectradio.ui
    src/settingswidget.cpp src/settingswidget.ui
    src/sidebandchooser.cpp
    src/tablewidget.cpp
    src/tciserver.cpp
    src/tcpserver.cpp
    src/usbcontroller.cpp
    src/wfmain.cpp src/wfmain.ui
)

target_include_directories(wfview PRIVATE
    include
    src
    src/audio
    src/audio/plugins
    src/audio/pocketfft
    src/audio/resampler
    qcustomplot
)

target_compile_definitions(wfview PRIVATE
    BUILD_WFVIEW
    EIGEN_DONT_VECTORIZE
    EIGEN_MPL2_ONLY
    OUTSIDE_SPEEX
    PREFIX=\"\"
    QCUSTOMPLOT_USE_LIBRARY
    QT_DEPRECATED_WARNINGS
    RANDOM_PREFIX=wf
    WFVIEW_VERSION=\"2.21\"
)

if(USB_CONTROLLER)
    target_compile_definitions(wfview PRIVATE USB_CONTROLLER)
endif()

#------------------------------------------------------------------------------#
# Link libraries common to all platforms
#------------------------------------------------------------------------------#
target_link_libraries(wfview PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Network
    Qt6::PrintSupport
    Qt6::SerialPort
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Xml
)

# Resources
set(style_resource_files
    "qdarkstyle/rc/Hmovetoolbar.png"
    "qdarkstyle/rc/Hsepartoolbar.png"
    "qdarkstyle/rc/Vmovetoolbar.png"
    "qdarkstyle/rc/Vsepartoolbar.png"
    "qdarkstyle/rc/branch_closed-on.png"
    "qdarkstyle/rc/branch_closed.png"
    "qdarkstyle/rc/branch_open-on.png"
    "qdarkstyle/rc/branch_open.png"
    "qdarkstyle/rc/checkbox_checked.png"
    "qdarkstyle/rc/checkbox_checked_disabled.png"
    "qdarkstyle/rc/checkbox_checked_focus.png"
    "qdarkstyle/rc/checkbox_indeterminate.png"
    "qdarkstyle/rc/checkbox_indeterminate_focus.png"
    "qdarkstyle/rc/checkbox_unchecked.png"
    "qdarkstyle/rc/checkbox_unchecked_disabled.png"
    "qdarkstyle/rc/checkbox_unchecked_focus.png"
    "qdarkstyle/rc/close-hover.png"
    "qdarkstyle/rc/close-pressed.png"
    "qdarkstyle/rc/close.png"
    "qdarkstyle/rc/down_arrow.png"
    "qdarkstyle/rc/down_arrow_disabled.png"
    "qdarkstyle/rc/left_arrow.png"
    "qdarkstyle/rc/left_arrow_disabled.png"
    "qdarkstyle/rc/radio_checked.png"
    "qdarkstyle/rc/radio_checked_disabled.png"
    "qdarkstyle/rc/radio_checked_focus.png"
    "qdarkstyle/rc/radio_unchecked.png"
    "qdarkstyle/rc/radio_unchecked_disabled.png"
    "qdarkstyle/rc/radio_unchecked_focus.png"
    "qdarkstyle/rc/right_arrow.png"
    "qdarkstyle/rc/right_arrow_disabled.png"
    "qdarkstyle/rc/sizegrip.png"
    "qdarkstyle/rc/stylesheet-branch-end.png"
    "qdarkstyle/rc/stylesheet-branch-more.png"
    "qdarkstyle/rc/stylesheet-vline.png"
    "qdarkstyle/rc/transparent.png"
    "qdarkstyle/rc/undock.png"
    "qdarkstyle/rc/up_arrow.png"
    "qdarkstyle/rc/up_arrow_disabled.png"
)

qt_add_resources(wfview "style"
    PREFIX
        "/qss_icons"
    BASE
        "qdarkstyle"
    FILES
        ${style_resource_files}
)

set(style1_resource_files
    "qdarkstyle/style.qss"
)

qt_add_resources(wfview "style1"
    PREFIX
        "/qdarkstyle"
    BASE
        "qdarkstyle"
    FILES
        ${style1_resource_files}
)

set(resources_resource_files
    "resources/ecoder.png"
    "resources/frequency.ttf"
    "resources/mirabox293.png"
    "resources/mirabox293s.png"
    "resources/miraboxn3.png"
    "resources/quickkeys.png"
    "resources/rc28.png"
    "resources/shuttlepro.png"
    "resources/shuttlexpress.png"
    "resources/streamdeck.png"
    "resources/streamdeckmini.png"
    "resources/streamdeckpedal.png"
    "resources/streamdeckplus.png"
    "resources/streamdeckxl.png"
    "resources/wfview.png"
    "resources/xbox.png"
)

qt_add_resources(wfview "resources"
    PREFIX
        "/resources"
    BASE
        "resources"
    FILES
        ${resources_resource_files}
)

set(translation_resource_files
    "translations/wfview_cs_CZ.qm"
    "translations/wfview_de.qm"
    "translations/wfview_en.qm"
    "translations/wfview_en_GB.qm"
    "translations/wfview_it.qm"
    "translations/wfview_ja.qm"
    "translations/wfview_nl_NL.qm"
    "translations/wfview_pl.qm"
    "translations/wfview_tr.qm"
    "translations/wfview_zh_CN.qm"
    "translations/wfview_zh_TW.qm"
)

qt_add_resources(wfview "translation"
    PREFIX
        "/translations"
    BASE
        "translations"
    FILES
        ${translation_resource_files}
)

if (WIN32)
    target_include_directories(wfview PRIVATE
        libraries
        qcustomplot
    )

    target_compile_definitions(wfview PRIVATE
        GITSHORT=\"-C-parse--shortHEAD\"
        HOST=\"\"
        UNAME=\"%USERNAME%\"
        __WINDOWS_WASAPI__
    )

    target_link_libraries(wfview PRIVATE
        ole32
        user32
    )
endif()

if (LINUX)
    target_compile_definitions(wfview PRIVATE
        __LINUX_PULSE__
    )

    target_link_libraries(wfview PRIVATE
        PkgConfig::Libudev
        pthread
        pulse
        pulse-simple
        rtaudio
    )
endif()

if (APPLE)
    target_compile_definitions(wfview PRIVATE
        __MACOSX_CORE__
    )

    target_link_libraries(wfview PRIVATE
        "-framework CoreAudio"
        "-framework CoreFoundation"
        pthread
    )
endif()

if (NOT LINUX)
    target_sources(wfview PUBLIC
        rtaudio/RTAudio.cpp rtaudio/RTAudio.h
    )

    target_include_directories(wfview PRIVATE
        eigen
        opus/include
        r8brain-free-src
        rtaudio
    )
endif()

if(UNIX)
    find_program(GIT_EXECUTABLE git)
    if(GIT_EXECUTABLE)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
        set(GIT_HASH "nogit")
    endif()

    execute_process(
        COMMAND hostname
        OUTPUT_VARIABLE HOSTNAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND whoami
        OUTPUT_VARIABLE USERNAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    target_compile_definitions(wfview PRIVATE
        GITSHORT="${GIT_HASH}"
        HOST="${HOSTNAME}"
        UNAME="${USERNAME}"
    )
endif()

#------------------------------------------------------------------------------#
# Link PortAudio and opus if available by package target
# otherwise fall back to name with debug/release selection
#------------------------------------------------------------------------------#
if(PortAudio_FOUND)
    if(TARGET PortAudio::PortAudio)
        target_link_libraries(wfview PRIVATE PortAudio::PortAudio)
    elseif(TARGET PortAudio::portaudio)
        target_link_libraries(wfview PRIVATE PortAudio::portaudio)
    else()
        target_link_libraries(wfview PRIVATE portaudio)
    endif()
else()
    target_link_libraries(wfview PRIVATE portaudio)
endif()

if(Opus_FOUND)
    # Common imported target name used by FindOpus modules
    if(TARGET Opus::opus)
        target_link_libraries(wfview PRIVATE Opus::opus)
    else()
        target_link_libraries(wfview PRIVATE opus)
    endif()
else()
    target_link_libraries(wfview PRIVATE opus)
endif()

#------------------------------------------------------------------------------#
# SIMD definitions based on architecture
#------------------------------------------------------------------------------#
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i.86")
    target_compile_definitions(wfview PRIVATE
        EIGEN_VECTORIZE_SSE3
        USE_SSE
        USE_SSE2
    )
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    target_compile_definitions(wfview PRIVATE
        USE_NEON
    )
endif()

#------------------------------------------------------------------------------#
# Link USB_CONTROLLER if available by package target
#------------------------------------------------------------------------------#
if(USB_CONTROLLER)
    if(APPLE)
        if(TARGET hidapi::hidapi)
            target_link_libraries(wfview PRIVATE hidapi::hidapi)
        else()
            target_link_libraries(wfview PRIVATE hidapi)
        endif()
    elseif(LINUX)
        if(TARGET hidapi::hidapi-libusb)
            target_link_libraries(wfview PRIVATE hidapi::hidapi-libusb)
        elseif(TARGET hidapi::hidapi)
            target_link_libraries(wfview PRIVATE hidapi::hidapi)
        else()
            target_link_libraries(wfview PRIVATE hidapi-libusb)
        endif()
    elseif(WIN32)
        # Keep include hint for local headers if needed
        target_include_directories(wfview PRIVATE ../hidapi/hidapi)
        if(TARGET hidapi::hidapi)
            target_link_libraries(wfview PRIVATE hidapi::hidapi)
        else()
            target_link_libraries(wfview PRIVATE hidapi)
        endif()
    endif()
endif()

install(TARGETS wfview
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

#------------------------------------------------------------------------------#
# In order to execute the macdeployqt and windeployqt commands, we need
# to know where they are by finding where the qmake binary is
#------------------------------------------------------------------------------#

get_target_property(QMAKE_EXECUTABLE Qt::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)

#------------------------------------------------------------------------------#
# On MacOS copy the rigs directory to Resources, then run macdeployqt
#------------------------------------------------------------------------------#

if (APPLE)
# copy rigs to Resources directory
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/rigs"
            "$<TARGET_BUNDLE_CONTENT_DIR:${TARGET}>/Resources/rigs"
    COMMENT "Copying rigs into app bundle Resources"
)

# run macdeployqt
  find_program(MACDEPLOYQT macdeployqt HINTS "${QT_BIN_DIR}")
  add_custom_command(
    TARGET ${TARGET}
    POST_BUILD
    COMMAND "${MACDEPLOYQT}"
      "$<TARGET_FILE_DIR:${TARGET}>/../.."
      -always-overwrite
    COMMENT "Running macdeployqt..."
  )
endif (APPLE)
