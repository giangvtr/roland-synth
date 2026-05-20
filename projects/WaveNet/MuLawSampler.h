#pragma once
#include <algorithm>
#include <cstddef>
#include <cmath>
#include <random>


class MuLawSampler
{
public:
    static const size_t NUM_BITS { 8 };
    static const size_t NUM_VALUES { 1 << NUM_BITS };

    void sample(size_t & output_idx, float & output_value, const float * logits)
    {
        // Match infer.py: argmax when temperature ~= 0; else softmax(logits / T) then sample.
        if (temperature < 1e-8f)
        {
            output_idx = 0;
            float maxLogit = logits[0];
            for (size_t i = 1; i < NUM_VALUES; ++i)
            {
                if (logits[i] > maxLogit)
                {
                    maxLogit = logits[i];
                    output_idx = i;
                }
            }
        }
        else
        {
            float maxLogit = logits[0];
            for (size_t i = 1; i < NUM_VALUES; ++i)
            {
                maxLogit = std::max(maxLogit, logits[i]);
            }

            float sum = 0.0f;
            for (size_t i = 0; i < NUM_VALUES; ++i)
            {
                distribution[i] = std::exp((logits[i] - maxLogit) / temperature);
                sum += distribution[i];
            }

            for (size_t i = 0; i < NUM_VALUES; ++i)
            {
                distribution[i] /= sum;
            }

            float r = uniform_dist(rng);
            float cumulative = 0.0f;

            output_idx = 128;
            for (size_t i = 0; i < NUM_VALUES; ++i)
            {
                cumulative += distribution[i];
                if (r <= cumulative)
                {
                    output_idx = i;
                    break;
                }
            }
        }
        // Mu-Law decoding
        static const float MU = 255.0f;
        float y = (static_cast<float>(output_idx) / (NUM_VALUES - 1)) * 2.0f - 1.0f; // Map to [-1, 1]
        float sign = (y >= 0) ? 1.0f : -1.0f;
        float x = sign * (std::pow(1.0f + MU, std::abs(y)) - 1.0f) / MU;
        output_value = x;
    }

    void set_temperature(float new_temperature)
    {
        temperature = new_temperature;
    }

private:
    float temperature { 1.0f };
    float distribution[NUM_VALUES];

    // Random number generation
    std::mt19937 rng { std::random_device{}() };
    std::uniform_real_distribution<float> uniform_dist { 0.0f, 1.0f };
};
