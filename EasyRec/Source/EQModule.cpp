#include "EQModule.h"

EQModule::EQModule() {}

EQModule::~EQModule() {}

void EQModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (float)spec.sampleRate;

    filterChain.reset();
    filterChain.prepare(spec);

    lowCutSmoothed.reset(sampleRate, 0.05); // 50 ms smoothing
    lowCutSmoothed.setCurrentAndTargetValue(110.0f);
    lastLowCutHz = 110.0f;
    toneSmoothedDb.reset(sampleRate, 0.05);
    toneSmoothedDb.setCurrentAndTargetValue(0.0f);
    lastToneDb = 0.0f;

    // Inizializza coefficenti per evitare stati non definiti
    constexpr float lowCutQ = 1.0f;
    auto hp = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 110.0f, lowCutQ);
    *filterChain.get<0>().state = *hp;
    *filterChain.get<1>().state = *hp;
    *filterChain.get<2>().state = *hp;
    *filterChain.get<3>().state = *hp;
    *filterChain.get<4>().state = *hp;
    *filterChain.get<5>().state = *hp;
    *filterChain.get<6>().state = *hp;
    *filterChain.get<7>().state = *hp;

    auto shelf = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 6000.0f, 0.7f, 1.0f);
    *filterChain.get<8>().state = *shelf;
}

void EQModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    // Aggiorna lentamente la frequenza di taglio per evitare scatti
    const float smoothedHz = lowCutSmoothed.getNextValue();
    // Avanza lo smoothing per la dimensione del blocco
    if (buffer.getNumSamples() > 1)
        lowCutSmoothed.skip((size_t)buffer.getNumSamples() - 1);
    if (lastLowCutHz < 0.0f || std::abs(smoothedHz - lastLowCutHz) > 0.01f)
    {
        constexpr float lowCutQ = 1.0f; // ginocchio più ripido
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, smoothedHz, lowCutQ);
        *filterChain.get<0>().state = *coeffs;
        *filterChain.get<1>().state = *coeffs;
        *filterChain.get<2>().state = *coeffs;
        *filterChain.get<3>().state = *coeffs;
        *filterChain.get<4>().state = *coeffs;
        *filterChain.get<5>().state = *coeffs;
        *filterChain.get<6>().state = *coeffs;
        *filterChain.get<7>().state = *coeffs;
        lastLowCutHz = smoothedHz;
    }

    const float smoothedToneDb = toneSmoothedDb.getNextValue();
    if (buffer.getNumSamples() > 1)
        toneSmoothedDb.skip((size_t)buffer.getNumSamples() - 1);
    if (std::abs(smoothedToneDb - lastToneDb) > 0.01f)
    {
        const float gain = juce::Decibels::decibelsToGain(smoothedToneDb);
        // High-shelf per una brillantezza più naturale sulla voce
        *filterChain.get<8>().state =
            *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 6000.0f, 0.7f, gain);
        lastToneDb = smoothedToneDb;
    }

    juce::dsp::AudioBlock<float> block(buffer);

    juce::dsp::ProcessContextReplacing<float> context(block);
    filterChain.process(context);
}

void EQModule::reset()
{
    filterChain.reset();
    lowCutSmoothed.reset(sampleRate, 0.05);
    lowCutSmoothed.setCurrentAndTargetValue(lastLowCutHz);
    toneSmoothedDb.reset(sampleRate, 0.05);
    toneSmoothedDb.setCurrentAndTargetValue(lastToneDb);
}

void EQModule::setLowCutFreq(float freq)
{
    freq = juce::jlimit(20.0f, 200.0f, freq);

    lowCutSmoothed.setTargetValue(freq);
}

void EQModule::setToneAmount(float amountDb)
{
    toneSmoothedDb.setTargetValue(amountDb);
}
