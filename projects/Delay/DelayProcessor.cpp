#include "DelayProcessor.h"
#include "DelayEditor.h"

#include <algorithm>

static const std::vector<mrta::ParameterInfo> Parameters
{
    { Param::ID::Enabled,    Param::Name::Enabled,    Param::Ranges::EnabledOff, Param::Ranges::EnabledOn, true },
    { Param::ID::Mix,        Param::Name::Mix,        Param::Units::Ms,  0.5f,   Param::Ranges::MixMin,        Param::Ranges::MixMax,        Param::Ranges::MixInc,        Param::Ranges::MixSkw },
    { Param::ID::Time,       Param::Name::Time,       Param::Units::Ms,  500.f,  Param::Ranges::TimeMin,       Param::Ranges::TimeMax,       Param::Ranges::TimeInc,       Param::Ranges::TimeSkw },
    { Param::ID::Feedback,   Param::Name::Feedback,   "",                0.5f,   Param::Ranges::FeedbackMin,   Param::Ranges::FeedbackMax,   Param::Ranges::FeedbackInc,   Param::Ranges::FeedbackSkw },
    { Param::ID::Wow,        Param::Name::Wow,        "",                0.5f,   Param::Ranges::WowMin,        Param::Ranges::WowMax,        Param::Ranges::WowInc,        Param::Ranges::WowSkw },
    { Param::ID::Tone,       Param::Name::Tone,       Param::Units::Hz,  5000.f, Param::Ranges::ToneMin,       Param::Ranges::ToneMax,       Param::Ranges::ToneInc,       Param::Ranges::ToneSkw },
    { Param::ID::Distortion, Param::Name::Distortion, Param::Units::dB,  3.f,    Param::Ranges::DistortionMin, Param::Ranges::DistortionMax, Param::Ranges::DistortionInc, Param::Ranges::DistortionSkw }
};

DelayProcessor::DelayProcessor() :
    mrta::BaseProcessor(Parameters),
    delay(Param::Ranges::TimeMax, 2),
    wetRamp(0.05f),
    dryRamp(0.05f)
{
    registerParameterCallback(Param::ID::Enabled,
    [this](float newValue, bool force)
    {
        enabled = newValue;
        wetRamp.setTarget(std::clamp(enabled * mix, 0.f, 1.f), force);
        dryRamp.setTarget(std::clamp((1.f - mix) * enabled + (1.f - enabled), 0.f, 1.f), force);
    });

    registerParameterCallback(Param::ID::Mix,
    [this] (float value, bool force)
    {
        mix = value;
        wetRamp.setTarget(std::clamp(enabled * mix, 0.f, 1.f), force);
        dryRamp.setTarget(std::clamp((1.f - mix) * enabled + (1.f - enabled), 0.f, 1.f), force);
    });

    registerParameterCallback(Param::ID::Time,
    [this] (float value, bool /*force*/)
    {
        delay.setDelayTime(value);
    });

    registerParameterCallback(Param::ID::Feedback,
    [this] (float value, bool /*force*/)
    {
        delay.setFeedback(value);
    });

    registerParameterCallback(Param::ID::Wow,
    [this] (float value, bool /*force*/)
    {
        delay.setWow(value);
    });

    registerParameterCallback(Param::ID::Tone,
    [this] (float value, bool /*force*/)
    {
        delay.setToneFrequency(value);
    });

    registerParameterCallback(Param::ID::Distortion,
    [this] (float value, bool /*force*/)
    {
        delay.setDistortion(value);
    });
}

DelayProcessor::~DelayProcessor()
{
}

void DelayProcessor::prepare(double newSampleRate, int samplesPerBlock)
{
    const unsigned int numChannels { static_cast<unsigned int>(std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels())) };

    delay.prepare(newSampleRate, Param::Ranges::TimeMax, numChannels);
    wetRamp.prepare(newSampleRate);
    dryRamp.prepare(newSampleRate);
    meterOutput.prepare(newSampleRate, numChannels);
    meterOutput.setTimeConstant(250.f);
    meterInput.prepare(newSampleRate, numChannels);
    meterInput.setTimeConstant(250.f);

    fxBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    fxBuffer.clear();
}

void DelayProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    const unsigned int numChannels { static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples { static_cast<unsigned int>(buffer.getNumSamples()) };

    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        fxBuffer.copyFrom(ch, 0, buffer, ch, 0, static_cast<int>(numSamples));

    meterInput.process(fxBuffer.getArrayOfReadPointers(), numChannels, numSamples);
    delay.process(fxBuffer.getArrayOfWritePointers(), fxBuffer.getArrayOfReadPointers(), numChannels, numSamples);
    meterOutput.process(fxBuffer.getArrayOfReadPointers(), numChannels, numSamples);

    wetRamp.applyGain(fxBuffer.getArrayOfWritePointers(), numChannels, numSamples);
    dryRamp.applyGain(buffer.getArrayOfWritePointers(), numChannels, numSamples);

    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        buffer.addFrom(ch, 0, fxBuffer, ch, 0, static_cast<int>(numSamples));
}

juce::AudioProcessorEditor* DelayProcessor::createEditor()
{
    return new DelayEditor(*this);
}

CREATE_PLUGIN(DelayProcessor)
