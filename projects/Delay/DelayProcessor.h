#pragma once

#include <BaseProcessor.h>

#include <Delay.h>
#include <Meter.h>

namespace Param
{
    namespace ID
    {
        static const juce::String Enabled { "enabled" };
        static const juce::String Mix { "mix" };
        static const juce::String Time { "time" };
        static const juce::String Feedback { "feedback" };
        static const juce::String Wow { "wow" };
        static const juce::String Tone { "tone" };
        static const juce::String Distortion { "distortion" };
    }

    namespace Name
    {
        static const juce::String Enabled { "Enabled" };
        static const juce::String Mix { "Mix" };
        static const juce::String Time { "Time" };
        static const juce::String Feedback { "Feedback" };
        static const juce::String Wow { "Wow" };
        static const juce::String Tone { "Tone" };
        static const juce::String Distortion { "Distortion" };
    }

    namespace Ranges
    {
        static constexpr float MixMin { 0.f };
        static constexpr float MixMax { 1.f };
        static constexpr float MixInc { 0.001f };
        static constexpr float MixSkw { 1.0f };

        static constexpr float TimeMin { 1.f };
        static constexpr float TimeMax { 2500.f };
        static constexpr float TimeInc { 0.01f };
        static constexpr float TimeSkw { 0.5f };

        static constexpr float FeedbackMin { 0.f };
        static constexpr float FeedbackMax { 1.f };
        static constexpr float FeedbackInc { 0.001f };
        static constexpr float FeedbackSkw { 1.0f };

        static constexpr float WowMin { 0.f };
        static constexpr float WowMax { 1.f };
        static constexpr float WowInc { 0.001f };
        static constexpr float WowSkw { 1.0f };

        static constexpr float ToneMin { 200.f };
        static constexpr float ToneMax { 20000.f };
        static constexpr float ToneInc { 1.f };
        static constexpr float ToneSkw { 0.4f };

        static constexpr float DistortionMin { 0.f };
        static constexpr float DistortionMax { 36.f };
        static constexpr float DistortionInc { 0.01f };
        static constexpr float DistortionSkw { 0.75f };

        static const juce::String EnabledOff { "Off" };
        static const juce::String EnabledOn { "On" };
    }

    namespace Units
    {
        static const juce::String Ms { "ms" };
        static const juce::String Hz { "Hz" };
        static const juce::String dB { "dB" };
    }
}

class DelayProcessor : public mrta::BaseProcessor
{
public:
    static const unsigned int MaxDelaySizeSamples { 1 << 12 };
    static const unsigned int MaxChannels { 2 };
    static const unsigned int MaxProcessBlockSamples{ 32 };

    DelayProcessor();
    ~DelayProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

    // Output meter getter
    DSP::Meter& getMeterOutput() { return meterOutput; }

    // Input meter getter
    DSP::Meter& getMeterInput() { return meterInput; }

private:
    DSP::Delay delay;
    DSP::Ramp<float> wetRamp;
    DSP::Ramp<float> dryRamp;
    DSP::Meter meterOutput;
    DSP::Meter meterInput;

    float enabled { 1.f };
    float mix { 0.5f };

    juce::AudioBuffer<float> fxBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayProcessor)
};