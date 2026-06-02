#include "TB303Processor.h"
#include "TB303Editor.h"
#include <algorithm>

void setTuning(const std::vector<DSP::TB303Voice*>& voices, float semitones, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [semitones, skipRamp](auto* v) { v->setTuning(semitones, skipRamp); });
}

void setCutoff(const std::vector<DSP::TB303Voice*>& voices, float Hz, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [Hz, skipRamp](auto* v) { v->setFilterCutoff(Hz, skipRamp); });
}

void setResonance(const std::vector<DSP::TB303Voice*>& voices, float norm, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [norm, skipRamp](auto* v) { v->setFilterResonance(norm, skipRamp); });
}

void setEnvMod(const std::vector<DSP::TB303Voice*>& voices, float depth, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [depth, skipRamp](auto* v) { v->setEnvMod(depth, skipRamp); });
}

void setDecay(const std::vector<DSP::TB303Voice*>& voices, float ms)
{
    std::for_each(voices.begin(), voices.end(), [ms](auto* v) { v->setDecay(ms); });
}

void setAccent(const std::vector<DSP::TB303Voice*>& voices, float amount, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [amount, skipRamp](auto* v) { v->setAccent(amount, skipRamp); });
}

void setVolume(const std::vector<DSP::TB303Voice*>& voices, float dB, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [dB, skipRamp](auto* v) { v->setVolume(dB, skipRamp); });
}

void setWaveform(const std::vector<DSP::TB303Voice*>& voices, bool useSaw)
{
    std::for_each(voices.begin(), voices.end(), [useSaw](auto* v) { v->setWaveform(useSaw); });
}

void setFilterType(const std::vector<DSP::TB303Voice*>& voices, bool useLadder)
{
    std::for_each(voices.begin(), voices.end(), [useLadder](auto* v) { v->setFilterType(useLadder); });
}

void setSweepStrength(const std::vector<DSP::TB303Voice*>& voices, float norm, bool skipRamp)
{
    std::for_each(voices.begin(), voices.end(), [norm, skipRamp](auto* v) { v->setSweepStrength(norm, skipRamp); });
}

void setEnvModMinNoteHz(const std::vector<DSP::TB303Voice*>& voices, float hz)
{
    std::for_each(voices.begin(), voices.end(), [hz](auto* v) { v->setEnvModMapMinNoteHz(hz); });
}

void setEnvModMaxNoteHz(const std::vector<DSP::TB303Voice*>& voices, float hz)
{
    std::for_each(voices.begin(), voices.end(), [hz](auto* v) { v->setEnvModMapMaxNoteHz(hz); });
}

void setEnvModScaleMin(const std::vector<DSP::TB303Voice*>& voices, float scale)
{
    std::for_each(voices.begin(), voices.end(), [scale](auto* v) { v->setEnvModScaleAtMinNote(scale); });
}

void setEnvModScaleMax(const std::vector<DSP::TB303Voice*>& voices, float scale)
{
    std::for_each(voices.begin(), voices.end(), [scale](auto* v) { v->setEnvModScaleAtMaxNote(scale); });
}

// --- MAIN STABLE PARAMETER BOUNDS LAYOUT ---
// Maps parameter definitions exactly to your project's TB303Processor.h file
static const std::vector<mrta::ParameterInfo> paramVector
{
    { Param::ID::Tuning,      Param::Name::Tuning,      Param::Units::Semitones, 0.0f,   Param::Ranges::TuningMin,     Param::Ranges::TuningMax,     Param::Ranges::TuningInc,     Param::Ranges::TuningSkw },
    { Param::ID::Cutoff,      Param::Name::Cutoff,      Param::Units::Hz,        1000.f, Param::Ranges::FilterFreqMin, Param::Ranges::FilterFreqMax, Param::Ranges::FilterFreqInc, Param::Ranges::FilterFreqSkw },
    { Param::ID::Resonance,   Param::Name::Resonance,   Param::Units::Amount,    0.5f,   Param::Ranges::ResonanceMin,  Param::Ranges::ResonanceMax,  Param::Ranges::ResonanceInc,  Param::Ranges::ResonanceSkw },
    { Param::ID::EnvelopeMod, Param::Name::EnvelopeMod, Param::Units::Amount,    0.5f,   Param::Ranges::EnvModMin,     Param::Ranges::EnvModMax,     Param::Ranges::EnvModInc,     Param::Ranges::EnvModSkw },
    { Param::ID::Decay,       Param::Name::Decay,       Param::Units::Ms,        200.f,  Param::Ranges::DecayMin,      Param::Ranges::DecayMax,      Param::Ranges::DecayInc,      Param::Ranges::DecaySkw },
    { Param::ID::Accent,      Param::Name::Accent,      Param::Units::Amount,    0.0f,   Param::Ranges::AccentMin,     Param::Ranges::AccentMax,     Param::Ranges::AccentInc,     Param::Ranges::AccentSkw },
    { Param::ID::Volume,      Param::Name::Volume,      Param::Units::dB,        0.0f,   Param::Ranges::VolumeMin,     Param::Ranges::VolumeMax,     Param::Ranges::VolumeInc,     Param::Ranges::VolumeSkw },
    { Param::ID::Waveform,       Param::Name::Waveform,       Param::Ranges::WaveformType,     0 },
    { Param::ID::FilterType,     Param::Name::FilterType,     Param::Ranges::FilterTypeChoice, 0 },
    { Param::ID::SweepStrength,  Param::Name::SweepStrength,  Param::Units::Amount, 1.0f, Param::Ranges::SweepStrengthMin, Param::Ranges::SweepStrengthMax, Param::Ranges::SweepStrengthInc, Param::Ranges::SweepStrengthSkw },
    { Param::ID::EnvModMinNoteHz, Param::Name::EnvModMinNoteHz, Param::Units::Hz, 55.f,  Param::Ranges::EnvModNoteHzMin, Param::Ranges::EnvModNoteHzMax, Param::Ranges::EnvModNoteHzInc, Param::Ranges::EnvModNoteHzSkw },
    { Param::ID::EnvModMaxNoteHz, Param::Name::EnvModMaxNoteHz, Param::Units::Hz, 880.f, Param::Ranges::EnvModNoteHzMin, Param::Ranges::EnvModNoteHzMax, Param::Ranges::EnvModNoteHzInc, Param::Ranges::EnvModNoteHzSkw },
    { Param::ID::EnvModScaleMin,  Param::Name::EnvModScaleMin,  Param::Units::Amount, 0.1f,  Param::Ranges::EnvModScaleValMin, Param::Ranges::EnvModScaleValMax, Param::Ranges::EnvModScaleValInc, Param::Ranges::EnvModScaleValSkw },
    { Param::ID::EnvModScaleMax,  Param::Name::EnvModScaleMax,  Param::Units::Amount, 0.85f, Param::Ranges::EnvModScaleValMin, Param::Ranges::EnvModScaleValMax, Param::Ranges::EnvModScaleValInc, Param::Ranges::EnvModScaleValSkw }
};

TB303Processor::TB303Processor() :
    mrta::BaseProcessor(paramVector)
{
    // Configure underlying sound capabilities
    synth.addSound(new DSP::SynthSound());
    
    // Instantiate specific number of synthesis voice units matching NUM_VOICES (1)
    for (size_t i = 0; i < NUM_VOICES; ++i)
    {
        voices.push_back(new DSP::TB303Voice());
        synth.addVoice(voices.back());
    }
    
    // Explicit monophonic note tracking rules
    synth.setNoteStealingEnabled(true);

    // --- REGISTER RUNTIME CALBACK LOOPS ---
    registerParameterCallback(Param::ID::Tuning,      [this] (float value, bool force) { ::setTuning(voices, value, force); });
    registerParameterCallback(Param::ID::Cutoff,      [this] (float value, bool force) { ::setCutoff(voices, value, force); });
    registerParameterCallback(Param::ID::Resonance,   [this] (float value, bool force) { ::setResonance(voices, value, force); });
    registerParameterCallback(Param::ID::EnvelopeMod, [this] (float value, bool force) { ::setEnvMod(voices, value, force); });
    registerParameterCallback(Param::ID::Decay,       [this] (float value, bool force) { ::setDecay(voices, value); });
    registerParameterCallback(Param::ID::Accent,      [this] (float value, bool force) { ::setAccent(voices, value, force); });
    registerParameterCallback(Param::ID::Volume,      [this] (float value, bool force) { ::setVolume(voices, value, force); });
    registerParameterCallback(Param::ID::Waveform,       [this] (float value, bool /*force*/) 
    { 
        // Index 0 = "Saw" (true), Index 1 = "Square" (false)
        bool useSaw = (static_cast<int>(std::round(value)) == 0);
        setWaveform(voices, useSaw); 
    });
    registerParameterCallback(Param::ID::FilterType,     [this] (float value, bool /*force*/)
    {
        // Index 0 = "Ladder" (true), Index 1 = "LPF" (false)
        bool useLadder = (static_cast<int>(std::round(value)) == 0);
        setFilterType(voices, useLadder);
    });
    registerParameterCallback(Param::ID::SweepStrength,  [this] (float value, bool force) { ::setSweepStrength(voices, value, force); });
    registerParameterCallback(Param::ID::EnvModMinNoteHz, [this] (float value, bool /*force*/) { ::setEnvModMinNoteHz(voices, value); });
    registerParameterCallback(Param::ID::EnvModMaxNoteHz, [this] (float value, bool /*force*/) { ::setEnvModMaxNoteHz(voices, value); });
    registerParameterCallback(Param::ID::EnvModScaleMin,  [this] (float value, bool /*force*/) { ::setEnvModScaleMin(voices, value); });
    registerParameterCallback(Param::ID::EnvModScaleMax,  [this] (float value, bool /*force*/) { ::setEnvModScaleMax(voices, value); });
}

TB303Processor::~TB303Processor()
{
}

void TB303Processor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void TB303Processor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* TB303Processor::createEditor()
{
    return new TB303Editor(*this);
}

CREATE_PLUGIN(TB303Processor)