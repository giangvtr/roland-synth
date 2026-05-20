#pragma once

#include <BaseProcessor.h>

#include <FIR.h>
#include <Ramp.h>

namespace Param
{
    namespace ID
    {
        static const juce::String CabType { "cab_type" };
        static const juce::String Level { "level" };
    }

    namespace Name
    {
        static const juce::String CabType { "Cab. Type" };
        static const juce::String Level { "Level" };
    }
}

class CabinetSimulatorProcessor final : public mrta::BaseProcessor
{
public:
    static constexpr unsigned int ImpulseResponseLength { 512 };

    CabinetSimulatorProcessor();
    ~CabinetSimulatorProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    DSP::FIR fir;
    DSP::Ramp<float> levelRamp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CabinetSimulatorProcessor)
};
