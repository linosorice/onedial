// Standalone test: drives ZoneBlender directly with a sine input, sweeps
// the dial across all five zones at 48 kHz, and asserts that the output
// RMS changes meaningfully between zones — proving the NAM stage is alive.

#include "BinaryData.h"
#include "ZoneBlender.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <cmath>
#include <cstdio>
#include <vector>

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int    kBlockSize  = 512;
constexpr int    kNumWarmup  = 16; // blocks to settle internal state
constexpr int    kNumMeas    = 64; // blocks measured per zone

// Pre-build a deterministic pink-noise test buffer, normalized to a target
// RMS. Pink noise approximates a guitar's spectral content far better than
// a single sine — and avoids the fundamental-cancellation artifact when
// summing the outputs of two amp models with different phase responses.
std::vector<float> makePinkNoise(int totalSamples, float targetRms)
{
    std::vector<float> sig(totalSamples);
    juce::Random rng(0xDEADBEEF);
    constexpr int N = 7;
    std::array<float, N> rows{};
    for (auto& r : rows) r = rng.nextFloat() * 2.0f - 1.0f;
    unsigned counter = 0;
    double sumSq = 0.0;
    for (int i = 0; i < totalSamples; ++i)
    {
        ++counter;
        for (int k = 0; k < N; ++k)
            if ((counter & ((1u << k) - 1u)) == 0)
                rows[k] = rng.nextFloat() * 2.0f - 1.0f;
        float s = 0.0f;
        for (auto v : rows) s += v;
        s /= (float) N;
        sig[i] = s;
        sumSq += (double) s * s;
    }
    const float currentRms = (float) std::sqrt(sumSq / (double) totalSamples);
    if (currentRms > 1e-9f)
    {
        const float scale = targetRms / currentRms;
        for (auto& s : sig) s *= scale;
    }
    return sig;
}

float measureZone(ZoneBlender& zb, float dial, const std::vector<float>& signal)
{
    juce::AudioBuffer<float> buffer(1, kBlockSize);
    const int totalBlocks = (int) signal.size() / kBlockSize;
    jassert(totalBlocks >= kNumWarmup + kNumMeas);

    for (int b = 0; b < kNumWarmup; ++b)
    {
        std::copy(signal.begin() + (b * kBlockSize),
                  signal.begin() + ((b + 1) * kBlockSize),
                  buffer.getWritePointer(0));
        zb.process(buffer, dial);
    }

    double accum = 0.0;
    for (int b = 0; b < kNumMeas; ++b)
    {
        const int idx = kNumWarmup + b;
        std::copy(signal.begin() + (idx * kBlockSize),
                  signal.begin() + ((idx + 1) * kBlockSize),
                  buffer.getWritePointer(0));
        zb.process(buffer, dial);
        accum += buffer.getRMSLevel(0, 0, kBlockSize);
    }
    return (float) (accum / kNumMeas);
}
} // namespace

int main()
{
    ZoneBlender zb;

    bool allLoaded = true;
    allLoaded &= zb.loadZoneProfile(0, BinaryData::clean_nam,  BinaryData::clean_namSize);
    allLoaded &= zb.loadZoneProfile(1, BinaryData::warm_nam,   BinaryData::warm_namSize);
    allLoaded &= zb.loadZoneProfile(2, BinaryData::crunch_nam, BinaryData::crunch_namSize);
    allLoaded &= zb.loadZoneProfile(3, BinaryData::drive_nam,  BinaryData::drive_namSize);
    allLoaded &= zb.loadZoneProfile(4, BinaryData::lead_nam,   BinaryData::lead_namSize);

    zb.loadZoneIR(0, BinaryData::clean_wav,  BinaryData::clean_wavSize);
    zb.loadZoneIR(1, BinaryData::warm_wav,   BinaryData::warm_wavSize);
    zb.loadZoneIR(2, BinaryData::crunch_wav, BinaryData::crunch_wavSize);
    zb.loadZoneIR(3, BinaryData::drive_wav,  BinaryData::drive_wavSize);
    zb.loadZoneIR(4, BinaryData::lead_wav,   BinaryData::lead_wavSize);

    if (!allLoaded || zb.getLoadedZoneCount() != 5)
    {
        std::fprintf(stderr, "FAIL: only %d/5 zones loaded\n", zb.getLoadedZoneCount());
        return 2;
    }

    zb.prepare(kSampleRate, kBlockSize);

    struct ZonePoint { const char* name; float dial; };
    // Pure-zone points and the four 50/50 blend midpoints between them.
    // The midpoints (1.5, 3.5, 5.5, 7.5) are where blendFactor reaches its
    // maximum value of 1.0 inside getZoneBlend's mapping — a linear
    // crossfade would visibly dip there for uncorrelated amp models.
    const ZonePoint points[] = {
        {"CLEAN",       0.0f},
        {"clean+warm",  1.5f},
        {"WARM",        2.0f},
        {"warm+crunch", 3.5f},
        {"CRUNCH",      4.0f},
        {"crunch+drive",5.5f},
        {"DRIVE",       6.0f},
        {"drive+lead",  7.5f},
        {"LEAD",       10.0f},
    };

    // -18 dBFS RMS pink noise — guitar-realistic input level.
    const float inputRms = std::pow(10.0f, -18.0f / 20.0f);
    const auto signal = makePinkNoise((kNumWarmup + kNumMeas + 4) * kBlockSize, inputRms);

    // Pre-pass: drive every zone briefly to settle each NAM's internal
    // state. Without this, the first measured zone shows ~3 dB of
    // run-to-run jitter (the model right after Reset+prewarm hasn't fully
    // converged for a high-amplitude broadband input).
    for (float d : {0.0f, 2.0f, 4.0f, 6.0f, 10.0f})
        (void) measureZone(zb, d, signal);

    std::vector<float> rms;
    std::printf("Input: pink noise -18 dBFS RMS, %d Hz, %d-sample blocks\n",
                (int) kSampleRate, kBlockSize);
    std::printf("%-14s %-6s %-10s %s\n", "Zone", "Dial", "RMS", "dBFS");
    std::printf("--------------------------------------------\n");
    for (const auto& p : points)
    {
        const float r = measureZone(zb, p.dial, signal);
        rms.push_back(r);
        const float dbfs = 20.0f * std::log10(std::max(r, 1e-9f));
        std::printf("%-14s %-6.1f %-10.6f %.2f\n", p.name, p.dial, r, dbfs);
    }

    // Pass criterion #1: at least 3 distinct RMS clusters (within 1%).
    int distinctZones = 0;
    for (size_t i = 0; i < rms.size(); ++i)
    {
        bool unique = true;
        for (size_t j = 0; j < i; ++j)
            if (std::abs(rms[i] - rms[j]) / std::max(rms[j], 1e-6f) < 0.01f)
                unique = false;
        if (unique) ++distinctZones;
    }
    std::printf("--------------------------------------------\n");
    std::printf("Distinct RMS levels: %d / %zu\n", distinctZones, rms.size());

    // Pass criterion #2: each midpoint sits within 4 dB of the louder of
    // its two neighbours. Equal-power crossfade between two uncorrelated
    // amp models keeps the midpoint within ~1 dB on pink noise; some
    // residual sag remains where adjacent amps have strongly different
    // phase responses (clean vs warm, in this preset). Linear crossfade
    // would be a strict 3 dB worse than equal-power on uncorrelated
    // sources, so 4 dB is a meaningful bound that still passes equal-power
    // but would fail a linear regression.
    const std::pair<int, std::pair<int,int>> midpointTriples[] = {
        {1, {0, 2}},  // clean+warm   between CLEAN and WARM
        {3, {2, 4}},  // warm+crunch  between WARM and CRUNCH
        {5, {4, 6}},  // crunch+drive between CRUNCH and DRIVE
        {7, {6, 8}},  // drive+lead   between DRIVE and LEAD
    };
    int sagFails = 0;
    std::printf("\nCrossfade midpoint sag check (equal-power should be small):\n");
    for (const auto& t : midpointTriples)
    {
        const float midDb = 20.0f * std::log10(std::max(rms[(size_t) t.first], 1e-9f));
        const float aDb   = 20.0f * std::log10(std::max(rms[(size_t) t.second.first],  1e-9f));
        const float bDb   = 20.0f * std::log10(std::max(rms[(size_t) t.second.second], 1e-9f));
        const float louderEndpointDb = std::max(aDb, bDb);
        const float sagDb = louderEndpointDb - midDb;
        std::printf("  %-14s mid %.2f dB vs max(a,b) %.2f dB → sag %.2f dB\n",
                    points[(size_t) t.first].name, midDb, louderEndpointDb, sagDb);
        if (sagDb > 4.0f) ++sagFails;
    }

    if (distinctZones < 3)
    {
        std::fprintf(stderr, "FAIL: dial not changing tone meaningfully\n");
        return 1;
    }
    if (sagFails > 0)
    {
        std::fprintf(stderr, "FAIL: %d midpoint(s) sag more than 4 dB — crossfade regressed\n", sagFails);
        return 1;
    }

    std::printf("\nPASS: dial sweep distinct + midpoints within 4 dB\n");
    return 0;
}
