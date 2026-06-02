#include "TB303.h"
#include <algorithm>
#include <cmath>

namespace DSP
{

TB303Voice::TB303Voice()
{
    // initialize ramps and defaults if needed
    osc.setType(Oscillator::OscType::SawAA);

    pitchRamp.setRampTime(SlideTimeSec);

    cutoffRamp.setTarget(cutoffHz, true);
    outputVolRamp.setTarget(1.f, true);

    vcaEnv.setAttackTime(0.1f);
    vcaEnv.setDecayTime(200.f);
    vcaEnv.setSustainLevel(0.0f);
    vcaEnv.setReleaseTime(200.f);

    vcfEnv.setAttackTime(0.1f);
    vcfEnv.setDecayTime(200.f);
    vcfEnv.setSustainLevel(0.0f);
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
    cutoffHz = std::clamp(Hz, MinFreqHz, MaxFreqHz);
    cutoffRamp.setTarget(Hz, skipRamp);
    filter.setCutoff(Hz);
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
    // 303-like behavior: envelope shape is decay-driven (no sustain plateau).
    vcaEnv.setDecayTime(ms);
    vcfEnv.setDecayTime(ms);

    // Keep release aligned so note-off tails remain consistent.
    vcaEnv.setReleaseTime(ms);
    vcfEnv.setReleaseTime(ms);
}

void TB303Voice::setAccent(float norm, bool skipRamp)
{
    accentAmount = std::clamp(norm, 0.f, 1.f);
}

void TB303Voice::setVolume(float dB, bool skipRamp)
{
    outputVolRamp.setTarget(std::pow(10.f, 0.05f * dB), skipRamp);
}

void TB303Voice::setSweepStrength(float norm, bool /*skipRamp*/)
{
    sweepStrength = std::clamp(norm, 0.f, 1.f);
}

void TB303Voice::setEnvModMapMinNoteHz(float hz)
{
    envModMapMinNoteHz = std::clamp(hz, MinFreqHz, MaxFreqHz);
}

void TB303Voice::setEnvModMapMaxNoteHz(float hz)
{
    envModMapMaxNoteHz = std::clamp(hz, MinFreqHz, MaxFreqHz);
}

void TB303Voice::setEnvModScaleAtMinNote(float scale)
{
    envModScaleAtMinNote = std::clamp(scale, 0.f, 2.f);
}

void TB303Voice::setEnvModScaleAtMaxNote(float scale)
{
    envModScaleAtMaxNote = std::clamp(scale, 0.f, 2.f);
}

bool TB303Voice::canPlaySound(juce::SynthesiserSound* ptr)
{
    return dynamic_cast<SynthSound*>(ptr) != nullptr;
}

void TB303Voice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    // pendingSlide is set by stopNote() when JUCE steals this voice for a new note
    // while the previous key was still held. Consume the flag here.
    const bool doSlide = pendingSlide;
    pendingSlide = false;

    currentNoteFreqHz = midiNoteToHz(midiNoteNumber);
    const float tuned = applyTuning(currentNoteFreqHz);
    pitchRamp.setTarget(tuned, !doSlide);
    if (!doSlide)
        osc.setFrequency(tuned);

    isAccented = (velocity > AccentVelocityThreshold) && (accentAmount > 0.f);

    if (!doSlide)
    {
        // Normal note: full retrigger.
        vcaEnv.start();
        vcfEnv.start();
    }
    else if (isAccented)
    {
        // Legato slide with accent: retrigger VCF env only for the filter snap.
        vcfEnv.start();
    }
    // else: pure legato slide — envelopes continue uninterrupted.

    noteHeld = true;
    voiceStarted = true;
}

void TB303Voice::stopNote(float velocity, bool allowTailOff)
{
    if (allowTailOff)
    {
        // Real key-release: clear held state and begin envelope release tails.
        noteHeld = false;
        vcaEnv.end();
        vcfEnv.end();
    }
    else
    {
        // JUCE is stealing this voice to play a new note immediately.
        // If slide is enabled and the key is still physically held, flag a slide
        // for startNote() and leave the envelopes running.
        if (slideEnabled && noteHeld)
        {
            pendingSlide = true;
            // noteHeld stays true — the key is still down.
        }
        else
        {
            // Hard stop: note was already released or slide is off.
            noteHeld = false;
            vcaEnv.end();
            vcfEnv.end();
        }

        // JUCE requires clearCurrentNote() here so it can assign startNote().
        clearCurrentNote();
    }
}

void TB303Voice::pitchWheelMoved(int)
{
}

void TB303Voice::controllerMoved(int, int)
{
}

float TB303Voice::envModFreqScale(float noteFreqHz) const
{
    const float clampedNoteHz = std::clamp(noteFreqHz, envModMapMinNoteHz, envModMapMaxNoteHz);
    const float norm = (clampedNoteHz - envModMapMinNoteHz) / (envModMapMaxNoteHz - envModMapMinNoteHz);
    return envModScaleAtMinNote + norm * (envModScaleAtMaxNote - envModScaleAtMinNote);
}

float TB303Voice::computeTargetCutoffHz(float vcfEnvOut, float effectiveEnvMod, float accent, float noteFreqHz) const
{
    const float modScale = envModFreqScale(noteFreqHz);
    const float envOctaves = vcfEnvOut * effectiveEnvMod * EnvModMaxOctaves * modScale * sweepStrength;
    const float accentOctaves = accent * AccentCutoffBoostOctaves;

    const float target = cutoffHz * std::pow(2.0f, envOctaves + accentOctaves);
    return std::clamp(target, MinFreqHz, MaxFreqHz);
}


void TB303Voice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    // placeholder: render audio
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

        // VCF
        float vcfEnvOut { 0.f };
        vcfEnv.process(&vcfEnvOut, 1);

        const float accent = isAccented ? accentAmount : 0.f;


        // Accent increases ENV MOD depth and resonance in the control path.
        const float effectiveEnvMod = envModDepth * (1.f + accent * AccentEnvModBoost);
        const float effectiveResonance = std::clamp(resonanceNorm + accent * AccentResonanceBoost, 0.f, 1.f);

        const float targetCutoff = computeTargetCutoffHz(vcfEnvOut, effectiveEnvMod, accent, freqHz);

        cutoffRamp.setTarget(targetCutoff);
        filter.setCutoff(cutoffRamp.getNext());
        filter.setResonance(effectiveResonance);

        const float filterDrive = 1.f + accent * AccentFilterDriveBoost;
        float sample = filter.process(oscOut * filterDrive);

        // Apply VCA
        float vcaEnvOut { 0.f };
        vcaEnv.process(&vcaEnvOut, 1);

        const float accentVcaGain = 1.f + accent * AccentVCABoost;
        sample *= vcaEnvOut * accentVcaGain;
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