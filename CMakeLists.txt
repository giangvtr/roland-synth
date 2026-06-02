cmake_minimum_required(VERSION 3.25)

# Windows static runtime
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# MacOS unversal binary
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# MacOS minimum deployment target
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

# Main project
project(MRTA VERSION 0.1.0 LANGUAGES C CXX)

# Get 'add_plugin' function and add JUCE to project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_plugin)


# General variables
# You should change this to something more interesting
set(company_name "Modern Real-Time Audio")
set(company_copyright "Modern Real-Time Audio Oy")
set(company_code "Mrta")
set(company_bundle_id "com.ModernRealTimeAudio")


## Plugin projects

# DSP directory helper variable
set(dsp_source ${CMAKE_CURRENT_SOURCE_DIR}/projects/DSP)

# GUI directory helper variable
set(gui_source ${CMAKE_CURRENT_SOURCE_DIR}/projects/GUI)

# MRTA framework helper variable
set(mrta_source ${CMAKE_CURRENT_SOURCE_DIR}/mrta_utils)

# Final project
set(synth303_source ${CMAKE_CURRENT_SOURCE_DIR}/projects/Synth303)
add_plugin(synth303
    VERSION 0.1.0
    PLUGIN_NAME "Synth303"
    PROD_NAME Synth303
    PROD_CODE S303
    SYNTH TRUE
    SOURCES
        ${synth303_source}/TB303Editor.cpp
        ${synth303_source}/TB303Processor.cpp
        
        # To modify accordingly
        ${dsp_source}/TB303.cpp
        ${dsp_source}/Oscillator.cpp
        ${dsp_source}/EnvelopeGenerator.cpp
        ${dsp_source}/LadderFilter.cpp
        ${dsp_source}/SimpleLPF.cpp

    INCLUDE_DIRS
        ${synth303_source}
        ${dsp_source}
)
