#pragma once

#include <juce_dsp/juce_dsp.h>

#include <BaseProcessor.h>
#include "WaveNet.h"
#include "MuLawSampler.h"

namespace Param
{
    namespace ID
    {
        static const juce::String temperature { "temperature" };
    }

    namespace Name
    {
        static const juce::String temperature { "Temperature" };
    }
}

class WaveNetProcessor final : public mrta::BaseProcessor
{
public:
    WaveNetProcessor();
    ~WaveNetProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    static const size_t NUM_EMBEDDINGS { 256 }; // 8-bit quantization
    static const size_t HIDDEN_CHANNELS { 32 };
    static const size_t LAYERS_PER_STACK { 10 };
    static const size_t NUM_STACKS { 3 };
    static const size_t KERNEL_SIZE { 3 };

    WaveNet<NUM_EMBEDDINGS, HIDDEN_CHANNELS, LAYERS_PER_STACK, NUM_STACKS, KERNEL_SIZE> wavenet;
    MuLawSampler sampler;

    size_t current_idx { 128 }; // Start at silence (mid-point of 8-bit range)
    float current_value { 0.0f };
    float logits[NUM_EMBEDDINGS] { 0.0f }; // Buffer for model output logits
    
    size_t downsample_counter { 0 }; // Counter for downsampling input
    static const size_t DOWNSAMPLE_FACTOR { 16u };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveNetProcessor)
};
