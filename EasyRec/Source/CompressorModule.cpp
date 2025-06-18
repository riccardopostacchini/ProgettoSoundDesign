#include "CompressorModule.h"

CompressorModule::CompressorModule() {}

CompressorModule::~CompressorModule() {}

void CompressorModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (float)spec.sampleRate;
    compressor.prepare(spec);

    // Parametri di default in soft mode
    compressor.setAttack(10.0f);
    compressor.setRelease(100.0f);
    compressor.setRatio(2.0f);
    compressor.setThreshold(-24.0f);
}

void CompressorModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);
}

void CompressorModule::setAmount(float amount)
{
    // Mappa l'amount da 0..1 a threshold e ratio a seconda della modalità
    if (softMode)
    {
        float threshold = juce::jmap(amount, -60.0f, -10.0f);
        compressor.setThreshold(threshold);
        compressor.setRatio(2.0f);
    }
    else
    {
        float threshold = juce::jmap(amount, -40.0f, -5.0f);
        compressor.setThreshold(threshold);
        compressor.setRatio(6.0f);
    }
}

void CompressorModule::setSoftMode(bool soft)
{
    softMode = soft;

    if (soft)
    {
        compressor.setAttack(10.0f);
        compressor.setRelease(100.0f);
        compressor.setRatio(2.0f);
    }
    else
    {
        compressor.setAttack(1.0f);
        compressor.setRelease(50.0f);
        compressor.setRatio(6.0f);
    }
}

void CompressorModule::reset()
{
    compressor.reset();
}
