// Temporary implementation stub for LadderFilter,

#include "LadderFilter.h"
#include <algorithm>
#include <cmath>

namespace DSP
{

LadderFilter::LadderFilter() = default;
LadderFilter::~LadderFilter() = default;

void LadderFilter::prepare(double newSampleRate)
{
	sampleRate = newSampleRate;
}

void LadderFilter::setCutoff(float hz)
{
	cutoffHz = std::clamp(hz, 20.0f, static_cast<float>(sampleRate * 0.49));
	float omega = 2.0f * static_cast<float>(M_PI) * cutoffHz / static_cast<float>(sampleRate);
	coefficient = 1.0f - std::exp(-omega);
}

void LadderFilter::setResonance(float norm)
{
	resonanceNorm = std::clamp(norm, 0.0f, 1.0f);
}

float LadderFilter::process(float input)
{
	state = coefficient * input + (1.0f - coefficient) * state;
	return state;
}

}
