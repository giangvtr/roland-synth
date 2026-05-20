#pragma once

#include <juce_dsp/juce_dsp.h>

#include <BaseProcessor.h>

namespace Param
{
    namespace ID
    {
        static const juce::String Drive { "drive" };
    }

    namespace Name
    {
        static const juce::String Drive { "Drive" };
    }
}

class MainProcessor final : public mrta::BaseProcessor
{
public:
    MainProcessor();
    ~MainProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    juce::SmoothedValue<float> outputGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainProcessor)
};
