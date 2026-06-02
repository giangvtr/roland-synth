// copied from Synth.h

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Oscillator.h"
#include "EnvelopeGenerator.h"
//#include "StateVariableFilter.h"
#include "LadderFilter.h"
#include "Ramp.h"

namespace DSP
{

class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class TB303Voice : public juce::SynthesiserVoice
{
public:
    TB303Voice();
    ~TB303Voice();

    enum LFOType : unsigned int
    {
        SIN = 0,
        TRI
    };

    enum FilterType : unsigned int
    {
        LPF = 0,
        BPF,
        HPF,
    };

    TB303Voice(const TB303Voice&) = delete;
    TB303Voice(TB303Voice&&) = delete;
    const TB303Voice& operator=(const TB303Voice&) = delete;
    const TB303Voice& operator=(TB303Voice&&) = delete;


    // Parameters
    // Waveform switch
    void setWaveform(bool isSaw);

    // Tuning offset in semitones  [-12 .. +12]
    void setTuning(float semitones, bool skipRamp = false);

    // Base cutoff frequency in Hz
    void setFilterCutoff(float Hz, bool skipRamp = false);

    // Resonance [0 .. 1] — mapped internally to ladder Q range
    void setFilterResonance(float norm, bool skipRamp = false);

    // Envelope modulation depth, bipolar [-1 .. +1]
    // Positive: envelope opens filter further. Negative: inverts.
    void setEnvMod(float bipolar, bool skipRamp = false);

    // VCA + VCF shared decay time in ms  [1 .. 1000]
    void setDecay(float ms);

    // Accent amount [0 .. 1]
    // Stored here so startNote can decide the boost magnitude.
    void setAccent(float norm, bool skipRamp = false);

    void setVolume(float dB, bool skipRamp = false);

    // Sweep strength [0 .. 1] — scales the maximum octave range of the filter envelope sweep
    void setSweepStrength(float norm, bool skipRamp = false);

    // Slide on/off + time, slide time typically ~60 ms in the original
    //void setSlide(bool enabled, float timeSec = 0.06f);


    bool canPlaySound(juce::SynthesiserSound* ptr) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;


    //Constants
    static constexpr float MaxFreqHz { 20000.f };
    static constexpr float MinFreqHz { 20.f };

    static constexpr float AccentVelocityThreshold { 0.5f };
 
    // Accent opens the filter by this amount in octaves.
    static constexpr float AccentCutoffBoostOctaves { 0.35f };

    // ENV MOD sweep amount in octaves at depth = 1 and env = 1.
    static constexpr float EnvModMaxOctaves { 4.5f };

    // Tunable linear map for env-mod scaling vs played note frequency.
    // At/under EnvModMapMinNoteHz -> EnvModScaleAtMinNote
    // At/over  EnvModMapMaxNoteHz -> EnvModScaleAtMaxNote
    static constexpr float EnvModMapMinNoteHzDefault { 100.f };
    static constexpr float EnvModMapMaxNoteHzDefault { 880.f };
    static constexpr float EnvModScaleAtMinNoteDefault { 0.25f };
    static constexpr float EnvModScaleAtMaxNoteDefault { 1.65f };

    void setEnvModMapMinNoteHz(float hz);
    void setEnvModMapMaxNoteHz(float hz);
    void setEnvModScaleAtMinNote(float scale);
    void setEnvModScaleAtMaxNote(float scale);

    // Accent boosts ENV MOD depth by this fraction of the current depth
    static constexpr float AccentEnvModBoost { 0.75f };

    // Accent adds this amount to normalized resonance before clamping
    static constexpr float AccentResonanceBoost { 0.2f };
 
    // How much the accent boosts the VCA (linear gain, on top of envelope)
    static constexpr float AccentVCABoost { 0.3f };

    // Accent pushes the filter input for extra bite.
    static constexpr float AccentFilterDriveBoost { 0.35f };

    // 303-style slide time.
    static constexpr float SlideTimeSec { 0.06f };
 
    // Ladder resonance range mapping: norm [0,1] -> Q [MinReso, MaxReso]
    static constexpr float MinReso { 0.5f };
    static constexpr float MaxReso { 4.0f };

private:
    double sampleRate { 0.0 };

    // --- VCO ---
    Oscillator osc;
    bool useSaw { true };
    float tuningOffsetSemitones { 0.f };

    // Pitch ramp handles slide between notes
    Ramp<float> pitchRamp;
    bool slideEnabled { true };
    float currentNoteFreqHz { 0.0f };  // target frequency of last note
 
    // --- VCF ---
    LadderFilter filter;
    float cutoffHz { 1000.f };
    float resonanceNorm { 0.f };
    float envModDepth { 0.f };           // bipolar [-1..1]
    float sweepStrength { 1.f };         // [0..1] scales EnvModMaxOctaves
 
    EnvelopeGenerator vcaEnv;   // easy Envelope gen
    EnvelopeGenerator vcfEnv;   // crazy envelope gen
 
    // --- VCA ---
    Ramp<float> outputVolRamp;
 
    // --- Accent ---
    float accentAmount { 0.f };   // [0..1], set by setAccentAmount()
    bool  isAccented   { false }; // set per-note in startNote()
 
    // --- Helpers ---
    float midiNoteToHz(int midiNote) const;
    float applyTuning(float freqHz) const;
    float envModFreqScale(float noteFreqHz) const;
    float computeTargetCutoffHz(float vcfEnvOut, float effectiveEnvMod, float accent, float noteFreqHz) const;

    float envModMapMinNoteHz   { EnvModMapMinNoteHzDefault };
    float envModMapMaxNoteHz   { EnvModMapMaxNoteHzDefault };
    float envModScaleAtMinNote { EnvModScaleAtMinNoteDefault };
    float envModScaleAtMaxNote { EnvModScaleAtMaxNoteDefault };
 
    // Smooth cutoff ramp (avoids zipper noise when envMod sweeps cutoff)
    Ramp<float> cutoffRamp;

    bool voiceStarted { false };
    bool noteHeld { false };    // true while the key is physically held (cleared on note-off)
    bool pendingSlide { false }; // set in stopNote during voice steal; consumed in startNote
};

}