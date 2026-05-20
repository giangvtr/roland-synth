#include "MidiHandlerProcessor.h"
#include "MidiHandlerEditor.h"


static const std::vector<mrta::ParameterInfo> paramVector
{
    { Param::ID::AttRelTime, Param::Name::AttRelTime, "ms", 100.f, Param::Ranges::AttRelTimeMin, Param::Ranges::AttRelTimeMax, Param::Ranges::AttRelTimeInc, Param::Ranges::AttRelTimeSkw },
    { Param::ID::WaveType, Param::Name::WaveType, Param::Ranges::WaveType, 0 }
};

MidiHandlerProcessor::MidiHandlerProcessor() :
    mrta::BaseProcessor(paramVector)
{
    voice = new DSP::SynthVoice();
    synth.addSound(new DSP::SynthSound());
    synth.addVoice(voice);

    registerParameterCallback(Param::ID::AttRelTime,
        [this] (float value, bool /*force*/)
        {
            voice->setAttRelTime(value);
        });

    registerParameterCallback(Param::ID::WaveType,
        [this] (float value, bool /*force*/)
        {
            DSP::Oscillator::OscType type = static_cast<DSP::Oscillator::OscType>(std::rint(value));
            voice->setWaveType(type);
        });
}

MidiHandlerProcessor::~MidiHandlerProcessor()
{
}

void MidiHandlerProcessor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void MidiHandlerProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* MidiHandlerProcessor::createEditor()
{
    return new MidiHandlerEditor(*this);
}

CREATE_PLUGIN(MidiHandlerProcessor)
