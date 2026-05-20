#include "FIR.h"

#include <algorithm>

namespace DSP
{

FIR::FIR(unsigned int _maxImpulseResponseLengthSamples, unsigned int _maxNumChannels) :
    maxImpulseResponseLength{ _maxImpulseResponseLengthSamples },
    maxNumChannels { _maxNumChannels },
    impulseResponse(maxImpulseResponseLength, 0.f),
    state(maxNumChannels, std::vector<float>(maxImpulseResponseLength, 0.f))
{
}

FIR::~FIR()
{
}

void FIR::loadImpulseResponse(const float* newImpulseResponse, unsigned int newImpulseResponseLength)
{
    std::fill(impulseResponse.begin(), impulseResponse.end(), 0.f);

    // copy new impulse response in reverse order
    const auto length { std::min(maxImpulseResponseLength, newImpulseResponseLength) };
    for (unsigned int k = 0, i = (length - 1); k < length; ++k, --i)
        impulseResponse[k] = newImpulseResponse[i];
}

void FIR::clear()
{
    // reset state index
    stateIndex = 0;

    // clear state buffers
    for (auto& s : state)
        std::fill(s.begin(), s.end(), 0.f);
}

void FIR::reallocateChannels(unsigned int newMaxNumChannels)
{
    maxNumChannels = newMaxNumChannels;

    // clear and reallocate channel states
    state.clear();
    state.resize(maxNumChannels, std::vector<float>(maxImpulseResponseLength, 0.f));
}

void FIR::process(float* const * output, const float* const * input, unsigned int numChannels, unsigned int numSamples)
{
    // effective num of channels to process
    const auto channels { std::min(maxNumChannels, numChannels) };

    for (unsigned int c = 0; c < channels; ++c)
    {
        // get a reference to the current channel 
        auto& channelState { state[c] };

        // state index to be used for this channel
        auto channelStateIndex { stateIndex };

        for (unsigned int n = 0; n < numSamples; ++n)
        {
            // write new input to state
            channelState[channelStateIndex] = input[c][n];

            // the next index after the input points the oldest sample in the state buffer
            auto auxStateIndex { channelStateIndex + 1 };
            if (auxStateIndex >= maxImpulseResponseLength)
                auxStateIndex -= maxImpulseResponseLength;

            float acc { 0.f };
            for (unsigned int k = 0; k < maxImpulseResponseLength; ++k)
            {
                acc += impulseResponse[k] * channelState[auxStateIndex];

                // increment and wrap
                ++auxStateIndex;
                if (auxStateIndex >= maxImpulseResponseLength)
                    auxStateIndex -= maxImpulseResponseLength;
            }
            output[c][n] = acc;

            ++channelStateIndex;
            if (channelStateIndex >= maxImpulseResponseLength)
                channelStateIndex -= maxImpulseResponseLength;
        }
    }

    stateIndex += numSamples;
    while (stateIndex >= maxImpulseResponseLength)
        stateIndex -= maxImpulseResponseLength;
}

}
