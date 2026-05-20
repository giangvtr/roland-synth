#pragma once

#include <BaseProcessor.h>

#include "EnvelopeGenerator.h"

namespace Param
{
    namespace ID
    {
        static const juce::String Trigger { "gate" };
        static const juce::String Analog { "analog" };
        static const juce::String AttackTime { "attack_time" };
        static const juce::String DecayTime { "decay_time" };
        static const juce::String SustainLevel { "sustain_level" };
        static const juce::String ReleaseTime { "release_time" };
    }

    namespace Name
    {
        static const juce::String Trigger { "Gate" };
        static const juce::String Analog { "Analog Style" };
        static const juce::String AttackTime { "Attack Time" };
        static const juce::String DecayTime { "Decay Time" };
        static const juce::String SustainLevel { "Sustain Level" };
        static const juce::String ReleaseTime { "Release Time" };
    }

    namespace Unit
    {
        static const juce::String Ms { "ms" };
    }

    namespace Range
    {
        static constexpr float TimeMin { 1.f };
        static constexpr float TimeMax { 1000.f };
        static constexpr float TimeInc { 0.1f };
        static constexpr float TimeSkw { 0.5f };

        static constexpr float SustainMin { 0.f };
        static constexpr float SustainMax { 1.f };
        static constexpr float SustainInc { 0.01f };
        static constexpr float SustainSkw { 1.f };

        static const juce::String TriggerOn { "ON" };
        static const juce::String TriggerOff { "OFF" };

        static const juce::String AnalogOn { "Analog" };
        static const juce::String AnalogOff { "Digital" };
    }
}

class EnvelopeGeneratorProcessor : public mrta::BaseProcessor
{
public:
    EnvelopeGeneratorProcessor();
    ~EnvelopeGeneratorProcessor() override;

    void prepare(double sampleRate, int samplesPerBlock) override;
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    DSP::EnvelopeGenerator env;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeGeneratorProcessor)
};