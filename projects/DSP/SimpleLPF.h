#pragma once

#include <cmath>
#include <algorithm>

namespace DSP
{

// 2-pole biquad lowpass filter (Audio EQ Cookbook design, Direct Form II transposed)
// setCutoff accepts Hz, setResonance accepts the same raw 0–5 parameter as the ladder filter.
class SimpleLPF
{
public:
    SimpleLPF() = default;

    SimpleLPF(const SimpleLPF&) = delete;
    SimpleLPF& operator=(const SimpleLPF&) = delete;
    SimpleLPF(SimpleLPF&&) = delete;
    SimpleLPF& operator=(SimpleLPF&&) = delete;

    void prepare(double sampleRate);
    void setCutoff(float hz);
    // Raw resonance value [0..5] matching the ladder parameter range.
    // Mapped internally: Q = max(0.5, value * 2.0)
    // Q = 0.707 ≈ resonance 0.35 (Butterworth), Q = 10 at resonance 5 (very resonant).
    void setResonance(float value);
    float process(float input);
    void reset();

private:
    void updateCoeffs();

    double sampleRate { 48000.0 };
    float cutoffHz    { 1000.0f };
    float q           { 0.707f };

    // Normalised biquad coefficients (a0 = 1 already factored out)
    float b0 { 1.0f }, b1 { 0.0f }, b2 { 0.0f };
    float a1 { 0.0f }, a2 { 0.0f };

    // Direct Form II transposed delay elements
    float z1 { 0.0f }, z2 { 0.0f };
};

} // namespace DSP
