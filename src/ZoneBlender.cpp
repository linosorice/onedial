#include "ZoneBlender.h"

ZoneBlender::ZoneBlender() = default;
ZoneBlender::~ZoneBlender() = default;

void ZoneBlender::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    tempBufferA.setSize(2, samplesPerBlock);
    tempBufferB.setSize(2, samplesPerBlock);

    for (auto& engine : namEngines)
        engine.prepare(sampleRate, samplesPerBlock);

    for (auto& ir : irLoaders)
        ir.prepare(sampleRate, samplesPerBlock);
}

void ZoneBlender::releaseResources()
{
    for (auto& engine : namEngines)
        engine.reset();

    for (auto& ir : irLoaders)
        ir.reset();
}

void ZoneBlender::getZoneBlend(float dialValue, int& zoneA, int& zoneB, float& blendFactor)
{
    // Clamp dial to 0-10
    dialValue = juce::jlimit(0.0f, 10.0f, dialValue);

    // Each zone spans 2 units: 0-2, 2-4, 4-6, 6-8, 8-10
    // Zone index = floor(dialValue / 2), clamped to 0-4
    zoneA = juce::jlimit(0, 4, static_cast<int>(dialValue / 2.0f));

    // Handle edge case at exactly 10.0
    if (zoneA >= 4)
    {
        zoneA = 4;
        zoneB = 4;
        blendFactor = 0.0f;
        return;
    }

    // Position within the current zone (0.0 to 1.0)
    float posInZone = (dialValue - static_cast<float>(zoneA) * 2.0f) / 2.0f;

    // Blend starts in the second half of the zone
    // First half (0.0-0.5): pure zone A
    // Second half (0.5-1.0): blend from zone A to zone B
    if (posInZone <= 0.5f)
    {
        zoneB = zoneA;
        blendFactor = 0.0f;
    }
    else
    {
        zoneB = juce::jlimit(0, 4, zoneA + 1);
        // Map 0.5-1.0 to 0.0-1.0
        blendFactor = (posInZone - 0.5f) * 2.0f;
    }
}

const char* ZoneBlender::getZoneName(float dialValue)
{
    int zoneA, zoneB;
    float blend;
    getZoneBlend(dialValue, zoneA, zoneB, blend);
    return kZones[static_cast<size_t>(zoneA)].name;
}

void ZoneBlender::process(juce::AudioBuffer<float>& buffer, float dialValue)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numSamples == 0)
        return;

    int zoneA, zoneB;
    float blendFactor;
    getZoneBlend(dialValue, zoneA, zoneB, blendFactor);

    // If no models loaded, pass through
    if (!namEngines[static_cast<size_t>(zoneA)].isLoaded())
        return;

    // Process mono (channel 0) through NAM, then copy to other channels
    // NAM models are mono

    if (blendFactor < 0.001f)
    {
        // Pure zone A - single NAM + IR
        namEngines[static_cast<size_t>(zoneA)].process(buffer.getWritePointer(0), numSamples);
        irLoaders[static_cast<size_t>(zoneA)].process(buffer.getWritePointer(0), numSamples);
    }
    else
    {
        // Blend between zone A and zone B
        // Copy input to temp buffers
        tempBufferA.copyFrom(0, 0, buffer, 0, 0, numSamples);
        tempBufferB.copyFrom(0, 0, buffer, 0, 0, numSamples);

        // Process each zone
        namEngines[static_cast<size_t>(zoneA)].process(tempBufferA.getWritePointer(0), numSamples);
        irLoaders[static_cast<size_t>(zoneA)].process(tempBufferA.getWritePointer(0), numSamples);

        if (namEngines[static_cast<size_t>(zoneB)].isLoaded())
        {
            namEngines[static_cast<size_t>(zoneB)].process(tempBufferB.getWritePointer(0), numSamples);
            irLoaders[static_cast<size_t>(zoneB)].process(tempBufferB.getWritePointer(0), numSamples);
        }

        // Mix: output = A * (1 - blend) + B * blend
        float* output = buffer.getWritePointer(0);
        const float* aData = tempBufferA.getReadPointer(0);
        const float* bData = tempBufferB.getReadPointer(0);
        const float invBlend = 1.0f - blendFactor;

        for (int i = 0; i < numSamples; ++i)
            output[i] = aData[i] * invBlend + bData[i] * blendFactor;
    }

    // Copy mono result to all channels
    for (int ch = 1; ch < numChannels; ++ch)
        buffer.copyFrom(ch, 0, buffer, 0, 0, numSamples);
}

void ZoneBlender::loadZoneProfile(int zoneIndex, const void* data, size_t dataSize)
{
    if (zoneIndex >= 0 && zoneIndex < kNumZones)
        namEngines[static_cast<size_t>(zoneIndex)].loadModel(data, dataSize);
}

void ZoneBlender::loadZoneProfileFromFile(int zoneIndex, const juce::String& filePath)
{
    if (zoneIndex >= 0 && zoneIndex < kNumZones)
        namEngines[static_cast<size_t>(zoneIndex)].loadModelFromFile(filePath);
}

void ZoneBlender::loadZoneIR(int zoneIndex, const void* data, size_t dataSize, double sampleRate)
{
    if (zoneIndex >= 0 && zoneIndex < kNumZones)
        irLoaders[static_cast<size_t>(zoneIndex)].loadFromMemory(data, dataSize, sampleRate);
}

void ZoneBlender::loadZoneIRFromFile(int zoneIndex, const juce::String& filePath, double sampleRate)
{
    if (zoneIndex >= 0 && zoneIndex < kNumZones)
        irLoaders[static_cast<size_t>(zoneIndex)].loadFromFile(filePath, sampleRate);
}
