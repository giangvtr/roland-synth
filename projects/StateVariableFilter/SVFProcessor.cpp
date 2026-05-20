#include "SVFProcessor.h"
#include "SVFEditor.h"

#include <algorithm>

void modeMix(float mode, float& lpf, float& bpf, float& hpf)
{
    mode = std::clamp(mode, -1.f, 1.f);
    lpf = std::fmax(-mode, 0.f);
    bpf = std::fmax(1.f - std::fabs(mode), 0.f);
    hpf = std::fmax(mode, 0.f);
}

static const std::vector<mrta::ParameterInfo> parameters
{
    { Param::ID::Freq,        Param::Name::Freq,        Param::Unit::Hz, 500.0f,  Param::Range::FreqMin,        Param::Range::FreqMax,        Param::Range::FreqInc,        Param::Range::FreqSkw },
    { Param::ID::FreqModAmt,  Param::Name::FreqModAmt,  "",                0.25f, Param::Range::FreqModAmtMin,  Param::Range::FreqModAmtMax,  Param::Range::FreqModAmtInc,  Param::Range::FreqModAmtSkw },
    { Param::ID::FreqModRate, Param::Name::FreqModRate, Param::Unit::Hz,   0.5f,  Param::Range::FreqModRateMin, Param::Range::FreqModRateMax, Param::Range::FreqModRateInc, Param::Range::FreqModRateSkw },
    { Param::ID::Reso,        Param::Name::Reso,        "",                1.0f,  Param::Range::ResoMin,        Param::Range::ResoMax,        Param::Range::ResoInc,        Param::Range::ResoSkw },
    { Param::ID::Mode,        Param::Name::Mode,        "",                0.0f,  Param::Range::ModeMin,        Param::Range::ModeMax,        Param::Range::ModeInc,        Param::Range::ModeSkw }
};

SVFProcessor::SVFProcessor() :
    mrta::BaseProcessor(parameters),
    freqRamp(0.005f),
    freqModAmtRamp(0.005f)
{
    registerParameterCallback(Param::ID::Freq,
    [this] (float value, bool force)
    {
        freqHz = value;
        freqRamp.setTarget(value, force);
    });

    registerParameterCallback(Param::ID::FreqModAmt,
    [this] (float value, bool force)
    {
        freqModAmt = std::clamp(value, 0.f, 1.f);
        freqModAmtRamp.setTarget(freqModAmt, force);
    });

    registerParameterCallback(Param::ID::FreqModRate,
    [this] (float value, bool force)
    {
        lfo.setFrequency(value);
    });

    registerParameterCallback(Param::ID::Reso,
    [this] (float value, bool force)
    {
        reso = value;
        resoRamp.setTarget(value, force);
    });

    registerParameterCallback(Param::ID::Mode,
    [this] (float value, bool force)
    {
        mode = value;
        float lpf(0.f), bpf(0.f), hpf(0.f);
        modeMix(mode, lpf, bpf, hpf);
        lpfRamp.setTarget(lpf, force);
        bpfRamp.setTarget(bpf, force);
        hpfRamp.setTarget(hpf, force);
    });

    lfo.setType(DSP::Oscillator::Sin);
}

SVFProcessor::~SVFProcessor()
{
}

void SVFProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    svfLeft.prepare(sampleRate);
    svfRight.prepare(sampleRate);
    lfo.prepare(sampleRate);

    freqRamp.prepare(sampleRate, true, freqHz);
    freqModAmtRamp.prepare(sampleRate, true, freqModAmt);
    resoRamp.prepare(sampleRate, true, reso);

    float lpf, bpf, hpf;
    modeMix(mode, lpf, bpf, hpf);
    lpfRamp.prepare(sampleRate, true, lpf);
    bpfRamp.prepare(sampleRate, true, bpf);
    hpfRamp.prepare(sampleRate, true, hpf);

    // resize the aux buffers
    freqInBuffer.setSize(1, samplesPerBlock);
    freqModAmtBuffer.setSize(1, samplesPerBlock);
    lfoBuffer.setSize(1, samplesPerBlock);
    resoInBuffer.setSize(1, samplesPerBlock);
    lpfOutBuffer.setSize(2, samplesPerBlock);
    bpfOutBuffer.setSize(2, samplesPerBlock);
    hpfOutBuffer.setSize(2, samplesPerBlock);
}

void SVFProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    const unsigned int numChannels{ static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples{ static_cast<unsigned int>(buffer.getNumSamples()) };

    // clear all aux buffers
    freqInBuffer.clear();
    freqModAmtBuffer.clear();
    lfoBuffer.clear();
    resoInBuffer.clear();
    lpfOutBuffer.clear();
    bpfOutBuffer.clear();
    hpfOutBuffer.clear();

    // get the freq and reso controls
    freqRamp.applySum(freqInBuffer.getWritePointer(0), numSamples);
    resoRamp.applySum(resoInBuffer.getWritePointer(0), numSamples);
    freqModAmtRamp.applySum(freqModAmtBuffer.getWritePointer(0), numSamples);

    // calculate LFO in Hz
    lfo.process(lfoBuffer.getWritePointer(0), numSamples);
    for (unsigned int n = 0; n < numSamples; ++n)
    {
        const float curFreq = freqInBuffer.getSample(0, n);
        const float modAmtHz = curFreq * FreqModAmtMax * freqModAmtBuffer.getSample(0, n);
        const float modFreq = modAmtHz * lfoBuffer.getSample(0, n) + curFreq;
        freqInBuffer.setSample(0, n, modFreq);
    }

    // always process left channel
    svfLeft.process(lpfOutBuffer.getWritePointer(0),
                    bpfOutBuffer.getWritePointer(0),
                    hpfOutBuffer.getWritePointer(0),
                    buffer.getReadPointer(0),
                    freqInBuffer.getReadPointer(0),
                    resoInBuffer.getReadPointer(0),
                    numSamples);

    // if stereo, also process right channel
    if (numChannels > 1)
    {
        svfRight.process(lpfOutBuffer.getWritePointer(1),
                         bpfOutBuffer.getWritePointer(1),
                         hpfOutBuffer.getWritePointer(1),
                         buffer.getReadPointer(1),
                         freqInBuffer.getReadPointer(0),
                         resoInBuffer.getReadPointer(0),
                         numSamples);
    }

    // mix outputs
    lpfRamp.applyGain(lpfOutBuffer.getArrayOfWritePointers(), numChannels, numSamples);
    bpfRamp.applyGain(bpfOutBuffer.getArrayOfWritePointers(), numChannels, numSamples);
    hpfRamp.applyGain(hpfOutBuffer.getArrayOfWritePointers(), numChannels, numSamples);
    buffer.clear();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.addFrom(ch, 0, lpfOutBuffer, ch, 0, numSamples);
        buffer.addFrom(ch, 0, bpfOutBuffer, ch, 0, numSamples);
        buffer.addFrom(ch, 0, hpfOutBuffer, ch, 0, numSamples);
    }
}

juce::AudioProcessorEditor* SVFProcessor::createEditor()
{
    return new SVFEditor(*this);
}

CREATE_PLUGIN(SVFProcessor)
