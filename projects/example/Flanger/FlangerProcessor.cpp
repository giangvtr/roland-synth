#include "FlangerProcessor.h"
#include "FlangerEditor.h"

#include <algorithm>

static const std::vector<mrta::ParameterInfo> parameters
{
    { Param::ID::Enabled,  Param::Name::Enabled,  Param::Ranges::EnabledOff, Param::Ranges::EnabledOn, true },
    { Param::ID::Offset,   Param::Name::Offset,   Param::Units::Ms,  2.f,  Param::Ranges::OffsetMin,   Param::Ranges::OffsetMax,   Param::Ranges::OffsetInc,   Param::Ranges::OffsetSkw },
    { Param::ID::Depth,    Param::Name::Depth,    Param::Units::Ms,  2.f,  Param::Ranges::DepthMin,    Param::Ranges::DepthMax,    Param::Ranges::DepthInc,    Param::Ranges::DepthSkw },
    { Param::ID::Rate,     Param::Name::Rate,     Param::Units::Hz,  0.5f, Param::Ranges::RateMin,     Param::Ranges::RateMax,     Param::Ranges::RateInc,     Param::Ranges::RateSkw },
    { Param::ID::ModType,  Param::Name::ModType,  Param::Ranges::ModLabels, 0 }
};

FlangerProcessor::FlangerProcessor() :
    mrta::BaseProcessor(parameters),
    flanger(MaxDelaySizeMs, DSP::Flanger::MaxChannels),
    enableRamp(0.05f)
{
    registerParameterCallback(Param::ID::Enabled,
        [this](float newValue, bool force)
        {
            enabled = newValue > 0.5f;
            enableRamp.setTarget(enabled ? 1.f : 0.f, force);
        });

    registerParameterCallback(Param::ID::Offset,
        [this] (float newValue, bool /*force*/)
        {
            flanger.setOffset(newValue);
        });

    registerParameterCallback(Param::ID::Depth,
        [this](float newValue, bool /*force*/)
        {
            flanger.setDepth(newValue);
        });

    registerParameterCallback(Param::ID::Rate,
        [this] (float newValue, bool /*force*/)
        {
            flanger.setModulationRate(newValue);
        });

    registerParameterCallback(Param::ID::ModType,
        [this](float newValue, bool /*force*/)
        {
            DSP::Flanger::ModulationType modType = static_cast<DSP::Flanger::ModulationType>(std::round(newValue));
            flanger.setModulationType(std::min(std::max(modType, DSP::Flanger::Sin), DSP::Flanger::Tri));
        });
}

FlangerProcessor::~FlangerProcessor()
{
}

void FlangerProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    const unsigned int numChannels { static_cast<unsigned int>(std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels())) };

    flanger.clear();
    flanger.prepare(sampleRate, MaxDelaySizeMs, numChannels);
    enableRamp.prepare(sampleRate, true, enabled ? 1.f : 0.f);

    fxBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    fxBuffer.clear();
}

void FlangerProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    const unsigned int numChannels { static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples { static_cast<unsigned int>(buffer.getNumSamples()) };

    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        fxBuffer.copyFrom(ch, 0, buffer, ch, 0, static_cast<int>(numSamples));

    flanger.process(fxBuffer.getArrayOfWritePointers(), fxBuffer.getArrayOfReadPointers(), numChannels, numSamples);
    enableRamp.applyGain(fxBuffer.getArrayOfWritePointers(), numChannels, numSamples);

    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        buffer.addFrom(ch, 0, fxBuffer, ch, 0, static_cast<int>(numSamples));
}

juce::AudioProcessorEditor* FlangerProcessor::createEditor()
{
    return new FlangerEditor(*this);
}

CREATE_PLUGIN(FlangerProcessor)
