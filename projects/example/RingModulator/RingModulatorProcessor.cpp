#include "RingModulatorProcessor.h"
#include "RingModulatorEditor.h"

static const std::vector<mrta::ParameterInfo> parameters
{
    { Param::ID::ModRate,  Param::Name::ModRate,  Param::Unit::Hz,  1.f,  Param::Range::ModRateMin,  Param::Range::ModRateMax,  Param::Range::ModRateInc,  Param::Range::ModRateSkw },
    { Param::ID::ModType,  Param::Name::ModType,  Param::Range::ModTypeLabels, 0 }
};

RingModulatorProcessor::RingModulatorProcessor() :
    mrta::BaseProcessor(parameters)
{
    registerParameterCallback(Param::ID::ModRate,
    [this] (float value, bool /*force*/)
    {
        ringMod.setModRate(value);
    });

    registerParameterCallback(Param::ID::ModType,
    [this] (float value, bool /*force*/)
    {
        DSP::RingMod::ModType modType = static_cast<DSP::RingMod::ModType>(std::round(value));
        ringMod.setModType(modType);
    });
}

RingModulatorProcessor::~RingModulatorProcessor()
{
}

void RingModulatorProcessor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    ringMod.prepare(sampleRate);
}

void RingModulatorProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    const unsigned int numChannels{ static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples{ static_cast<unsigned int>(buffer.getNumSamples()) };

    ringMod.process(buffer.getArrayOfWritePointers(), buffer.getArrayOfReadPointers(), numChannels, numSamples);
}

juce::AudioProcessorEditor* RingModulatorProcessor::createEditor()
{
    return new RingModulatorEditor(*this);
}

CREATE_PLUGIN(RingModulatorProcessor)
