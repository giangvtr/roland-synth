#pragma once

#include <vector>

class OverlapSave
{
public:
    OverlapSave(size_t windowSize);
    ~OverlapSave();

    // Reset indices, reallocate and clear buffer.
    void prepare();

    // Writes a new buffer of audio.
    void writeNewBuffer(const float* const inputBuffer, size_t length);

    // Tries to read a ready a frame, if there is any available.
    // If there is at least one ready frame, TRUE will be returned and the OLDEST frame it will be copied to 'frameBuffer',
    // which is expected to be at least 'windowSize' samples.
    // If there is no frame ready, FALSE will be returned and nothing will be be written to 'frameBuffer'.
    bool readReadyFrame(float* frameBuffer);

    void setHopSizeSamples(size_t hopSizeSamples);

private:
    std::vector<float> buffer;

    const size_t windowSize;
    size_t hopSizeSamples;
    size_t writeIndex { 0 };
    size_t readIndex { 0 };
    size_t bufferMask;

    OverlapSave() = delete;
    OverlapSave(const OverlapSave&) = delete;
    OverlapSave(OverlapSave&&) = delete;
    const OverlapSave& operator=(const OverlapSave&) = delete;
    const OverlapSave& operator=(OverlapSave&&) = delete;
};
