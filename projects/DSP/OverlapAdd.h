#pragma once

#include <vector>

class OverlapAdd
{
public:
    OverlapAdd(size_t windowSize);
    ~OverlapAdd();

    // Reset indices, reallocate and clear buffer.
    void prepare();

    // Writes a new buffer of audio.
    void writeNewFrame(const float* frameBuffer);

    // Tries to read 'length' samples.
    // Return the nnumber of samples actually read, which will always be less than 'length'.
    size_t readyReadBuffer(float* outputBuffer, size_t length);

    void setHopSizeSamples(size_t hopSizeSamples);

private:
    std::vector<float> buffer;

    const size_t windowSize;
    size_t hopSizeSamples;
    size_t writeIndex { 0 };
    size_t readIndex { 0 };
    size_t bufferMask;

    OverlapAdd() = delete;
    OverlapAdd(const OverlapAdd&) = delete;
    OverlapAdd(OverlapAdd&&) = delete;
    const OverlapAdd& operator=(const OverlapAdd&) = delete;
    const OverlapAdd& operator=(OverlapAdd&&) = delete;
};
