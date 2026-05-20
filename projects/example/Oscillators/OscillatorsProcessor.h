#pragma once

#include <BaseProcessor.h>

#include "Oscillator.h"
#include "Ramp.h"

namespace Param
{
    namespace ID
    {
        static const juce::String OscRate { "osc_rate" };
        static const juce::String OscType { "osc_type" };
        static const juce::String Volume { "volume" };
    }

    namespace Name
    {
        static const juce::String OscRate { "Osc. Rate" };
        static const juce::String OscType { "Osc. Type" };
        static const juce::String Volume { "Volume" };
    }

    namespace Unit
    {
        static const juce::String Hz { "Hz" };
        static const juce::String dB { "dB" };
    }

    namespace Range
    {
        static constexpr float OscRateMin { 20.f };
        static constexpr float OscRateMax { 10000.f };
        static constexpr float OscRateInc { 0.1f };
        static constexpr float OscRateSkw { 0.5f };

        static constexpr float VolumeMin { -60.f };
        static constexpr float VolumeMax { 12.f };
        static constexpr float VolumeInc { 0.1f };
        static constexpr float VolumeSkw { 3.8018f };

        static const juce::StringArray OscTypeLabels { "Sine", "Triangle Aliased", "Saw Aliased", "Triangle AA", "Saw AA" };
    }
}

class OscillatorsProcessor : public mrta::BaseProcessor
{
public:
    OscillatorsProcessor();
    ~OscillatorsProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    DSP::Oscillator oscLeft;
    DSP::Oscillator oscRight;

    float volumeDb { 0.f };
    DSP::Ramp<float> volumeRamp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorsProcessor)
};