#include "DeEsserModule.h"

DeEsserModule::DeEsserModule() {}

DeEsserModule::~DeEsserModule() {}

void DeEsserModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (float)spec.sampleRate;

    bandpassFilter.reset();
    bandpassFilter.prepare(spec);
    *bandpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, currentCenterHz, 1.5f);

    compressor.reset();
    compressor.prepare(spec);

    compressor.setAttack(10.0f);
    compressor.setRelease(100.0f);
    compressor.setRatio(4.0f);
    compressor.setThreshold(-24.0f);

    dryBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    bandBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    detectBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);

    for (int i = 0; i < numDetectBands; ++i)
    {
        detectFilters[i].reset();
        detectFilters[i].prepare(spec);
        *detectFilters[i].state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, detectFreqs[i], 2.0f);
    }
}

void DeEsserModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);

    // Copia dry (senza allocazioni in realtime)
    if (dryBuffer.getNumSamples() < buffer.getNumSamples() || dryBuffer.getNumChannels() < buffer.getNumChannels())
        dryBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
    dryBuffer.makeCopyOf(buffer, true);

    if (bandBuffer.getNumSamples() < buffer.getNumSamples() || bandBuffer.getNumChannels() < buffer.getNumChannels())
        bandBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
    if (detectBuffer.getNumSamples() < buffer.getNumSamples() || detectBuffer.getNumChannels() < buffer.getNumChannels())
        detectBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());

    // === Auto-detection: trova la banda più energica tra 4-10 kHz ===
    int bestIndex = 0;
    float bestEnergy = 0.0f;

    for (int i = 0; i < numDetectBands; ++i)
    {
        detectBuffer.makeCopyOf(buffer, true);
        juce::dsp::AudioBlock<float> detectBlock(detectBuffer);
        juce::dsp::ProcessContextReplacing<float> detectContext(detectBlock);
        detectFilters[i].process(detectContext);

        double energy = 0.0;
        for (int ch = 0; ch < detectBuffer.getNumChannels(); ++ch)
        {
            auto* data = detectBuffer.getReadPointer(ch);
            for (int n = 0; n < detectBuffer.getNumSamples(); ++n)
                energy += (double)data[n] * (double)data[n];
        }

        if (energy > bestEnergy)
        {
            bestEnergy = (float)energy;
            bestIndex = i;
        }
    }

    const float targetHz = detectFreqs[bestIndex];
    if (std::abs(targetHz - currentCenterHz) > 1.0f)
    {
        *bandpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, targetHz, 1.5f);
        currentCenterHz = targetHz;
    }

    // === Split-band: estrai banda sibilante e comprimila ===
    bandBuffer.makeCopyOf(buffer, true);
    juce::dsp::AudioBlock<float> bandBlock(bandBuffer);
    juce::dsp::ProcessContextReplacing<float> bandContext(bandBlock);
    bandpassFilter.process(bandContext);

    // Salva banda originale prima della compressione
    detectBuffer.makeCopyOf(bandBuffer, true);

    // Comprime solo la banda
    juce::dsp::ProcessContextReplacing<float> compressorContext(bandBlock);
    compressor.process(compressorContext);

    // Mix split-band: sottrae la riduzione dalla banda al segnale dry
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        auto* dryData = dryBuffer.getReadPointer(ch);
        auto* bandDry = detectBuffer.getReadPointer(ch);
        auto* bandComp = bandBuffer.getReadPointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float reduction = bandDry[i] - bandComp[i];
            channelData[i] = dryData[i] - reduction * amount;
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
