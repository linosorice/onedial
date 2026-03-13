#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

// Loads and applies an IR cabinet convolution
class IRLoader
{
public:
    IRLoader();
    ~IRLoader();

    // Load IR from memory (binary resource)
    void loadFromMemory(const void* data, size_t dataSize, double sampleRate);

    // Load IR from a .wav file path
    void loadFromFile(const juce::String& filePath, double sampleRate);

    // Prepare for playback
    void prepare(double sampleRate, int samplesPerBlock);

    // Process mono audio in-place
    void process(float* buffer, int numSamples);

    // Check if loaded
    bool isLoaded() const { return loaded; }

    // Reset
    void reset();

private:
    juce::dsp::Convolution convolution;
    bool loaded = false;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IRLoader)
};
