// Temporary implementation stub for LadderFilter,

#include "LadderFilter.h"

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
	cutoffHz = hz;
}

void LadderFilter::setResonance(float norm)
{
	resonanceNorm = norm;
}

float LadderFilter::process(float input)
{
	return input;
}

}
