#include "IRLoader.h"

IRLoader::IRLoader()
    : convolution(juce::dsp::Convolution::NonUniform{128})
{
}

IRLoader::~IRLoader() = default;

void IRLoader::loadFromMemory(const void* data, size_t dataSize, double sampleRate)
{
    convolution.loadImpulseResponse(data, dataSize,
                                     juce::dsp::Convolution::Stereo::no,
                                     juce::dsp::Convolution::Trim::yes,
                                     0); // 0 = use full IR
    loaded = true;
}

void IRLoader::loadFromFile(const juce::String& filePath, double sampleRate)
{
    juce::File irFile(filePath);
    if (!irFile.existsAsFile())
        return;

    convolution.loadImpulseResponse(irFile,
                                     juce::dsp::Convolution::Stereo::no,
                                     juce::dsp::Convolution::Trim::yes,
                                     0);
    loaded = true;
}

void IRLoader::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;
    convolution.prepare(spec);
}

void IRLoader::process(float* buffer, int numSamples)
{
    if (!loaded)
        return;

    juce::dsp::AudioBlock<float> block(&buffer, 1, static_cast<size_t>(numSamples));
    juce::dsp::ProcessContextReplacing<float> context(block);
    convolution.process(context);
}

void IRLoader::reset()
{
    convolution.reset();
    loaded = false;
}
