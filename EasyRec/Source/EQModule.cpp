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
    // Rumble roll-off leggero, più vicino al comportamento "vocal bass" tipo CLA.
    *filterChain.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 75.0f, 0.72f);

    // Bass control "CLA-like": body su 115 Hz + anti-mud su 280 Hz.
    const float bassNorm = juce::jlimit(-1.0f, 1.0f, bassDb / 10.0f);
    float bassCurve = 0.0f;
    if (bassNorm >= 0.0f)
        bassCurve = std::pow(bassNorm, 1.12f);
    else
        bassCurve = -std::pow(std::abs(bassNorm), 0.90f);

    const float bassShelfDb = juce::jmap(bassCurve, -1.0f, 1.0f, -6.0f, 9.0f);
    const float bassGain = juce::Decibels::decibelsToGain(bassShelfDb);
    *filterChain.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 105.0f, 0.68f, bassGain);

    // Mud control: attivo soprattutto quando il bass e' in boost.
    const float mudCutDb = (bassCurve > 0.0f) ? (-3.2f * std::pow(bassCurve, 0.9f))
                                              : (0.6f * bassCurve);
    *filterChain.get<2>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, 260.0f, 1.0f, juce::Decibels::decibelsToGain(mudCutDb));

    // Treble in stile vocal-strip: presence + air, con risposta musicale.
    const bool soft = isSoftPreset;
    const float presetScale = soft ? 0.75f : 1.0f;
    const float trebleNorm = juce::jlimit(-1.0f, 1.0f, trebleDb / 10.0f);

    // Curva: piu' dolce vicino a 0, piu' incisiva salendo.
    float trebleCurve = 0.0f;
    if (trebleNorm >= 0.0f)
        trebleCurve = std::pow(trebleNorm, 1.08f);
    else
        trebleCurve = -std::pow(std::abs(trebleNorm), 1.0f);

    // Range target "CLA-like"
    float presenceDb = juce::jmap(trebleCurve, -1.0f, 1.0f, -2.5f, 5.0f) * presetScale;
    float shelfDb = juce::jmap(trebleCurve, -1.0f, 1.0f, -4.0f, 10.0f) * presetScale;

    // Protezione sibilanti: oltre +5 riduce leggermente la presence.
    if (trebleDb > 5.0f)
    {
        const float t = juce::jlimit(0.0f, 1.0f, (trebleDb - 5.0f) / 5.0f);
        presenceDb -= 0.8f * t;
    }

    const float shelfFreq = 11000.0f;
    const float shelfQ = 0.62f;
    const float presenceFreq = 5000.0f;
    const float presenceQ = 0.8f;

    *filterChain.get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, shelfFreq, shelfQ, juce::Decibels::decibelsToGain(shelfDb));
    *filterChain.get<4>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, presenceFreq, presenceQ, juce::Decibels::decibelsToGain(presenceDb));

    lastBassDb = bassDb;
    lastTrebleDb = trebleDb;
    forceUpdate = false;
}
