cmake_minimum_required(VERSION 3.22)
project(OneDial VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies via CPM ---
include(cmake/CPM.cmake)

# JUCE
CPMAddPackage(
    NAME JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    OPTIONS
        "JUCE_BUILD_EXAMPLES OFF"
        "JUCE_BUILD_EXTRAS OFF"
)

# NeuralAmpModelerCore
CPMAddPackage(
    NAME NeuralAmpModelerCore
    GIT_REPOSITORY https://github.com/sdatkinson/NeuralAmpModelerCore.git
    GIT_TAG main
)

# Eigen (required by NeuralAmpModelerCore) - header-only, skip full configure
CPMAddPackage(
    NAME Eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG master
    DOWNLOAD_ONLY YES
)

# nlohmann_json (required by NeuralAmpModelerCore)
CPMAddPackage(
    NAME nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

# --- Plugin Target ---
juce_add_plugin(OneDial
    COMPANY_NAME "OneDial"
    PLUGIN_MANUFACTURER_CODE ODia
    PLUGIN_CODE ODi1
    FORMATS AU Standalone
    PRODUCT_NAME "OneDial"
    BUNDLE_ID "com.onedial.plugin"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    AU_MAIN_TYPE "kAudioUnitType_Effect"
    DESCRIPTION "Neural Amp Modeler with One Dial"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/onedial_icon.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/onedial_icon.png"
)

# --- Source Files ---
target_sources(OneDial PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/NAMEngine.cpp
    src/ZoneBlender.cpp
    src/IRLoader.cpp
)

# --- Include Directories ---
target_include_directories(OneDial PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# NeuralAmpModelerCore include paths
target_include_directories(OneDial PRIVATE
    ${NeuralAmpModelerCore_SOURCE_DIR}
    ${NeuralAmpModelerCore_SOURCE_DIR}/Dependencies/nlohmann
    ${NeuralAmpModelerCore_SOURCE_DIR}/Dependencies
)

# Eigen include path
target_include_directories(OneDial PRIVATE
    ${Eigen_SOURCE_DIR}
)

# --- Bundle Resources ---
file(GLOB NAM_PROFILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/profiles/*.nam")
file(GLOB IR_CABINETS "${CMAKE_CURRENT_SOURCE_DIR}/resources/cabinets/*.wav")

if(NAM_PROFILES OR IR_CABINETS)
    juce_add_binary_data(OneDial_Resources SOURCES ${NAM_PROFILES} ${IR_CABINETS})
    target_link_libraries(OneDial PRIVATE OneDial_Resources)
endif()

# --- JUCE Compile Definitions ---
target_compile_definitions(OneDial PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    NAM_SAMPLE_FLOAT=1
)

# --- Link Libraries ---
target_link_libraries(OneDial
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Link NeuralAmpModelerCore
if(TARGET nam)
    target_link_libraries(OneDial PRIVATE nam)
elseif(TARGET NeuralAmpModelerCore)
    target_link_libraries(OneDial PRIVATE NeuralAmpModelerCore)
else()
    # Build NAM core sources directly
    file(GLOB_RECURSE NAM_SOURCES "${NeuralAmpModelerCore_SOURCE_DIR}/NAM/*.cpp")
    target_sources(OneDial PRIVATE ${NAM_SOURCES})
endif()

target_link_libraries(OneDial PRIVATE nlohmann_json::nlohmann_json)
