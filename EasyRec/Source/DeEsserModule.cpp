#include "DeEsserModule.h"

DeEsserModule::DeEsserModule() {}

DeEsserModule::~DeEsserModule() {}

void DeEsserModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (float)spec.sampleRate;

    bandpassFilter.reset();
    bandpassFilter.prepare(spec);
    *bandpassFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 6000.0f, 1.0f);

    compressor.reset();
    compressor.prepare(spec);

    compressor.setAttack(10.0f);
    compressor.setRelease(100.0f);
    compressor.setRatio(4.0f);
    compressor.setThreshold(-24.0f);
}

void DeEsserModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);

    // Creiamo una copia del buffer per il segnale originale (dry)
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Processa bandpass filter per ogni canale singolarmente (mono filter)
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock(ch);
        juce::dsp::ProcessContextReplacing<float> channelContext(channelBlock);

        bandpassFilter.process(channelContext);
    }

    // Processa compressore multicanale
    juce::dsp::ProcessContextReplacing<float> compressorContext(block);
    compressor.process(compressorContext);

    // Mix dry/wet controllato da amount
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        auto* dryData = dryBuffer.getReadPointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            channelData[i] = dryData[i] * (1.0f - amount) + channelData[i] * amount;
        }
    }
}

void DeEsserModule::setAmount(float newAmount)
{
    amount = juce::jlimit(0.0f, 1.0f, newAmount);
}

void DeEsserModule::reset()
{
    bandpassFilter.reset();
    compressor.reset();
}
