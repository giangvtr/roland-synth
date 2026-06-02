#include "SimpleLPF.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace DSP
{

void SimpleLPF::prepare(double newSampleRate)
{
    sampleRate = std::max(1.0, newSampleRate);
    reset();
    updateCoeffs();
}

void SimpleLPF::setCutoff(float hz)
{
    // Keep well below Nyquist so the biquad design remains valid
    cutoffHz = std::clamp(hz, 20.0f, static_cast<float>(sampleRate * 0.499));
    updateCoeffs();
}

void SimpleLPF::setResonance(float value)
{
    // Map raw 0–5 resonance param -> Q: [0.5, 10.5]
    q = std::max(0.5f, value * 2.0f);
    updateCoeffs();
}

float SimpleLPF::process(float input)
{
    const float out = b0 * input + z1;
    z1 = b1 * input - a1 * out + z2;
    z2 = b2 * input - a2 * out;
    return out;
}

void SimpleLPF::reset()
{
    z1 = z2 = 0.0f;
}

void SimpleLPF::updateCoeffs()
{
    // Audio EQ Cookbook: LPF
    // H(s) = 1 / (s/Q + s² + 1)
    const double w0    = 2.0 * M_PI * static_cast<double>(cutoffHz) / sampleRate;
    const double cosW0 = std::cos(w0);
    const double sinW0 = std::sin(w0);
    const double alpha = sinW0 / (2.0 * static_cast<double>(q));

    const double a0inv = 1.0 / (1.0 + alpha);
    b0 = static_cast<float>(((1.0 - cosW0) * 0.5) * a0inv);
    b1 = static_cast<float>( (1.0 - cosW0)         * a0inv);
    b2 = static_cast<float>(((1.0 - cosW0) * 0.5) * a0inv);
    a1 = static_cast<float>((-2.0 * cosW0)          * a0inv);
    a2 = static_cast<float>( (1.0 - alpha)           * a0inv);
}

} // namespace DSP
