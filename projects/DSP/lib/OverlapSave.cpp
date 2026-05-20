#include "OverlapSave.h"
#include <algorithm>

#include <juce_core/juce_core.h>

OverlapSave::OverlapSave(size_t _windowSize) :
    windowSize { _windowSize },
    hopSizeSamples { windowSize / 4 }
{
    jassert(juce::isPowerOfTwo(windowSize));
}

OverlapSave::~OverlapSave()
{
}

void OverlapSave::prepare()
{
    buffer.resize(windowSize << 3);
    std::fill(buffer.begin(), buffer.end(), 0.f);

    writeIndex = 0;
    readIndex = 0;
    bufferMask = buffer.size() - 1;
}

void OverlapSave::writeNewBuffer(const float* const inputBuffer, size_t length)
{
    const auto bufferSize { buffer.size() };

    const auto writeSamples0 { std::min(bufferSize - writeIndex, length) };
    const auto writeSamples1 { length - writeSamples0 };

    std::memcpy(&buffer[writeIndex], &inputBuffer[0], writeSamples0 * sizeof(float));
    std::memcpy(&buffer[0], &inputBuffer[writeSamples0], writeSamples1 * sizeof(float));

    writeIndex += length;
    writeIndex &= bufferMask;
}

bool OverlapSave::readReadyFrame(float* frameBuffer)
{
    const auto bufferSize { buffer.size() };

    // calculate the number of samples available since the last frame was read
    auto numSamplesAvailable { ((writeIndex + bufferSize) - readIndex) & bufferMask };

    bool frameAvailable { false };
    if (numSamplesAvailable >= hopSizeSamples)
    {
        // advance read index to the end of the current frame avaiable
        readIndex += hopSizeSamples;
        readIndex &= bufferMask;

        // calculate the current frame starting index
        auto startFrameIndex { ((readIndex + bufferSize) - windowSize) & bufferMask };

        const auto readSamples0 { std::min(bufferSize - startFrameIndex, windowSize) };
        const auto readSamples1 { windowSize - readSamples0 };

        std::memcpy(&frameBuffer[0], &buffer[startFrameIndex], readSamples0 * sizeof(float));
        std::memcpy(&frameBuffer[readSamples0], &buffer[0], readSamples1 * sizeof(float));

        frameAvailable = true;
    }

    return frameAvailable;
}

void OverlapSave::setHopSizeSamples(size_t newHopSizeSamples)
{
    hopSizeSamples = newHopSizeSamples;
}
