#include "TB303.h"
#include <algorithm>
#include <cmath>

namespace DSP
{

TB303Voice::TB303Voice()
{
    osc.setType(Oscillator::OscType::SawAA);

    const float tuned = applyTuning(currentNoteFreqHz);
    pitchRamp.setTarget(tuned, true);
    osc.setFrequency(tuned);

    cutoffRamp.setTarget(cutoffHz, true);
    outputVolRamp.setTarget(1.f, true);
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
    cutoffHz = std::clamp(Hz, MinFreqHz, MaxFreqHz);
    cutoffRamp.setTarget(cutoffHz, skipRamp);
    filter.setCutoff(cutoffHz);
}

void TB303Voice::setFilterResonance(float norm, bool skipRamp)
{
    resonanceNorm = std::clamp(norm, 0.f, 1.f);
    // map normalized resonance to ladder implementation if needed
    filter.setResonance(resonanceNorm);
}

void TB303Voice::setEnvMod(float bipolar, bool skipRamp)
{
    envModDepth = std::clamp(bipolar, -1.f, 1.f);
}

void TB303Voice::setDecay(float ms)
{
    vcaEnv.setDecayTime(ms);
    vcfEnv.setDecayTime(ms);
}

void TB303Voice::setAccent(float norm, bool skipRamp)
{
    accentAmount = std::clamp(norm, 0.f, 1.f);
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
    pitchRamp.setTarget(tuned, !slideEnabled);
    if (!slideEnabled)
        osc.setFrequency(tuned);

    // accent decision
    isAccented = (velocity > AccentVelocityThreshold) && (accentAmount > 0.f);

    vcaEnv.start();
    vcfEnv.start();

    voiceStarted = true;
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
    // Handle possibly changed sample rate each render call
    const auto newSampleRate { getSampleRate() };
    if (sampleRate != newSampleRate)
    {
        sampleRate = newSampleRate;

        // Call prepare func for all sample rate dependent components
        osc.prepare(sampleRate);
        vcaEnv.prepare(sampleRate);
        vcfEnv.prepare(sampleRate); 

        filter.prepare(sampleRate);

        pitchRamp.prepare(sampleRate);
        cutoffRamp.prepare(sampleRate);
        outputVolRamp.prepare(sampleRate);

    }    
    
    // Render Audio Block
    for (int i = 0; i < numSamples; ++i)
    {
        const float freqHz = pitchRamp.getNext();
        osc.setFrequency(freqHz);
        const float oscOut = osc.process();

        float vcaEnvOut { 0.f };
        vcaEnv.process(&vcaEnvOut, 1);

        float vcfEnvOut { 0.f };
        vcfEnv.process(&vcfEnvOut, 1);
    
        const float accent = isAccented ? accentAmount : 0.f;

        // Accent increases ENV MOD depth and resonance in the control path.
        const float effectiveEnvMod = envModDepth * (1.f + accent * AccentEnvModBoost);
        const float effectiveResonance = std::clamp(resonanceNorm + accent * AccentResonanceBoost, 0.f, 1.f);

        // VCF cutoff is base cutoff plus envelope modulation, then smoothed.
        // TODO: Ramp the EnvModeSweep depending on note frequency to better match the original TB303's behavior 
        const float targetCutoff = std::clamp(
            cutoffHz + (vcfEnvOut * (-1) * effectiveEnvMod * EnvModSweepHz) + (accent * AccentFilterBoostHz),
            MinFreqHz,
            MaxFreqHz);

        
        cutoffRamp.setTarget(targetCutoff);
        filter.setCutoff(cutoffRamp.getNext());
        filter.setResonance(effectiveResonance);

        float sample = filter.process(oscOut);
        sample *= vcaEnvOut * (1.f + accent * AccentVCABoost);
        sample *= outputVolRamp.getNext();

        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            outputBuffer.addSample(ch, startSample + i, sample);

        if (voiceStarted && vcaEnv.isOff() && vcfEnv.isOff())
        {
            voiceStarted = false;
            clearCurrentNote();
        }
    }
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