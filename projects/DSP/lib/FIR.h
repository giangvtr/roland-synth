#pragma once

#include <vector>

namespace DSP
{

class FIR
{
public:
    FIR(unsigned int maxImpulseResponseLengthSamples, unsigned int maxNumChannels = 2);
    ~FIR();

    FIR() = delete;
    FIR(const FIR&) = delete;
    FIR(FIR&&) = delete;
    const FIR& operator=(const FIR&) = delete;
    const FIR& operator=(FIR&&) = delete;

    // Will copy the impulse response.
    // If the impulse length is longer than the one passed in the ctor, it will be truncated.
    // If the impulse length is shorter than the one passed in the ctor, it will be zero padded.
    void loadImpulseResponse(const float* newImpulseResponse, unsigned int length);

    // Clear the state
    void clear();

    // Reallocate state storage
    // Calling this method will clear the states
    void reallocateChannels(unsigned int maxNumChannels);

    // Process audio
    // This method can be called with a lower number of channels than allocated
    void process(float* const * output, const float* const * input, unsigned int numChannels, unsigned int numSamples);

private:
    unsigned int maxImpulseResponseLength { 0 };
    unsigned int maxNumChannels { 0 };
    unsigned int stateIndex { 0 };

    std::vector<float> impulseResponse;

    // Each channel has its own state
    std::vector<std::vector<float>> state;
};

}
