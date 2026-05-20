#include "CabinetSimulatorProcessor.h"
#include "CabinetSimulatorEditor.h"
#include "Cabs.h"

static const std::vector<mrta::ParameterInfo> ParameterInfos
{
    { Param::ID::CabType,   Param::Name::CabType,   { "2x12", "4x12" }, 0 },
    { Param::ID::Level,     Param::Name::Level,     "dB", -12.0f, -60.f, 12.f, 0.1f, 3.8018f },
};

CabinetSimulatorProcessor::CabinetSimulatorProcessor() :
    mrta::BaseProcessor(ParameterInfos),
    fir(ImpulseResponseLength, 2)
{
    registerParameterCallback(Param::ID::CabType,
        [this] (float value, bool)
        {
            const auto type { static_cast<unsigned int>(std::round(value)) };
            switch (type)
            {
                case 0:
                    fir.loadImpulseResponse(Cab2x12, Cab2x12_length);
                    break;

                case 1:
                    fir.loadImpulseResponse(Cab4x12, Cab4x12_length);
                    break;

                default:
                    jassertfalse;
            }
            fir.clear();
        });

    registerParameterCallback(Param::ID::Level,
        [this] (float value, bool forced)
        {
            float dbValue { 0.f };
            if (value > -60.f)
                dbValue = std::pow(10.f, value * 0.05f);
            levelRamp.setTarget(dbValue, forced);
        });
}

CabinetSimulatorProcessor::~CabinetSimulatorProcessor()
{
}

void CabinetSimulatorProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    fir.clear();
}

void CabinetSimulatorProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    fir.process(buffer.getArrayOfWritePointers(), buffer.getArrayOfReadPointers(), buffer.getNumChannels(), buffer.getNumSamples());
    levelRamp.applyGain(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples());
}

juce::AudioProcessorEditor* CabinetSimulatorProcessor::createEditor()
{
    return new CabinetSimulatorEditor(*this);
}

CREATE_PLUGIN(CabinetSimulatorProcessor)
