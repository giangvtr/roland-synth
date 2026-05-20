#pragma once
#include <cstddef>
#include <cstring>


template<size_t NUM_EMBEDDINGS, size_t EMBEDDING_DIM>
class Embedding
{
public:
    Embedding()
    {
        memset(embedding_table, 0, sizeof(embedding_table));
    }

    // process one "sample"
    void forward(float * output, size_t input_index)
    {
        memcpy(output, embedding_table[input_index], sizeof(float) * EMBEDDING_DIM);
    }

    void load_weights(const float * embedding_data)
    {
        memcpy(embedding_table, embedding_data, sizeof(embedding_table));
    }

private:
    float embedding_table[NUM_EMBEDDINGS][EMBEDDING_DIM] {};
};
