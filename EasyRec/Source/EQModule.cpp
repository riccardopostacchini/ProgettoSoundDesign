#include "EQModule.h"

EQModule::EQModule() {}

EQModule::~EQModule() {}

void EQModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (float) spec.sampleRate;

    filterChain.reset();
    filterChain.prepare(spec);

    bassSmoothedDb.reset(sampleRate, 0.04);
    trebleSmoothedDb.reset(sampleRate, 0.04);
    bassSmoothedDb.setCurrentAndTargetValue(0.0f);
    trebleSmoothedDb.setCurrentAndTargetValue(0.0f);

    lastBassDb = 0.0f;
    lastTrebleDb = 0.0f;
    forceUpdate = true;
    updateFilters(0.0f, 0.0f);
}

void EQModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    const float bassDb = bassSmoothedDb.getNextValue();
    const float trebleDb = trebleSmoothedDb.getNextValue();

    if (buffer.getNumSamples() > 1)
    {
        const int samplesToSkip = buffer.getNumSamples() - 1;
        bassSmoothedDb.skip(samplesToSkip);
        trebleSmoothedDb.skip(samplesToSkip);
    }

    if (forceUpdate || std::abs(bassDb - lastBassDb) > 0.01f || std::abs(trebleDb - lastTrebleDb) > 0.01f)
        updateFilters(bassDb, trebleDb);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filterChain.process(context);
}

void EQModule::reset()
{
    filterChain.reset();
    bassSmoothedDb.reset(sampleRate, 0.04);
    trebleSmoothedDb.reset(sampleRate, 0.04);
    bassSmoothedDb.setCurrentAndTargetValue(lastBassDb);
    trebleSmoothedDb.setCurrentAndTargetValue(lastTrebleDb);
    forceUpdate = true;
}

void EQModule::setBassAmount(float bassAmountDb)
{
    bassSmoothedDb.setTargetValue(juce::jlimit(-10.0f, 10.0f, bassAmountDb));
}

void EQModule::setTrebleAmount(float trebleAmountDb)
{
    trebleSmoothedDb.setTargetValue(juce::jlimit(-10.0f, 10.0f, trebleAmountDb));
}

void EQModule::setSoftPreset(bool softPreset)
{
    if (isSoftPreset == softPreset)
        return;

    isSoftPreset = softPreset;
    forceUpdate = true;
}

void EQModule::updateFilters(float bassDb, float trebleDb)
{
    // Rumble roll-off costante sotto 80 Hz.
    *filterChain.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 80.0f, 0.707f);

    // Bass control: curva logaritmica/percettiva (console-like).
    const float bassNorm = juce::jlimit(-1.0f, 1.0f, bassDb / 10.0f);
    const float bassCurve = std::copysign(std::pow(std::abs(bassNorm), 0.75f), bassNorm);
    const float bassShelfDb = bassCurve * 10.0f;
    const float bassGain = juce::Decibels::decibelsToGain(bassShelfDb);
    *filterChain.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 120.0f, 0.75f, bassGain);

    // Treble control: high-shelf + presence peak dipendenti dal preset.
    const bool soft = isSoftPreset;
    const float trebleNorm = juce::jlimit(-1.0f, 1.0f, trebleDb / 10.0f);
    const float maxShelfDb = soft ? 6.0f : 9.0f;
    const float maxPresenceDb = soft ? 1.5f : 3.0f;
    const float shelfFreq = soft ? 12000.0f : 10000.0f;
    const float shelfQ = soft ? 0.55f : 0.70f;
    const float presenceFreq = soft ? 4500.0f : 5000.0f;
    const float presenceQ = soft ? 0.65f : 1.0f;

    const float shelfDb = trebleNorm * maxShelfDb;
    const float presenceDb = trebleNorm * maxPresenceDb;
    *filterChain.get<2>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, shelfFreq, shelfQ, juce::Decibels::decibelsToGain(shelfDb));
    *filterChain.get<3>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, presenceFreq, presenceQ, juce::Decibels::decibelsToGain(presenceDb));

    lastBassDb = bassDb;
    lastTrebleDb = trebleDb;
    forceUpdate = false;
}
