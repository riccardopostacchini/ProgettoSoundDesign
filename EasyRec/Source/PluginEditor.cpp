#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::String formatHz(double hz)
{
    if (std::floor(hz) == hz)
        return juce::String((int)hz);
    return juce::String(hz, 1);
}

static juce::String formatValue(float value)
{
    if (std::floor(value) == value)
        return juce::String((int)value);
    return juce::String(value, 1);
}

//==============================================================================
EasyRecAudioProcessorEditor::EasyRecAudioProcessorEditor (EasyRecAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Font
    earlyGameBoyFont = juce::Typeface::createSystemTypefaceFor(BinaryData::EarlyGameBoy_ttf, BinaryData::EarlyGameBoy_ttfSize);
    auto font = juce::Font(earlyGameBoyFont);
    font.setHeight(9.0f);
    font.setBold(true);

    setSize (825, 660);
    
    // Background
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::Gameboy_png, BinaryData::Gameboy_pngSize);

    // === COMP KNOB ===
    compKnobDrawable = juce::Drawable::createFromImageData(BinaryData::Comp_Knob_svg, BinaryData::Comp_Knob_svgSize);
    compKnobLookAndFeel.knobImage = compKnobDrawable.get();
    compKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    compKnob.setRange(0.0, 1.0, 0.01);
    compKnob.setValue(0.5);
    compKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    compKnob.setLookAndFeel(&compKnobLookAndFeel);
    addAndMakeVisible(compKnob);

    // === EQ KNOBS ===
    
    // === LOW KNOB ===
    lowKnobDrawable = juce::Drawable::createFromImageData(BinaryData::LowEq_Knob_svg, BinaryData::LowEq_Knob_svgSize);
    lowKnobLookAndFeel.knobImage = lowKnobDrawable.get();
    lowKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lowKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lowKnob.setRange(20.0, 200.0, 0.1);
    lowKnob.setValue(0.5);  // valore iniziale ragionevole
    lowKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    lowKnob.setLookAndFeel(&lowKnobLookAndFeel);
    addAndMakeVisible(lowKnob);

    lowLabelDescription.setText("Low Cut", juce::dontSendNotification);
    lowLabelDescription.setFont(font);
    lowLabelDescription.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
    //addAndMakeVisible(lowLabelDescription);

    lowLabelValue.setFont(font);
    lowLabelValue.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
    lowLabelValue.setJustificationType(juce::Justification::centred);
    lowLabelValue.setEditable(false, false, false);
    lowLabelValue.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(lowLabelValue);

    lowLabelValue.setText(formatHz(lowKnob.getValue()), juce::dontSendNotification);

    // === TONE KNOB ===
    toneKnobDrawable = juce::Drawable::createFromImageData(BinaryData::ToneEq_Knob_svg, BinaryData::ToneEq_Knob_svgSize);
    toneKnobLookAndFeel.knobImage = toneKnobDrawable.get();
    toneKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    toneKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    toneKnob.setRange(0.0, 1.0, 0.01);
    toneKnob.setValue(0.5);
    toneKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    toneKnob.setLookAndFeel(&toneKnobLookAndFeel);
    addAndMakeVisible(toneKnob);

    toneLabelDescription.setText("Tone", juce::dontSendNotification);
    toneLabelDescription.setFont(font);
    toneLabelDescription.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
    //addAndMakeVisible(toneLabelDescription);

    toneLabelValue.setFont(font);
    toneLabelValue.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
    toneLabelValue.setJustificationType(juce::Justification::centred);
    toneLabelValue.setEditable(false, false, false);
    toneLabelValue.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(toneLabelValue);

    toneLabelValue.setText(formatValue(toneKnob.getValue()), juce::dontSendNotification);

    // === DE-ESSER KNOB ===
    deeKnobDrawable = juce::Drawable::createFromImageData(BinaryData::DeEsser_Knob_svg, BinaryData::DeEsser_Knob_svgSize);
    deeKnobLookAndFeel.knobImage = deeKnobDrawable.get();
    deeKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    deeKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    deeKnob.setRange(0.0, 1.0, 0.01);
    deeKnob.setValue(0.5);
    deeKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    deeKnob.setLookAndFeel(&deeKnobLookAndFeel);
    addAndMakeVisible(deeKnob);

    // === SATURATION KNOB ===
    satKnobDrawable = juce::Drawable::createFromImageData(BinaryData::Satur_Knob_svg, BinaryData::Satur_Knob_svgSize);
    satKnobLookAndFeel.knobImage = satKnobDrawable.get();
    satKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    satKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    satKnob.setRange(0.0, 1.0, 0.01);
    satKnob.setValue(0.5);
    satKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    satKnob.setLookAndFeel(&satKnobLookAndFeel);
    addAndMakeVisible(satKnob);

    // === OUTPUT KNOB ===
    outKnobDrawable = juce::Drawable::createFromImageData(BinaryData::Output_Knob_svg, BinaryData::Output_Knob_svgSize);
    outKnobLookAndFeel.knobImage = outKnobDrawable.get();
    outKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    outKnob.setRange(0.0, 1.0, 0.01);
    outKnob.setValue(0.5);
    outKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    outKnob.setLookAndFeel(&outKnobLookAndFeel);
    addAndMakeVisible(outKnob);

    auto formatValue = [](float value) -> juce::String
    {
        if (std::floor(value) == value)
            return juce::String((int)value); // Nessun decimale se è intero

        return juce::String(value, 1);
    };
    
    auto setupLabel = [this, font = font, formatValue](juce::Label& label, juce::Slider& slider)
    {
        label.setFont(font);
        label.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
        label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        label.setJustificationType(juce::Justification::centred);
        label.setEditable(false, false, false);
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);

        float displayedValue = (slider.getValue() - 0.5f) * 24.0f;
        label.setText(formatValue(displayedValue), juce::dontSendNotification);
    };
    
    auto setupLabelFormatted = [this, font](juce::Label& label, juce::Slider& slider, std::function<juce::String(double)> formatter)
    {
        label.setFont(font);
        label.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
        label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        label.setJustificationType(juce::Justification::centred);
        label.setEditable(false, false, false);
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);

        double displayedValue = slider.getValue();
        label.setText(formatter(displayedValue), juce::dontSendNotification);
    };
    
    enum class FormatterType { FormatHz, FormatValue };

    auto addListener = [](juce::Slider& slider, juce::Label& label, std::function<juce::String(double)> formatter, FormatterType type)
    {
        slider.onValueChange = [&slider, &label, formatter, type]()
        {
            double displayedValue;

            if (type == FormatterType::FormatHz)
                displayedValue = slider.getValue(); // lowKnob: valore già in Hz
            else
                displayedValue = (slider.getValue() - 0.5) * 24.0; // gli altri: scala

            label.setText(formatter(displayedValue), juce::dontSendNotification);
        };
    };

    // Usa le funzioni per tutte le label e slider
    setupLabel(compLabel, compKnob);
    setupLabelFormatted(lowLabelValue, lowKnob, formatHz);
    setupLabelFormatted(toneLabelValue, toneKnob, formatValue);
    setupLabel(deeLabel, deeKnob);
    setupLabel(satLabel, satKnob);
    setupLabel(outLabel, outKnob);

    addListener(compKnob, compLabel, formatValue, FormatterType::FormatValue);
    addListener(lowKnob, lowLabelValue, formatHz, FormatterType::FormatHz);
    addListener(toneKnob, toneLabelValue, formatValue, FormatterType::FormatValue);
    addListener(deeKnob, deeLabel, formatValue, FormatterType::FormatValue);
    addListener(satKnob, satLabel, formatValue, FormatterType::FormatValue);
    addListener(outKnob, outLabel, formatValue, FormatterType::FormatValue);
    
    // === TOGGLE COMP ===
    softHighlightDrawable = juce::Drawable::createFromImageData(BinaryData::Soft_Comp_svg, BinaryData::Soft_Comp_svgSize);
    hardHighlightDrawable = juce::Drawable::createFromImageData(BinaryData::Hard_Comp_svg, BinaryData::Hard_Comp_svgSize);

    toggleCompButton.setAlpha(0.0f);
    toggleCompButton.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
    toggleCompButton.setClickingTogglesState(true);
    toggleCompButton.setToggleState(isSoftMode, juce::dontSendNotification);
    addAndMakeVisible(toggleCompButton);

    softHighlightArea = { 253, 190, 37, 41 };
    hardHighlightArea = { 289, 190, 37, 41 };
    currentCompHighlightRect = softHighlightArea.toFloat();

    toggleCompButton.onClick = [this]()
    {
        isSoftMode = !isSoftMode;
        toggleCompButton.setToggleState(isSoftMode, juce::dontSendNotification);
        compAnimating = true;
        startTimerHz(60);
    };

    // === TOGGLE SATURAZIONE ===
    saturToggleButton.setAlpha(0.0f);
    saturToggleButton.setClickingTogglesState(true);
    saturToggleButton.setToggleState(isSoftSaturMode, juce::dontSendNotification);
    addAndMakeVisible(saturToggleButton);

    softSatHighlightArea = { 254, 278, 37, 41 };
    hardSatHighlightArea = { 289, 278, 37, 41 };
    currentSaturHighlightRect = softSatHighlightArea.toFloat();

    softSatHighlightDrawable = juce::Drawable::createFromImageData(BinaryData::Soft_Satur_svg, BinaryData::Soft_Satur_svgSize);
    hardSatHighlightDrawable = juce::Drawable::createFromImageData(BinaryData::Hard_Satur_svg, BinaryData::Hard_Satur_svgSize);

    saturToggleButton.onClick = [this]()
    {
        isSoftSaturMode = !isSoftSaturMode;
        saturToggleButton.setToggleState(isSoftSaturMode, juce::dontSendNotification);
        saturAnimating = true;
        startTimerHz(60);
    };
}

EasyRecAudioProcessorEditor::~EasyRecAudioProcessorEditor() = default;

//==============================================================================
void EasyRecAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    if (backgroundImage.isValid())
        g.drawImage(backgroundImage, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);

    if (isSoftMode && softHighlightDrawable && !compAnimating)
        softHighlightDrawable->drawWithin(g, softHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (!isSoftMode && hardHighlightDrawable && !compAnimating)
        hardHighlightDrawable->drawWithin(g, hardHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (compAnimating)
    {
        auto* drawable = isSoftMode ? softHighlightDrawable.get() : hardHighlightDrawable.get();
        if (drawable)
            drawable->drawWithin(g, currentCompHighlightRect, juce::RectanglePlacement::centred, 1.0f);
    }

    if (isSoftSaturMode && softSatHighlightDrawable && !saturAnimating)
        softSatHighlightDrawable->drawWithin(g, softSatHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (!isSoftSaturMode && hardSatHighlightDrawable && !saturAnimating)
        hardSatHighlightDrawable->drawWithin(g, hardSatHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (saturAnimating)
    {
        auto* drawable = isSoftSaturMode ? softSatHighlightDrawable.get() : hardSatHighlightDrawable.get();
        if (drawable)
            drawable->drawWithin(g, currentSaturHighlightRect, juce::RectanglePlacement::centred, 1.0f);
    }
}

void EasyRecAudioProcessorEditor::resized()
{
    compKnob.setBounds(344, 194, 37, 37);
    lowKnob.setBounds(419, 116, 37, 37);
    toneKnob.setBounds(490, 116, 37, 37);
    deeKnob.setBounds(489, 194, 37, 37);
    satKnob.setBounds(344, 282, 37, 37);
    outKnob.setBounds(489, 274, 38, 37);
    
    compLabel.setBounds(compKnob.getBounds().translated(1, 0));
    //lowLabelDescription.setBounds(lowKnob.getBounds().translated(1, 0));
    lowLabelValue.setBounds(lowKnob.getBounds().translated(1, 0));
    //toneLabelDescription.setBounds(toneKnob.getBounds().translated(1, 0));
    toneLabelValue.setBounds(toneKnob.getBounds().translated(1, 0));
    deeLabel.setBounds(deeKnob.getBounds().translated(1, 0));
    satLabel.setBounds(satKnob.getBounds().translated(1, 0));
    outLabel.setBounds(outKnob.getBounds().translated(1, 0));
    
    toggleCompButton.setBounds(240, 195, 100, 30);
    saturToggleButton.setBounds(240, 283, 100, 30);
    
}


void EasyRecAudioProcessorEditor::timerCallback()
{
    auto animate = [](juce::Rectangle<float>& current, const juce::Rectangle<float>& target) -> bool
    {
        constexpr float speed = 0.2f;
        auto delta = target.getCentre() - current.getCentre();
        if (delta.getDistanceFromOrigin() < 0.5f)
        {
            current = target;
            return false;
        }

        current.setCentre(current.getCentre() + delta * speed);
        return true;
    };

    bool stillAnimating = false;

    if (compAnimating)
        compAnimating = animate(currentCompHighlightRect, isSoftMode ? softHighlightArea.toFloat() : hardHighlightArea.toFloat());

    if (saturAnimating)
        saturAnimating = animate(currentSaturHighlightRect, isSoftSaturMode ? softSatHighlightArea.toFloat() : hardSatHighlightArea.toFloat());

    stillAnimating = compAnimating || saturAnimating;

    if (!stillAnimating)
        stopTimer();

    repaint();
}

void EasyRecAudioProcessorEditor::updateEQ()
{
    float lowFreq = (float)lowKnob.getValue();
    float toneVal = (float)toneKnob.getValue();

    audioProcessor.updateEQFilters(lowFreq, toneVal);
}

