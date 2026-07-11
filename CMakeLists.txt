cmake_minimum_required(VERSION 3.22)

project(MixMentor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prefer a local JUCE checkout if one is added later.
set(JUCE_SOURCE_DIR "" CACHE PATH "Path to a JUCE source checkout")

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
elseif (JUCE_SOURCE_DIR AND EXISTS "${JUCE_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("${JUCE_SOURCE_DIR}" JUCE)
else()
    find_package(JUCE CONFIG QUIET)
endif()

if (NOT TARGET juce::juce_gui_extra)
    message(FATAL_ERROR
        "JUCE was not found.\n"
        "Add JUCE as a subdirectory at MixMentor/JUCE, configure CMake with -DJUCE_SOURCE_DIR=/path/to/JUCE,\n"
        "or provide a JUCE package config with -DJUCE_DIR=/path/to/JUCEConfig.cmake parent."
    )
endif()

juce_add_gui_app(MixMentor
    PRODUCT_NAME "MixMentor"
    COMPANY_NAME "MixMentor"
)

target_sources(MixMentor
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/Engine/AudioEngine.cpp
        Source/UI/ChannelStripComponent.cpp
)

target_include_directories(MixMentor
    PRIVATE
        Source
)

target_compile_definitions(MixMentor
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(MixMentor
    PRIVATE
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
