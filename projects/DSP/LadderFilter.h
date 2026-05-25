#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>

// Adapted code from moogvcf.m
// Original authors: Välimäki, Bilbao, Smith, Abel, Pakarinen, Berners

namespace DSP
{

    class LadderFilter
    {
    public:
        LadderFilter();
        ~LadderFilter();

        LadderFilter(const LadderFilter&) = delete;
        LadderFilter& operator=(const LadderFilter&) = delete;
        LadderFilter(LadderFilter&&) = delete;
        LadderFilter& operator=(LadderFilter&&) = delete;

        void prepare(double sampleRate);
        void setCutoff(float hz);
        void setResonance(float norm);
        float process(float input);

    private:
        // Internal helpers
        void updateCoeffs();           // updates g, h0, h1 based on cutoff and fs2
        void updateFIR();              // (re)builds the 11-tap Hamming-windowed FIR at Wn=0.5
        float processFIRIn(float x);   // anti-imaging FIR at fs2, stream-based
        float processFIROut(float x);  // anti-aliasing FIR at fs2, stream-based
        float processLadderSubsample(float x); // one fs2 subsample through ladder

        // Streaming FIR state and coefficients
        static constexpr int kFIROrder = 10;
        static constexpr int kFIRTaps  = kFIROrder + 1; // 11
        std::array<double, kFIRTaps> firH {};           // fir1(10,0.5) Hamming taps, normalized
        std::array<float, kFIRTaps>  firDelayIn {};     // input FIR state at fs2
        std::array<float, kFIRTaps>  firDelayOut {};    // output FIR state at fs2
        int firIndexIn  = 0;
        int firIndexOut = 0;

        // Ladder state (5 internal nodes, keep "previous" as in MATLAB)
        std::array<float, 5> w {};     // current
        std::array<float, 5> wold {};  // z^-1

        // Parameters and derived values
        double sampleRate { 48000.0 }; // fs
        double fs2        { 96000.0 }; // 2*fs
        float cutoffHz    { 1000.0f }; // fc
        float resonanceNorm { 0.0f };  // res
        float Gcomp       { 0.5f };    // passband gain compensation
        float Gres        { 0.0f };    // effective resonance (mapped from resonanceNorm)
        float g           { 0.0f };    // feedback coeff at fs2
        float h0          { 0.0f };    // FIR part in each 1st-order section
        float h1          { 0.0f };

        // Utility
        static inline double sinc(double x) {
            if (std::abs(x) < 1e-12) return 1.0;
            return std::sin(M_PI * x) / (M_PI * x);
        }
    };

}
