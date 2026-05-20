#pragma once

#include <BaseProcessor.h>

#include <SynthVoice.h>

namespace Param
{

    namespace ID
    {
        static const juce::String AttRelTime { "att_rel_time" };
        static const juce::String WaveType { "wave_type" };
    }

    namespace Name
    {
        static const juce::String AttRelTime { "Att. Rel. Time" };
        static const juce::String WaveType { "Wave Type" };
    }

    namespace Ranges
    {
        static constexpr float AttRelTimeMin { 1.f };
        static constexpr float AttRelTimeMax { 1000.f };
        static constexpr float AttRelTimeInc { 0.1f };
        static constexpr float AttRelTimeSkw { 0.5f };

        static const juce::StringArray WaveType { "Sine", "Tri. Aliased", "Saw Aliased", "Tri. AA", "Saw AA" };
    }
}

class MidiHandlerProcessor : public mrta::BaseProcessor
{
public:
    MidiHandlerProcessor();
    ~MidiHandlerProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    juce::Synthesiser synth;
    DSP::SynthVoice* voice { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiHandlerProcessor)
};