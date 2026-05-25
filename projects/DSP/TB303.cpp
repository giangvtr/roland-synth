#include "TB303.h"
#include <algorithm>
#include <cmath>

namespace DSP
{

TB303Voice::TB303Voice()
{
    // initialize ramps and defaults if needed
    outputVolRamp.setTarget(1.0f, true); // default unity gain
    cutoffRamp.setTarget(1000.0f, true); // default cutoff

    vcaEnv.setAttackTime(0.1f);
    vcaEnv.setDecayTime(0.1f);
    vcaEnv.setSustainLevel(1.0f);
    vcaEnv.setReleaseTime(200.f);

    vcfEnv.setAttackTime(0.1f);
    vcfEnv.setDecayTime(0.1f);
    vcfEnv.setSustainLevel(1.0f);
    vcfEnv.setReleaseTime(200.f);
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
    // TB-303 style decay controls release after the key is released.
    // The note is held while the key is pressed, then decays on note-off.
    vcaEnv.setReleaseTime(ms);
    vcfEnv.setReleaseTime(ms);
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
    {
        clearCurrentNote();
        currentNoteFreqHz = 0.0f;
    }
}

void TB303Voice::pitchWheelMoved(int)
{
}

void TB303Voice::controllerMoved(int, int)
{
}


void TB303Voice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    double newSampleRate = getSampleRate();
    if (newSampleRate != sampleRate)
    {
        sampleRate = newSampleRate;
        osc.prepare(sampleRate);
        pitchRamp.prepare(sampleRate);
        outputVolRamp.prepare(sampleRate);
        cutoffRamp.prepare(sampleRate);
        vcaEnv.prepare(sampleRate);
        vcfEnv.prepare(sampleRate);
        filter.prepare(sampleRate);
    }

    auto* leftBuffer = outputBuffer.getWritePointer(0, startSample);

    if (currentNoteFreqHz <= 0.0f)
    {
        for (int n = 0; n < numSamples; ++n)
            leftBuffer[n] = 0.0f;
    }
    else
    {
        for (int n = 0; n < numSamples; ++n)
        {
            const float freq = pitchRamp.getNext();
            osc.setFrequency(freq);
            float out = osc.process();

            float vcfSample = 0.f;
            vcfEnv.process(&vcfSample, 1);

            float cutoff = cutoffRamp.getNext();
            cutoff += envModDepth * vcfSample * MaxEnvModHz;
            if (isAccented)
                cutoff += AccentFilterBoostHz * accentAmount;

            cutoff = std::clamp(cutoff, MinFreqHz, MaxFreqHz);
            filter.setCutoff(cutoff);
            filter.setResonance(resonanceNorm);
            out = filter.process(out);

            float envSample = 0.f;
            vcaEnv.process(&envSample, 1);
            out *= envSample;

            if (isAccented)
                out *= (1.0f + AccentVCABoost * accentAmount);

            out *= outputVolRamp.getNext();
            leftBuffer[n] = out;
        }

        if (vcaEnv.isOff() && vcfEnv.isOff())
        {
            clearCurrentNote();
            currentNoteFreqHz = 0.0f;
        }
    }

    if (outputBuffer.getNumChannels() > 1)
        outputBuffer.copyFrom(1, startSample, outputBuffer, 0, startSample, numSamples);
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