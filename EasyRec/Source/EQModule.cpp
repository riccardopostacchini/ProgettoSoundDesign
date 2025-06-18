#include "EQModule.h"

EQModule::EQModule() {}

EQModule::~EQModule() {}

void EQModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (float)spec.sampleRate;

    filterChain.reset();
    filterChain.prepare(spec);

    setLowCutFreq(110.0f);
    setToneAmount(0.0f);
}

void EQModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto channelBlock = block.getSingleChannelBlock(ch);
            juce::dsp::ProcessContextReplacing<float> context(channelBlock);

            // Processa ogni filtro IIR individualmente per il canale
            filterChain.get<0>().process(context); // Low Cut
            filterChain.get<1>().process(context); // Tone
        }
}

void EQModule::reset()
{
    filterChain.reset();
}

void EQModule::setLowCutFreq(float freq)
{
    freq = juce::jlimit(20.0f, 150.0f, freq);

    *filterChain.get<0>().coefficients =
        *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, freq);
}

void EQModule::setToneAmount(float amountDb)
{
    float gain = juce::Decibels::decibelsToGain(amountDb);

    *filterChain.get<1>().coefficients =
        *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 5000.0f, 0.5f, gain);
}
