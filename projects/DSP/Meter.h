#pragma once

#include <atomic>
#include <array>

namespace DSP
{

class Meter
{
public:
    Meter();
    ~Meter();

    static constexpr unsigned int MaxNumChannels { 2 };
    using atomic_array = std::atomic<std::array<float, MaxNumChannels>>;

    // prevent compiling unsupported CPUs
    static_assert(atomic_array::is_always_lock_free, "Not supported!");

    Meter(const Meter&) = delete;
    Meter(Meter&&) = delete;
    const Meter& operator=(const Meter&) = delete;
    const Meter& operator=(Meter&&) = delete;

    void prepare(double sampleRate, unsigned int numChannels);

    // Buffer flavour
    void process(const float* const* input, unsigned int numChannels, unsigned int numSamples);

    // Single-sample flavour
    void process(const float* const input, unsigned int numChannels);

    void setTimeConstant(float releaseTimeMs);

    // Will be called from the GUI to get the current envelope value
    float getEnvelope(unsigned int channel) const;

    unsigned int getNumChannels() const;

private:
    double sampleRate { 48000.0 };

    unsigned int numChannels { 0 };

    float releaseTimeMs { 250.f };

    atomic_array envelopeState;
    float envelopeCoeff { 1.f };
};

}
