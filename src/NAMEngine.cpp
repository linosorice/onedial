#include <NAM/get_dsp.h>
#include "NAMEngine.h"
#include <fstream>

NAMEngine::NAMEngine() = default;
NAMEngine::~NAMEngine() = default;

bool NAMEngine::loadModel(const void* data, size_t dataSize)
{
    try
    {
        // Write binary data to a temp file since NAM API loads from path
        auto tempFile = juce::File::getSpecialLocation(
                            juce::File::tempDirectory)
                            .getChildFile("onedial_nam_" + juce::String(juce::Random::getSystemRandom().nextInt64()) + ".nam");

        {
            juce::FileOutputStream out(tempFile);
            if (!out.openedOk())
                return false;
            out.write(data, dataSize);
        }

        bool result = loadModelFromFile(tempFile.getFullPathName());
        tempFile.deleteFile();
        return result;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool NAMEngine::loadModelFromFile(const juce::String& filePath)
{
    try
    {
        auto newModel = nam::get_dsp(std::filesystem::path(filePath.toStdString()));
        if (newModel == nullptr)
            return false;

        model = std::move(newModel);
        model->prewarm();

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

void NAMEngine::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    if (model)
        model->prewarm();
}

void NAMEngine::process(float* buffer, int numSamples)
{
    if (!isLoaded() || numSamples <= 0)
        return;

    // NAM API expects NAM_SAMPLE** (pointer to array of channel pointers)
    // We have mono, so one channel
    float* inputChannels[1] = {buffer};
    float* outputChannels[1] = {buffer}; // in-place

    model->process(inputChannels, outputChannels, numSamples);
}

bool NAMEngine::isLoaded() const
{
    return model != nullptr;
}

void NAMEngine::reset()
{
    model.reset();
}
