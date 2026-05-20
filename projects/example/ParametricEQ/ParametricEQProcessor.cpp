#include "ParametricEQProcessor.h"
#include "ParametricEQEditor.h"

static std::vector<mrta::ParameterInfo> parameters
{
    { Param::ID::Band0Type, Param::Name::Band0Type, Param::Ranges::Types, DSP::ParametricEqualizer::LowShelf },
    { Param::ID::Band0Freq, Param::Name::Band0Freq, Param::Unit::Freq, 100.f, Param::Ranges::FreqMin, Param::Ranges::FreqMax, Param::Ranges::FreqInc, Param::Ranges::FreqSkw },
    { Param::ID::Band0Reso, Param::Name::Band0Reso, "", 0.71f, Param::Ranges::ResoMin, Param::Ranges::ResoMax, Param::Ranges::ResoInc, Param::Ranges::ResoSkw },
    { Param::ID::Band0Gain, Param::Name::Band0Gain, Param::Unit::Gain, 0.f, Param::Ranges::GainMin, Param::Ranges::GainMax, Param::Ranges::GainInc, Param::Ranges::GainSkw },

    { Param::ID::Band1Type, Param::Name::Band1Type, Param::Ranges::Types, DSP::ParametricEqualizer::Peak },
    { Param::ID::Band1Freq, Param::Name::Band1Freq, Param::Unit::Freq, 1000.f, Param::Ranges::FreqMin, Param::Ranges::FreqMax, Param::Ranges::FreqInc, Param::Ranges::FreqSkw },
    { Param::ID::Band1Reso, Param::Name::Band1Reso, "", 0.71f, Param::Ranges::ResoMin, Param::Ranges::ResoMax, Param::Ranges::ResoInc, Param::Ranges::ResoSkw },
    { Param::ID::Band1Gain, Param::Name::Band1Gain, Param::Unit::Gain, 0.f, Param::Ranges::GainMin, Param::Ranges::GainMax, Param::Ranges::GainInc, Param::Ranges::GainSkw },

    { Param::ID::Band2Type, Param::Name::Band2Type, Param::Ranges::Types, DSP::ParametricEqualizer::HighShelf },
    { Param::ID::Band2Freq, Param::Name::Band2Freq, Param::Unit::Freq, 10000.f, Param::Ranges::FreqMin, Param::Ranges::FreqMax, Param::Ranges::FreqInc, Param::Ranges::FreqSkw },
    { Param::ID::Band2Reso, Param::Name::Band2Reso, "", 0.71f, Param::Ranges::ResoMin, Param::Ranges::ResoMax, Param::Ranges::ResoInc, Param::Ranges::ResoSkw },
    { Param::ID::Band2Gain, Param::Name::Band2Gain, Param::Unit::Gain, 0.f, Param::Ranges::GainMin, Param::Ranges::GainMax, Param::Ranges::GainInc, Param::Ranges::GainSkw },
};

ParametricEQProcessor::ParametricEQProcessor() :
    mrta::BaseProcessor(parameters),
    eq(3)
{
    registerParameterCallback(Param::ID::Band0Type,
    [this] (float val, bool /*force*/)
    {
        eq.setBandType(0, static_cast<DSP::ParametricEqualizer::FilterType>(std::round(val)));
    });

    registerParameterCallback(Param::ID::Band0Freq,
    [this] (float val, bool /*force*/)
    {
        eq.setBandFrequency(0, val);
    });

    registerParameterCallback(Param::ID::Band0Reso,
    [this] (float val, bool /*force*/)
    {
        eq.setBandResonance(0, val);
    });

    registerParameterCallback(Param::ID::Band0Gain,
    [this] (float val, bool /*force*/)
    {
        eq.setBandGain(0, val);
    });

    registerParameterCallback(Param::ID::Band1Type,
    [this] (float val, bool /*force*/)
    {
        eq.setBandType(1, static_cast<DSP::ParametricEqualizer::FilterType>(std::round(val)));
    });

    registerParameterCallback(Param::ID::Band1Freq,
    [this] (float val, bool /*force*/)
    {
        eq.setBandFrequency(1, val);
    });

    registerParameterCallback(Param::ID::Band1Reso,
    [this] (float val, bool /*force*/)
    {
        eq.setBandResonance(1, val);
    });

    registerParameterCallback(Param::ID::Band1Gain,
    [this] (float val, bool /*force*/)
    {
        eq.setBandGain(1, val);
    });

    registerParameterCallback(Param::ID::Band2Type,
    [this] (float val, bool /*force*/)
    {
        eq.setBandType(2, static_cast<DSP::ParametricEqualizer::FilterType>(std::round(val)));
    });

    registerParameterCallback(Param::ID::Band2Freq,
    [this] (float val, bool /*force*/)
    {
        eq.setBandFrequency(2, val);
    });

    registerParameterCallback(Param::ID::Band2Reso,
    [this] (float val, bool /*force*/)
    {
        eq.setBandResonance(2, val);
    });

    registerParameterCallback(Param::ID::Band2Gain,
    [this] (float val, bool /*force*/)
    {
        eq.setBandGain(2, val);
    });
}

ParametricEQProcessor::~ParametricEQProcessor()
{
}

void ParametricEQProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    unsigned int maxNumChannels = std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels());
    eq.prepare(sampleRate, maxNumChannels);
}

void ParametricEQProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    eq.process(buffer.getArrayOfWritePointers(), buffer.getArrayOfReadPointers(), buffer.getNumChannels(), buffer.getNumSamples());
}

juce::AudioProcessorEditor* ParametricEQProcessor::createEditor()
{
    return new ParametricEQEditor(*this);
}

CREATE_PLUGIN(ParametricEQProcessor)
