// Temporary stub for LadderFilter.

#pragma once

namespace DSP
{

class LadderFilter
{
public:
	LadderFilter();
	~LadderFilter();

	LadderFilter(const LadderFilter&) = delete;
	LadderFilter& operator=(const LadderFilter&) = delete;
	LadderFilter(LadderFilter&&) = delete;
	LadderFilter& operator=(LadderFilter&&) = delete;

	void prepare(double sampleRate);
	void setCutoff(float hz);
	void setResonance(float norm);
	float process(float input);

private:
	double sampleRate { 48000.0 };
	float cutoffHz { 1000.0f };
	float resonanceNorm { 0.0f };
};

}
