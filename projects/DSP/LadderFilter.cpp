#include "LadderFilter.h"

// Adapted code from moogvcf.m
// Original authors: Välimäki, Bilbao, Smith, Abel, Pakarinen, Berners

namespace DSP
{

LadderFilter::LadderFilter() = default;
LadderFilter::~LadderFilter() = default;

void LadderFilter::prepare(double newSampleRate)
{
    sampleRate = std::max(1.0, newSampleRate);
    fs2 = 2.0 * sampleRate;

    // Clear states
    w.fill(0.0f);
    wold.fill(0.0f);
    firDelayIn.fill(0.0f);
    firDelayOut.fill(0.0f);
    firIndexIn = 0;
    firIndexOut = 0;

    // Update parameters and FIR
    updateCoeffs();
    updateFIR();
}

void LadderFilter::setCutoff(float hz)
{
    cutoffHz = std::max(0.0f, hz);
    updateCoeffs();
}

void LadderFilter::setResonance(float value)
{
    resonanceNorm = std::max(0.0f, value);
    Gres = resonanceNorm;
}

float LadderFilter::process(float input)
{
    // 2x oversampled path:
    // Step 1 at fs2: upsampled value = input, filter -> ladder -> out filter
    float up0 = processFIRIn(input);
    float y0  = processLadderSubsample(up0);
    float y0f = processFIROut(y0);

    // Step 2 at fs2: upsampled value = 0, filter -> ladder -> out filter
    float up1 = processFIRIn(0.0f);
    float y1  = processLadderSubsample(up1);
    float y1f = processFIROut(y1);

    // Decimate by 2: keep the first of the pair, matching MATLAB's out = out2(1:2:end)
    (void)y1f; // unused but computed to keep states consistent
    return y0f;
}

// -------------------- Internal helpers --------------------

void LadderFilter::updateCoeffs()
{
    // MATLAB: fs2 = 2*fs; g = 2*pi*fc/fs2;
    // That simplifies to g = pi * fc / fs.
    const double gRaw = M_PI * static_cast<double>(cutoffHz) / std::max(1.0, sampleRate);
    // Keep g within a sane range to avoid instability when fc is very high
    g  = static_cast<float>(std::clamp(gRaw, 0.0, 1.0));
    h0 = g / 1.3f;
    h1 = g * 0.3f / 1.3f;

    Gres = resonanceNorm; // keep in sync
}

// Build a symmetric 11-tap Hamming-windowed FIR lowpass with Wn=0.5 (normalized to Nyquist), like fir1(10,0.5).
// For Wn=0.5, the ideal LP impulse is h_ideal[n] = sinc(n - M), with M = N/2, since 2*Wn = 1.
// Then apply Hamming window and normalize to DC gain 1 (sum of taps = 1.0).
void LadderFilter::updateFIR()
{
    constexpr int N = kFIROrder;           // 10
    constexpr int M = N / 2;               // 5
    // Hamming window: w[n] = 0.54 - 0.46*cos(2*pi*n/N)
    double sum = 0.0;
    for (int n = 0; n <= N; ++n)
    {
        const double wHamm = 0.54 - 0.46 * std::cos(2.0 * M_PI * n / N);
        const double hIdeal = sinc(static_cast<double>(n - M)); // Wn=0.5 => 2*Wn=1 => sinc(n-M)
        firH[n] = hIdeal * wHamm;
        sum += firH[n];
    }
    // Normalize so DC gain is 1.0
    if (std::abs(sum) > 1e-12) {
        for (auto& c : firH) c /= sum;
    }
}

// Simple direct-form FIR process for the upsampled input stream
float LadderFilter::processFIRIn(float x)
{
    // Circular buffer push
    firIndexIn = (firIndexIn - 1 + kFIRTaps) % kFIRTaps;
    firDelayIn[firIndexIn] = x;

    // Convolution
    double acc = 0.0;
    int idx = firIndexIn;
    for (int i = 0; i < kFIRTaps; ++i)
    {
        acc += firH[i] * static_cast<double>(firDelayIn[idx]);
        idx = (idx + 1) % kFIRTaps;
    }
    return static_cast<float>(acc);
}

// Same FIR for the output stream (anti-aliasing at fs2)
float LadderFilter::processFIROut(float x)
{
    // Circular buffer push
    firIndexOut = (firIndexOut - 1 + kFIRTaps) % kFIRTaps;
    firDelayOut[firIndexOut] = x;

    // Convolution
    double acc = 0.0;
    int idx = firIndexOut;
    for (int i = 0; i < kFIRTaps; ++i)
    {
        acc += firH[i] * static_cast<double>(firDelayOut[idx]);
        idx = (idx + 1) % kFIRTaps;
    }
    return static_cast<float>(acc);
}

float LadderFilter::processLadderSubsample(float x)
{
    // MATLAB core per fs2 sample:
    // u = in2(n) - 4*Gres*(wold(5) - Gcomp*in2(n));
    const float u = x - 4.0f * Gres * (wold[4] - Gcomp * x);

    // Saturation tanh
    w[0] = std::tanh(u);

    // Four 1st-order sections with FIR part (h0,h1) and IIR part (1-g)
    // w(2) = h0*w(1) + h1*wold(1) + (1-g)*wold(2);
    w[1] = h0 * w[0] + h1 * wold[0] + (1.0f - g) * wold[1];
    w[2] = h0 * w[1] + h1 * wold[1] + (1.0f - g) * wold[2];
    w[3] = h0 * w[2] + h1 * wold[2] + (1.0f - g) * wold[3];
    w[4] = h0 * w[3] + h1 * wold[3] + (1.0f - g) * wold[4];

    const float y = w[4];

    // Update z^-1 states
    wold = w;

    return y;
}

}
