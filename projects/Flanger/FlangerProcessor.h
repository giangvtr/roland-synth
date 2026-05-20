#pragma once

#include <BaseProcessor.h>

#include <Flanger.h>
#include <Ramp.h>

namespace Param
{
    namespace ID
    {
        static const juce::String Enabled { "enabled" };
        static const juce::String Offset { "offset" };
        static const juce::String Depth { "depth" };
        static const juce::String Rate { "rate" };
        static const juce::String ModType { "mod_type" };
    }

    namespace Name
    {
        static const juce::String Enabled { "Enabled" };
        static const juce::String Offset { "Offset" };
        static const juce::String Depth { "Depth" };
        static const juce::String Rate { "Rate" };
        static const juce::String ModType { "Mod. Type" };
    }

    namespace Ranges
    {
        static constexpr float OffsetMin { 0.f };
        static constexpr float OffsetMax { 10.f };
        static constexpr float OffsetInc { 0.01f };
        static constexpr float OffsetSkw { 0.5f };

        static constexpr float DepthMin { 0.f };
        static constexpr float DepthMax { 10.f };
        static constexpr float DepthInc { 0.01f };
        static constexpr float DepthSkw { 0.5f };

        static constexpr float RateMin { 0.1f };
        static constexpr float RateMax { 5.f };
        static constexpr float RateInc { 0.01f };
        static constexpr float RateSkw { 0.5f };

        static const juce::StringArray ModLabels { "Sine", "Triangle" };

        static const juce::String EnabledOff { "Off" };
        static const juce::String EnabledOn { "On" };
    }

    namespace Units
    {
        static const juce::String Ms { "ms" };
        static const juce::String Hz { "Hz" };
    }
}

class FlangerProcessor : public mrta::BaseProcessor
{
public:
    static constexpr float MaxDelaySizeMs { 20.f };
    static const unsigned int MaxChannels { 2 };
    static const unsigned int MaxProcessBlockSamples{ 32 };

    FlangerProcessor();
    ~FlangerProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    DSP::Flanger flanger;
    DSP::Ramp<float> enableRamp;

    bool enabled { true };
    juce::AudioBuffer<float> fxBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlangerProcessor)
};