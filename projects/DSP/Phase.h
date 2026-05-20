#pragma once

#include <juce_dsp/juce_dsp.h>

class Phase
{
public:
    Phase(size_t windowSize);
    ~Phase();

    void reset();

    void processFrame(float* outputFrame, const float* inputFrame);

    void setAnalysisHopSize(size_t hopSize) { analysisHopSize = hopSize; }
    void setSynthesisHopSize(size_t hopSize) { synthesisHopSize = hopSize; }

private:
    size_t windowSize;
    juce::dsp::FFT fft;

    std::vector<float> analysisFftBuffer;
    std::vector<float> synthesisFftBuffer;

    std::vector<float> analysisPrevPhaseBuffer;
    std::vector<float> synthesisPrevPhaseBuffer;

    size_t analysisHopSize;
    size_t synthesisHopSize;

    Phase() = delete;
    Phase(const Phase&) = delete;
    Phase(Phase&&) = delete;
    const Phase& operator=(const Phase&) = delete;
    const Phase& operator=(Phase&&) = delete;
};
