#pragma once

#include <BaseProcessor.h>

#include "Gru.h"
#include "AmpGruParameters.h"

namespace Param
{
    namespace ID
    {
        static const juce::String Volume { "volume" };
        static const juce::String Tone { "tone" };
    }

    namespace Name
    {
        static const juce::String Volume { "Volume" };
        static const juce::String Tone { "Tone" };
    }
}

class AmpModelProcessor final : public mrta::BaseProcessor
{
public:
    AmpModelProcessor();
    ~AmpModelProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    juce::SmoothedValue<float> volume;
    juce::SmoothedValue<float> tone;

    juce::AudioBuffer<float> nnInputBuffer;
    juce::AudioBuffer<float> nnOutputBuffer;

    static const size_t INPUT_SIZE = 3u;
    static const size_t OUTPUT_SIZE = 1u;
    static const size_t HIDDEN_SIZE = 16u;

    Gru<INPUT_SIZE, OUTPUT_SIZE, HIDDEN_SIZE> gru[2];

    AmpGruParameters gruParameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmpModelProcessor)
};
