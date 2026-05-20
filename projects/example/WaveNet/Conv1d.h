#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#include <vector>


template<size_t IN_CHANNELS, size_t OUT_CHANNELS, size_t KERNEL_SIZE>
class Conv1d
{
public:
    Conv1d(size_t dilation = 1) : dilation(dilation), ring_buffer_length((KERNEL_SIZE - 1) * dilation)
    {
        memset(weights, 0, sizeof(weights));
        memset(bias, 0, sizeof(bias));
        ring_buffer.resize(ring_buffer_length);
        reset_state();
    }

    void reset_state()
    {
        for (size_t i = 0; i < ring_buffer_length; ++i)
        {
            ring_buffer[i].fill(0.0f);
        }
        write_ptr = 0;
    }

    // process one "sample"
    // PyTorch nn.Conv1d is cross-correlation: weight[k] multiplies the input k*dilation to the
    // *left* of weight[K-1]'s position. For streaming inference aligned to the current frame,
    // weight[K-1] hits the current input and weight[0] the oldest sample in the receptive field.
    void forward(float * output, const float * input)
    {
        for (size_t i = 0; i < OUT_CHANNELS; ++i)
        {
            output[i] = bias[i];
            for (size_t j = 0; j < IN_CHANNELS; j++)
            {
                output[i] += weights[KERNEL_SIZE - 1][i][j] * input[j];
                for (size_t k = 1; k < KERNEL_SIZE; ++k)
                {
                    size_t read_ptr = (write_ptr + ring_buffer_length - k * dilation) % ring_buffer_length;
                    output[i] += weights[KERNEL_SIZE - 1 - k][i][j] * ring_buffer[read_ptr][j];
                }
            }
        }
        // update ring buffer
        if (KERNEL_SIZE > 1)
        {
            memcpy(ring_buffer[write_ptr].data(), input, sizeof(float) * IN_CHANNELS);
            write_ptr = (write_ptr + 1) % ring_buffer_length;
        }
    }

    void load_weights(const float * weight_data, const float * bias_data)
    {
        memcpy(weights, weight_data, sizeof(weights));
        memcpy(bias, bias_data, sizeof(bias));
    }

private:
    const size_t dilation;
    const size_t ring_buffer_length;

    std::vector<std::array<float, IN_CHANNELS>> ring_buffer;
    size_t write_ptr { 0 };

    float weights[KERNEL_SIZE][OUT_CHANNELS][IN_CHANNELS] {};
    float bias[OUT_CHANNELS] {};
};
