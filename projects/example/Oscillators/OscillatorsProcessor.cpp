#include "OscillatorsProcessor.h"
#include "OscillatorsEditor.h"

static const std::vector<mrta::ParameterInfo> parameters
{
    { Param::ID::OscType, Param::Name::OscType, Param::Range::OscTypeLabels, 0 },
    { Param::ID::OscRate, Param::Name::OscRate, Param::Unit::Hz, 440.f, Param::Range::OscRateMin, Param::Range::OscRateMax, Param::Range::OscRateInc, Param::Range::OscRateSkw },
    { Param::ID::Volume,  Param::Name::Volume,  Param::Unit::dB, -12.f, Param::Range::VolumeMin,  Param::Range::VolumeMax,  Param::Range::VolumeInc,  Param::Range::VolumeSkw },
};

OscillatorsProcessor::OscillatorsProcessor() :
    mrta::BaseProcessor(parameters)
{
    registerParameterCallback(Param::ID::OscRate,
        [this] (float value, bool /*force*/)
        {
            oscLeft.setFrequency(value);
            oscRight.setFrequency(value);
        });

    registerParameterCallback(Param::ID::OscType,
        [this] (float value, bool /*force*/)
        {
            DSP::Oscillator::OscType type = static_cast<DSP::Oscillator::OscType>(std::rint(value));
            oscLeft.setType(type);
            oscRight.setType(type);
        });

    registerParameterCallback(Param::ID::Volume,
        [this] (float value, bool force)
        {
            volumeDb = value;
            float volumeLin = std::pow(10.f, 0.05f * volumeDb);
            volumeRamp.setTarget(volumeLin, force);
        });
}

OscillatorsProcessor::~OscillatorsProcessor()
{
}

void OscillatorsProcessor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    oscLeft.prepare(sampleRate);
    oscRight.prepare(sampleRate);
    volumeRamp.prepare(sampleRate, true, std::pow(10.f, 0.05f * volumeDb));
}

void OscillatorsProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    const unsigned int numChannels{ static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples{ static_cast<unsigned int>(buffer.getNumSamples()) };

    oscLeft.process(buffer.getWritePointer(0), numSamples);
    if (numChannels == 2)
        oscRight.process(buffer.getWritePointer(1), numSamples);

    volumeRamp.applyGain(buffer.getArrayOfWritePointers(), numChannels, numSamples);
}

juce::AudioProcessorEditor* OscillatorsProcessor::createEditor()
{
    return new OscillatorsEditor(*this);
}

CREATE_PLUGIN(OscillatorsProcessor)