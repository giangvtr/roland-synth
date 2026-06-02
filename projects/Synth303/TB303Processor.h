#pragma once

#include <BaseProcessor.h>
#include "TB303.h"

namespace Param
{
    namespace ID
    {
        static const juce::String Tuning { "tuning" };
        static const juce::String Cutoff { "cutoff" };
        static const juce::String Resonance { "resonance" };
        static const juce::String EnvelopeMod { "envelope_mod" };
        static const juce::String Decay { "decay" };
        static const juce::String Accent { "accent" };
        static const juce::String Volume { "volume" };
        static const juce::String Waveform { "waveform" };
        static const juce::String FilterType { "filter_type" };
        static const juce::String SweepStrength { "sweep_strength" };
        static const juce::String EnvModMinNoteHz { "envmod_min_note_hz" };
        static const juce::String EnvModMaxNoteHz { "envmod_max_note_hz" };
        static const juce::String EnvModScaleMin { "envmod_scale_min" };
        static const juce::String EnvModScaleMax { "envmod_scale_max" };
    }

    namespace Name
    {
        static const juce::String Tuning { "Tuning" };
        static const juce::String Cutoff { "Cut-off Freq" };
        static const juce::String Resonance { "Resonance" };
        static const juce::String EnvelopeMod { "Envelope Mod." };
        static const juce::String Decay { "Decay" };
        static const juce::String Accent { "Accent" };
        static const juce::String Volume { "Volume" };
        static const juce::String Waveform { "Waveform" };
        static const juce::String FilterType { "Filter Type" };
        static const juce::String SweepStrength { "Sweep Strength" };
        static const juce::String EnvModMinNoteHz { "EnvMod Min Note Hz" };
        static const juce::String EnvModMaxNoteHz { "EnvMod Max Note Hz" };
        static const juce::String EnvModScaleMin { "EnvMod Scale Min" };
        static const juce::String EnvModScaleMax { "EnvMod Scale Max" };
    }

    namespace Ranges
    {
        static constexpr float TuningMin { -12.f };
        static constexpr float TuningMax { 12.f };
        static constexpr float TuningInc { 0.01f };
        static constexpr float TuningSkw { 1.0f };

        static constexpr float FilterFreqMin { 20.0f };
        static constexpr float FilterFreqMax { 5000.f };
        static constexpr float FilterFreqInc { 1.f };
        static constexpr float FilterFreqSkw { 0.25f };

        static constexpr float ResonanceMin { 0.0f };
        static constexpr float ResonanceMax { 5.0f };
        static constexpr float ResonanceInc { 0.01f };
        static constexpr float ResonanceSkw { 0.5f };

        static constexpr float EnvModMin { -1.f };
        static constexpr float EnvModMax { 2.f };
        static constexpr float EnvModInc { 0.01f };
        static constexpr float EnvModSkw { 1.f };

        static constexpr float DecayMin { 1.f };
        static constexpr float DecayMax { 1000.f };
        static constexpr float DecayInc { 1.f };
        static constexpr float DecaySkw { 0.5f };

        static constexpr float AccentMin { 0.f };
        static constexpr float AccentMax { 1.f };
        static constexpr float AccentInc { 0.01f };
        static constexpr float AccentSkw { 1.f };

        static constexpr float VolumeMin { -60.f };
        static constexpr float VolumeMax { 12.f };
        static constexpr float VolumeInc { 0.1f };
        static constexpr float VolumeSkw { 2.8f };

        static const juce::StringArray WaveformType { "Saw", "Square" };
        static const juce::StringArray FilterTypeChoice { "Ladder", "LPF" };

        static constexpr float SweepStrengthMin { 0.f };
        static constexpr float SweepStrengthMax { 1.f };
        static constexpr float SweepStrengthInc { 0.01f };
        static constexpr float SweepStrengthSkw { 1.f };

        static constexpr float EnvModNoteHzMin { 20.f };
        static constexpr float EnvModNoteHzMax { 20000.f };
        static constexpr float EnvModNoteHzInc { 1.f };
        static constexpr float EnvModNoteHzSkw { 0.25f };

        static constexpr float EnvModScaleValMin { 0.f };
        static constexpr float EnvModScaleValMax { 2.f };
        static constexpr float EnvModScaleValInc { 0.01f };
        static constexpr float EnvModScaleValSkw { 1.f };
    }

    namespace Units
    {
        static const juce::String Semitones { "st" };
        static const juce::String Hz { "Hz" };
        static const juce::String Amount { "" };
        static const juce::String Ms { "ms" };
        static const juce::String dB { "dB" };
    }
}

class TB303Processor : public mrta::BaseProcessor
{
public:
    TB303Processor();
    ~TB303Processor() override;

    void prepare(double sampleRate, int samplesPerBlock) override;
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;

private:
    std::vector<DSP::TB303Voice*> voices;
    juce::Synthesiser synth;

    static constexpr size_t NUM_VOICES { 1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TB303Processor)
};