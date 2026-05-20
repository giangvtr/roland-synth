#pragma once

#include <BaseProcessor.h>

#include <RingMod.h>

namespace Param
{
    namespace ID
    {
        static const juce::String ModRate { "mod_rate" };
        static const juce::String ModType { "mod_type" };
    }

    namespace Name
    {
        static const juce::String ModRate { "Mod. Rate" };
        static const juce::String ModType { "Mod. Type" };
    }

    namespace Unit
    {
        static const juce::String Hz { "Hz" };
    }

    namespace Range
    {
        static constexpr float ModRateMin { 1.f };
        static constexpr float ModRateMax { 1000.f };
        static constexpr float ModRateInc { 0.1f };
        static constexpr float ModRateSkw { 0.35f };

        static constexpr float ModDepthMin { 0.f };
        static constexpr float ModDepthMax { 100.f };
        static constexpr float ModDepthInc { 0.1f };
        static constexpr float ModDepthSkw { 1.f };

        static const juce::StringArray ModTypeLabels { "Sine", "Triangle", "Square" };
    }
}

class RingModulatorProcessor : public mrta::BaseProcessor
{
public:
    RingModulatorProcessor();
    ~RingModulatorProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    DSP::RingMod ringMod;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingModulatorProcessor)
};