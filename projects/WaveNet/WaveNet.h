#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#include <memory>

#include "Embedding.h"
#include "Conv1d.h"


template<size_t NUM_EMBEDDINGS, size_t HIDDEN_CHANNELS, size_t LAYERS_PER_STACK, size_t NUM_STACKS, size_t KERNEL_SIZE>
class WaveNet
{
public:
    static const size_t NUM_LAYERS { LAYERS_PER_STACK * NUM_STACKS };

    WaveNet()
    {
        for (size_t i = 0; i < NUM_STACKS; ++i)
        {
            for (size_t j = 0; j < LAYERS_PER_STACK; ++j)
            {
                size_t dilation = 1 << j; // 1, 2, 4, 8, ...
                layers[i * LAYERS_PER_STACK + j] = std::make_unique<Conv1d<HIDDEN_CHANNELS, HIDDEN_CHANNELS, KERNEL_SIZE>>(dilation);
            }
        }
    }

    ~WaveNet() = default;

    void reset_state()
    {
        for (size_t i = 0; i < NUM_LAYERS; ++i)
        {
            layers[i]->reset_state();
        }
        conv_out.reset_state();
    }

    // process one "sample"
    void forward(float * output, size_t input)
    {
        embedding_in.forward(embedding_buffer, input);

        // forward through layers
        float * input_ptr = embedding_buffer;
        for (size_t i = 0; i < NUM_LAYERS; ++i)
        {
            float * output_ptr = skips + i * HIDDEN_CHANNELS;
            layers[i]->forward(output_ptr, input_ptr);
            for (size_t j = 0; j < HIDDEN_CHANNELS; ++j)
            {
                output_ptr[j] = std::tanh(output_ptr[j]) + input_ptr[j]; // residual connection
            }
            input_ptr = output_ptr; // for next layer
        }
        conv_out.forward(output, skips);
    }

    void load_weights(const float * parameters)
    {
        size_t offset = 0;
        embedding_in.load_weights(parameters + offset);
        offset += NUM_EMBEDDINGS * HIDDEN_CHANNELS;
        for (size_t i = 0; i < NUM_LAYERS; ++i)
        {
            size_t NUM_KERNEL_WEIGHTS = HIDDEN_CHANNELS * HIDDEN_CHANNELS * KERNEL_SIZE;
            layers[i]->load_weights(parameters + offset, parameters + offset + NUM_KERNEL_WEIGHTS);
            offset += NUM_KERNEL_WEIGHTS + HIDDEN_CHANNELS; // weights + bias
        }
        size_t NUM_OUT_WEIGHTS = HIDDEN_CHANNELS * NUM_LAYERS * NUM_EMBEDDINGS;
        conv_out.load_weights(parameters + offset, parameters + offset + NUM_OUT_WEIGHTS);
    }

private:
    Embedding<NUM_EMBEDDINGS, HIDDEN_CHANNELS> embedding_in;
    std::array<std::unique_ptr<Conv1d<HIDDEN_CHANNELS, HIDDEN_CHANNELS, KERNEL_SIZE>>, NUM_LAYERS> layers {};
    Conv1d<HIDDEN_CHANNELS * NUM_LAYERS, NUM_EMBEDDINGS, 1> conv_out;

    float embedding_buffer[HIDDEN_CHANNELS] {};
    float skips[HIDDEN_CHANNELS * NUM_LAYERS] {};
};
