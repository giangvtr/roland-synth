#include "EnvelopeGeneratorProcessor.h"
#include "EnvelopeGeneratorEditor.h"

static const std::vector<mrta::ParameterInfo> parameters
{
    { Param::ID::Trigger,      Param::Name::Trigger,      Param::Range::TriggerOff, Param::Range::TriggerOn, false },
    { Param::ID::Analog,       Param::Name::Analog,       Param::Range::AnalogOff,  Param::Range::AnalogOn, false },
    { Param::ID::AttackTime,   Param::Name::AttackTime,   Param::Unit::Ms,  50.0f, Param::Range::TimeMin,    Param::Range::TimeMax,    Param::Range::TimeInc,    Param::Range::TimeSkw },
    { Param::ID::DecayTime,    Param::Name::DecayTime,    Param::Unit::Ms,  25.0f, Param::Range::TimeMin,    Param::Range::TimeMax,    Param::Range::TimeInc,    Param::Range::TimeSkw },
    { Param::ID::SustainLevel, Param::Name::SustainLevel, "",                0.7f, Param::Range::SustainMin, Param::Range::SustainMax, Param::Range::SustainInc, Param::Range::SustainSkw },
    { Param::ID::ReleaseTime,  Param::Name::ReleaseTime,  Param::Unit::Ms, 100.0f, Param::Range::TimeMin,    Param::Range::TimeMax,    Param::Range::TimeInc,    Param::Range::TimeSkw },
};

EnvelopeGeneratorProcessor::EnvelopeGeneratorProcessor() :
    mrta::BaseProcessor(parameters)
{
    registerParameterCallback(Param::ID::Trigger,
    [this] (float value, bool /*force*/)
    {
        if (value > 0.5f)
            env.start();
        else
            env.end();
    });

    registerParameterCallback(Param::ID::Analog,
    [this] (float value, bool /*force*/)
    {
        env.setAnalogStyle(value > 0.5f);
    });

    registerParameterCallback(Param::ID::AttackTime,
    [this] (float value, bool /*force*/)
    {
        env.setAttackTime(value);
    });

    registerParameterCallback(Param::ID::DecayTime,
    [this] (float value, bool /*force*/)
    {
        env.setDecayTime(value);
    });

    registerParameterCallback(Param::ID::SustainLevel,
    [this] (float value, bool /*force*/)
    {
        env.setSustainLevel(value);
    });

    registerParameterCallback(Param::ID::ReleaseTime,
    [this] (float value, bool /*force*/)
    {
        env.setReleaseTime(value);
    });
}

EnvelopeGeneratorProcessor::~EnvelopeGeneratorProcessor()
{
}

void EnvelopeGeneratorProcessor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    env.prepare(sampleRate);
}

void EnvelopeGeneratorProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    const unsigned int numChannels{ static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples{ static_cast<unsigned int>(buffer.getNumSamples()) };

    env.process(buffer.getWritePointer(0), numSamples);

    if (numChannels > 1)
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
}

juce::AudioProcessorEditor* EnvelopeGeneratorProcessor::createEditor()
{
    return new EnvelopeGeneratorEditor(*this);
}

CREATE_PLUGIN(EnvelopeGeneratorProcessor)
