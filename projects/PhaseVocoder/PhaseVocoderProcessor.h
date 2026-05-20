#pragma once

#include <BaseProcessor.h>

#include <juce_dsp/juce_dsp.h>

#include "OverlapSave.h"
#include "OverlapAdd.h"
#include "Phase.h"

class PhaseVocoderProcessor final : public mrta::BaseProcessor
{
public:
    static constexpr size_t WINDOW_SIZE { 1 << 12 };
    static constexpr size_t BASE_HOP_SIZE { WINDOW_SIZE >> 4 };

    PhaseVocoderProcessor();
    ~PhaseVocoderProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    OverlapSave overlapSaveLeft;
    OverlapSave overlapSaveRight;

    OverlapAdd overlapAddLeft;
    OverlapAdd overlapAddRight;

    Phase phaseLeft;
    Phase phaseRight;

    juce::dsp::WindowingFunction<float> windowFunction;

    juce::LagrangeInterpolator resamplerLeft;
    juce::LagrangeInterpolator resamplerRight;

    double ratio { 1.0 };
    size_t analysisHopSize { BASE_HOP_SIZE };
    size_t synthesisHopSize { BASE_HOP_SIZE };

    double resampleFracAcc { 0.0 };

    std::vector<float> resamplerBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseVocoderProcessor)
};
