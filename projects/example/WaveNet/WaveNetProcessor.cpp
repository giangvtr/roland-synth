#include "WaveNetProcessor.h"
#include "WaveNetEditor.h"
#include <BinaryData.h>

static const std::vector<mrta::ParameterInfo> ParameterInfos
{
    { Param::ID::temperature, Param::Name::temperature, "", 1.0f, 0.00f, 10.0f, 0.01f, 0.5f }
};

WaveNetProcessor::WaveNetProcessor() :
    mrta::BaseProcessor(ParameterInfos)
{
    registerParameterCallback(Param::ID::temperature,
    [this] (float val, bool /*force*/)
    {
        sampler.set_temperature(val);
    });

    const auto* rawData = BinaryData::wavenet_weights_bin;
    const int rawSize = BinaryData::wavenet_weights_binSize;

    const size_t numParameters = rawSize / sizeof(float);
    std::vector<float> wavenet_parameters(numParameters);
    std::memcpy(wavenet_parameters.data(), rawData, rawSize);
    wavenet.load_weights(wavenet_parameters.data());
}

WaveNetProcessor::~WaveNetProcessor()
{
}

void WaveNetProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    wavenet.reset_state();
    current_idx = 128; // Start at silence
    current_value = 0.0f;
}

void WaveNetProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();
    for (size_t i = 0; i < numSamples; ++i)
    {
        if (downsample_counter == 0)
        {
            wavenet.forward(logits, current_idx);
            sampler.sample(current_idx, current_value, logits);
        }
        for (int ch = 0; ch < numChannels; ++ch)
        {
            buffer.setSample(ch, i, current_value);
        }
        downsample_counter = (downsample_counter + 1) % DOWNSAMPLE_FACTOR;
    }
}

juce::AudioProcessorEditor* WaveNetProcessor::createEditor()
{
    return new WaveNetProcessorEditor(*this);
}

CREATE_PLUGIN(WaveNetProcessor)
