#include "TB303.h"
#include <cmath>

namespace DSP
{

TB303Voice::TB303Voice()
{
    // initialize ramps and defaults if needed
}

TB303Voice::~TB303Voice()
{
}

void TB303Voice::setWaveform(bool useSaw)
{
    this->useSaw = useSaw;
    if (useSaw)
        osc.setType(Oscillator::OscType::SawAA);
    else
        osc.setType(Oscillator::OscType::SquareAA);
}

void TB303Voice::setTuning(float semitones, bool skipRamp)
{
    tuningOffsetSemitones = semitones;
    // if there is a current note, update pitch ramp
    if (currentNoteFreqHz > 0.0f)
    {
        const float f = applyTuning(currentNoteFreqHz);
        pitchRamp.setTarget(f, skipRamp);
    }
}

void TB303Voice::setFilterCutoff(float Hz, bool skipRamp)
{
    cutoffHz = Hz;
    cutoffRamp.setTarget(Hz, skipRamp);
    filter.setCutoff(Hz);
}

void TB303Voice::setFilterResonance(float norm, bool skipRamp)
{
    resonanceNorm = norm;
    // map normalized resonance to ladder implementation if needed
    filter.setResonance(norm);
}

void TB303Voice::setEnvMod(float bipolar, bool skipRamp)
{
    envModDepth = bipolar;
}

void TB303Voice::setDecay(float ms)
{
    vcaEnv.setDecayTime(ms);
    vcfEnv.setDecayTime(ms);
}

void TB303Voice::setAccent(float norm, bool skipRamp)
{
    accentAmount = norm;
}

void TB303Voice::setVolume(float dB, bool skipRamp)
{
    outputVolRamp.setTarget(std::pow(10.f, 0.05f * dB), skipRamp);
}

bool TB303Voice::canPlaySound(juce::SynthesiserSound* ptr)
{
    return dynamic_cast<SynthSound*>(ptr) != nullptr;
}

void TB303Voice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    currentNoteFreqHz = midiNoteToHz(midiNoteNumber);
    const float tuned = applyTuning(currentNoteFreqHz);
    pitchRamp.setTarget(tuned);

    // accent decision
    isAccented = (velocity > AccentVelocityThreshold) && (accentAmount > 0.f);

    vcaEnv.start();
    vcfEnv.start();
}

void TB303Voice::stopNote(float velocity, bool allowTailOff)
{
    vcaEnv.end();
    vcfEnv.end();

    if (!allowTailOff)
        clearCurrentNote();
}

void TB303Voice::pitchWheelMoved(int)
{
}

void TB303Voice::controllerMoved(int, int)
{
}


void TB303Voice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    // placeholder: render audio
}

float TB303Voice::midiNoteToHz(int midiNote) const
{
    return 440.f * std::pow(2.f, static_cast<float>(midiNote - 69) / 12.f);
}

float TB303Voice::applyTuning(float freqHz) const
{
    return freqHz * std::pow(2.f, tuningOffsetSemitones / 12.f);
}

} // namespace DSP