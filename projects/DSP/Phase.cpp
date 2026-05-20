#include "Phase.h"

template<typename T>
static constexpr T Log2(T x)
{
    return x == 1 ? 0 : 1 + Log2(x >> 1);
}

static float Wrap2pi(float x)
{
    return x - static_cast<float>(2 * M_PI) * std::round(x * static_cast<float>(1.0 / (2 * M_PI)));
}

Phase::Phase(size_t _windowSize) :
    windowSize(_windowSize),
    fft(static_cast<int>(Log2(windowSize))),
    analysisFftBuffer(windowSize * 2),
    synthesisFftBuffer(windowSize * 2),
    analysisPrevPhaseBuffer(windowSize / 2 + 1),
    synthesisPrevPhaseBuffer(windowSize / 2 + 1),
    analysisHopSize { windowSize / 4 },
    synthesisHopSize { windowSize / 4 }
{
    reset();
}

Phase::~Phase()
{
}

void Phase::reset()
{
    std::fill(analysisFftBuffer.begin(), analysisFftBuffer.end(), 0.f);
    std::fill(synthesisFftBuffer.begin(), synthesisFftBuffer.end(), 0.f);
    std::fill(analysisPrevPhaseBuffer.begin(), analysisPrevPhaseBuffer.end(), 0.f);
    std::fill(synthesisPrevPhaseBuffer.begin(), synthesisPrevPhaseBuffer.end(), 0.f);
}

void Phase::processFrame(float* outputFrame, const float* inputFrame)
{
    std::memcpy(analysisFftBuffer.data(), inputFrame, windowSize * sizeof(float));
    fft.performRealOnlyForwardTransform(analysisFftBuffer.data(), true);

    const auto binCount { (windowSize / 2) + 1 };
    for (size_t k = 0; k < binCount; ++k)
    {
        const auto binReal { 2 * k };
        const auto binImag { 2 * k + 1 };

        const std::complex<float> analysisCurrBin { analysisFftBuffer[binReal], analysisFftBuffer[binImag] };
        const auto analysisCurrPhase { std::arg(analysisCurrBin) };

        const auto analysisPrevPhase { analysisPrevPhaseBuffer[k] };
        analysisPrevPhaseBuffer[k] = analysisCurrPhase;

        const auto omegaK { k * static_cast<float>(2 * M_PI) / windowSize };
        const auto kappa { Wrap2pi(analysisCurrPhase - analysisPrevPhase - analysisHopSize * omegaK) };
        const auto omegaInst { omegaK + kappa / static_cast<float>(analysisHopSize) };

        const auto synthesisCurrPhase { synthesisPrevPhaseBuffer[k] + synthesisHopSize * omegaInst };
        synthesisPrevPhaseBuffer[k] = (synthesisCurrPhase >= static_cast<float>(2 * M_PI)) ? synthesisCurrPhase - static_cast<float>(2 * M_PI): synthesisCurrPhase;

        const std::complex<float> synthesisCurrBin { std::polar(std::abs(analysisCurrBin), synthesisCurrPhase) };
        synthesisFftBuffer[binReal] = synthesisCurrBin.real();
        synthesisFftBuffer[binImag] = synthesisCurrBin.imag();
    }

    fft.performRealOnlyInverseTransform(synthesisFftBuffer.data());
    std::memcpy(outputFrame, synthesisFftBuffer.data(), windowSize * sizeof(float));
}
