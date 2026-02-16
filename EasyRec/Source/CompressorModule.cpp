#include "CompressorModule.h"

CompressorModule::CompressorModule() {}

CompressorModule::~CompressorModule() {}

void CompressorModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (float)spec.sampleRate;
    compressor.prepare(spec);

    // Parametri di default (soft)
    compressor.setAttack(3.005f);
    compressor.setRelease(41.87f);
    compressor.setRatio(3.8f);
    compressor.setThreshold(-18.0f);
}

void CompressorModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);
}

void CompressorModule::setAmount(float amount)
{
    // Amount -> modifica solo il threshold (carattere fissato dalla modalità)
    if (softMode)
    {
        const float threshold = juce::jmap(amount, -30.0f, -18.0f);
        compressor.setThreshold(threshold);
    }
    else
    {
        const float threshold = juce::jmap(amount, -20.0f, -5.0f);
        compressor.setThreshold(threshold);
    }
}

void CompressorModule::setSoftMode(bool soft)
{
    softMode = soft;

    if (soft)
    {
        compressor.setAttack(3.005f);
        compressor.setRelease(41.87f);
        compressor.setRatio(3.8f);
        compressor.setThreshold(-18.0f);
    }
    else
    {
        compressor.setAttack(23.0f);
        compressor.setRelease(100.0f);
        compressor.setRatio(101.0f);
        compressor.setThreshold(-5.0f);
    }
}

void CompressorModule::reset()
{
    compressor.reset();
}
