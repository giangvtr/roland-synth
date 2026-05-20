#include "PluginProcessor.h"
#include "PluginEditor.h"

static const std::vector<mrta::ParameterInfo> ParameterInfos
{
    { Param::ID::Drive, Param::Name::Drive, "", 0.5f, 0.f, 1.f, 0.001f, 1.f }
};

MainProcessor::MainProcessor() :
    mrta::BaseProcessor(ParameterInfos)
{
    registerParameterCallback(Param::ID::Drive,
        [this] (float value, bool /*forced*/)
        {
            DBG(Param::Name::Drive + ": " + juce::String { value });
            outputGain.setTargetValue(value);
        });
}

MainProcessor::~MainProcessor()
{
}

void MainProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    outputGain.reset(sampleRate, 0.01f);
}

void MainProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    outputGain.applyGain(buffer, buffer.getNumSamples());
}

juce::AudioProcessorEditor* MainProcessor::createEditor()
{
    return new MainProcessorEditor(*this);
}

CREATE_PLUGIN(MainProcessor)
