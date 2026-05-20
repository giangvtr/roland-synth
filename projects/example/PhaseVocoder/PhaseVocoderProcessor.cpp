#include "PhaseVocoderProcessor.h"
#include "PhaseVocoderEditor.h"

static const std::vector<mrta::ParameterInfo> params
{
    mrta::ParameterInfo("ratio", "Ratio", "X", 1.f, 0.5f, 2.f, 0.001f, 1.f)
};

PhaseVocoderProcessor::PhaseVocoderProcessor() :
    mrta::BaseProcessor(params),
    overlapSaveLeft(WINDOW_SIZE),
    overlapSaveRight(WINDOW_SIZE),
    overlapAddLeft(WINDOW_SIZE),
    overlapAddRight(WINDOW_SIZE),
    phaseLeft(WINDOW_SIZE),
    phaseRight(WINDOW_SIZE),
    windowFunction(WINDOW_SIZE, juce::dsp::WindowingFunction<float>::hann)
{
    registerParameterCallback("ratio",
        [this] (float value, bool)
        {
            const auto auxRatio = value;
            analysisHopSize = static_cast<size_t>(std::floor(BASE_HOP_SIZE / auxRatio));
            ratio = static_cast<double>(synthesisHopSize) / static_cast<double>(analysisHopSize);

            overlapSaveLeft.setHopSizeSamples(analysisHopSize);
            overlapSaveRight.setHopSizeSamples(analysisHopSize);
            phaseLeft.setAnalysisHopSize(analysisHopSize);
            phaseRight.setAnalysisHopSize(analysisHopSize);
        });
}

PhaseVocoderProcessor::~PhaseVocoderProcessor()
{
}

void PhaseVocoderProcessor::prepare(double, int maxBufferSize)
{
    overlapSaveLeft.prepare();
    overlapSaveRight.prepare();

    overlapAddLeft.prepare();
    overlapAddRight.prepare();

    phaseLeft.reset();
    phaseRight.reset();

    resamplerLeft.reset();
    resamplerRight.reset();

    overlapSaveLeft.setHopSizeSamples(analysisHopSize);
    overlapSaveRight.setHopSizeSamples(analysisHopSize);

    overlapAddLeft.setHopSizeSamples(synthesisHopSize);
    overlapAddRight.setHopSizeSamples(synthesisHopSize);

    phaseLeft.setAnalysisHopSize(analysisHopSize);
    phaseLeft.setSynthesisHopSize(synthesisHopSize);
    phaseRight.setAnalysisHopSize(analysisHopSize);
    phaseRight.setSynthesisHopSize(synthesisHopSize);

    resampleFracAcc = 0.0;

    resamplerBuffer.resize(2 * maxBufferSize);
    std::fill(resamplerBuffer.begin(), resamplerBuffer.end(), 0.f);
}

void PhaseVocoderProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    const auto numSamples { static_cast<size_t>(buffer.getNumSamples()) };
    const auto numChannels { static_cast<size_t>(buffer.getNumChannels()) };
    std::array<float, WINDOW_SIZE> analysisFrameBuffer;
    std::array<float, WINDOW_SIZE> synthesisFrameBuffer;

    auto resampleLengthFrac { numSamples * ratio };
    auto resampleLength { static_cast<size_t>(std::floor(resampleLengthFrac)) };
    resampleFracAcc += resampleLengthFrac - resampleLength;
    if (resampleFracAcc >= 1.0)
    {
        ++resampleLength;
        --resampleFracAcc;
    }

    const auto resamplerRatio { static_cast<double>(resampleLength) / static_cast<double>(numSamples) };

    overlapSaveLeft.writeNewBuffer(buffer.getReadPointer(0), numSamples);
    while (overlapSaveLeft.readReadyFrame(analysisFrameBuffer.data()))
    {
        windowFunction.multiplyWithWindowingTable(analysisFrameBuffer.data(), WINDOW_SIZE);
        phaseLeft.processFrame(synthesisFrameBuffer.data(), analysisFrameBuffer.data());
        windowFunction.multiplyWithWindowingTable(synthesisFrameBuffer.data(), WINDOW_SIZE);
        overlapAddLeft.writeNewFrame(synthesisFrameBuffer.data());
    }

    overlapAddLeft.readyReadBuffer(resamplerBuffer.data(), resampleLength);
    resamplerLeft.process(resamplerRatio, resamplerBuffer.data(), buffer.getWritePointer(0),
        static_cast<int>(numSamples), static_cast<int>(resampleLength), 1);

    if (numChannels > 1)
    {
        overlapSaveRight.writeNewBuffer(buffer.getReadPointer(1), numSamples);
        while (overlapSaveRight.readReadyFrame(analysisFrameBuffer.data()))
        {
            windowFunction.multiplyWithWindowingTable(analysisFrameBuffer.data(), WINDOW_SIZE);
            phaseRight.processFrame(synthesisFrameBuffer.data(), analysisFrameBuffer.data());
            windowFunction.multiplyWithWindowingTable(synthesisFrameBuffer.data(), WINDOW_SIZE);
            overlapAddRight.writeNewFrame(synthesisFrameBuffer.data());
        }

        overlapAddRight.readyReadBuffer(resamplerBuffer.data(), resampleLength);
        resamplerRight.process(resamplerRatio, resamplerBuffer.data(), buffer.getWritePointer(1),
            static_cast<int>(numSamples), static_cast<int>(resampleLength), 1);
    }
}

juce::AudioProcessorEditor* PhaseVocoderProcessor::createEditor()
{
    return new PhaseVocoderEditor(*this);
}

CREATE_PLUGIN(PhaseVocoderProcessor)
