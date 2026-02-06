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

    // Timer per animazioni (micro-movimenti + highlight)
    startTimerHz(60);
    
    // Background
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::Gameboy_png, BinaryData::Gameboy_pngSize);
    backgroundImageAlt = juce::ImageCache::getFromMemory(BinaryData::Gameboy2_prova_png, BinaryData::Gameboy2_prova_pngSize);
    buttonSliderImage = juce::ImageCache::getFromMemory(BinaryData::buttonSlider_png, BinaryData::buttonSlider_pngSize);
    eqOnImage = juce::ImageCache::getFromMemory(BinaryData::eqon_png, BinaryData::eqon_pngSize);

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
    lowKnob.setValue(110.0);  // valore iniziale centrale reale
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

    toneLabelValue.setText(formatValue((toneKnob.getValue() - 0.5f) * 20.0f), juce::dontSendNotification);


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
    outKnob.setValue(6.0f / 9.0f);
    outKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    outKnob.setLookAndFeel(&outKnobLookAndFeel);
    addAndMakeVisible(outKnob);

    auto formatValue = [](float value) -> juce::String
    {
        if (std::floor(value) == value)
            return juce::String((int)value); // Nessun decimale se è intero

        return juce::String(value, 1);
    };

    auto formatMinMax = [formatValue](double valueDb) -> juce::String
    {
        if (valueDb <= -9.95)
            return "min";
        if (valueDb >= 9.95)
            return "max";
        return formatValue((float)valueDb);
    };
    
    auto setupLabel = [this, font = font, formatMinMax](juce::Label& label, juce::Slider& slider)
    {
        label.setFont(font);
        label.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
        label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        label.setJustificationType(juce::Justification::centred);
        label.setEditable(false, false, false);
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);

        float displayedValue = (slider.getValue() - 0.5f) * 20.0f;
        label.setText(formatMinMax(displayedValue), juce::dontSendNotification);
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

    auto addListener = [formatMinMax](juce::Slider& slider, juce::Label& label, std::function<juce::String(double)> formatter, FormatterType type)
    {
        slider.onValueChange = [&slider, &label, formatter, type, formatMinMax]()
        {
            double displayedValue;

            if (type == FormatterType::FormatHz)
                displayedValue = slider.getValue(); // lowKnob: valore già in Hz
            else
                displayedValue = (slider.getValue() - 0.5) * 20.0; // gli altri: scala

            if (type == FormatterType::FormatHz)
                label.setText(formatter(displayedValue), juce::dontSendNotification);
            else
                label.setText(formatMinMax(displayedValue), juce::dontSendNotification);
        };
    };

    // Usa le funzioni per tutte le label e slider
    setupLabel(compLabel, compKnob);
    setupLabelFormatted(lowLabelValue, lowKnob, formatHz);
    setupLabel(toneLabelValue, toneKnob);
    setupLabel(satLabel, satKnob);
    setupLabel(outLabel, outKnob);


    addListener(compKnob, compLabel, formatValue, FormatterType::FormatValue);
    addListener(lowKnob, lowLabelValue, formatHz, FormatterType::FormatHz);
    addListener(toneKnob, toneLabelValue, formatValue, FormatterType::FormatValue);
    addListener(satKnob, satLabel, formatValue, FormatterType::FormatValue);
    addListener(outKnob, outLabel, formatValue, FormatterType::FormatValue);

    auto outputValueToDb = [](double norm) -> double
    {
        constexpr double seg = 1.0 / 9.0;
        if (norm <= seg * 1.0) return juce::jmap(norm, seg * 0.0, seg * 1.0, -100.0, -40.0);
        if (norm <= seg * 2.0) return juce::jmap(norm, seg * 1.0, seg * 2.0,  -40.0, -30.0);
        if (norm <= seg * 3.0) return juce::jmap(norm, seg * 2.0, seg * 3.0,  -30.0, -20.0);
        if (norm <= seg * 4.0) return juce::jmap(norm, seg * 3.0, seg * 4.0,  -20.0, -15.0);
        if (norm <= seg * 5.0) return juce::jmap(norm, seg * 4.0, seg * 5.0,  -15.0, -10.0);
        if (norm <= seg * 6.0) return juce::jmap(norm, seg * 5.0, seg * 6.0,  -10.0,   0.0);
        if (norm <= seg * 7.0) return juce::jmap(norm, seg * 6.0, seg * 7.0,    0.0,   3.3333);
        if (norm <= seg * 8.0) return juce::jmap(norm, seg * 7.0, seg * 8.0,    3.3333, 6.6667);
        return juce::jmap(norm, seg * 8.0, seg * 9.0,    6.6667, 10.0);
    };

    auto updateOutputLabel = [this, formatValue, outputValueToDb]()
    {
        if (outKnob.getValue() <= 0.0001)
            outLabel.setText("-inf", juce::dontSendNotification);
        else
            outLabel.setText(formatValue(outputValueToDb(outKnob.getValue())), juce::dontSendNotification);
    };

    outKnob.onValueChange = [updateOutputLabel]()
    {
        updateOutputLabel();
    };
    updateOutputLabel();
    
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
        isSoftMode = toggleCompButton.getToggleState();
        compAnimating = true;
        startTimerHz(60);
    };
    toggleCompButton.onStateChange = [this]()
    {
        const bool newMode = toggleCompButton.getToggleState();
        if (newMode != isSoftMode)
        {
            isSoftMode = newMode;
            compAnimating = true;
            startTimerHz(60);
        }
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
    compBDrawable = juce::Drawable::createFromImageData(BinaryData::compB_png, BinaryData::compB_pngSize);
    satBDrawable = juce::Drawable::createFromImageData(BinaryData::satB_png, BinaryData::satB_pngSize);
    compADrawable = juce::Drawable::createFromImageData(BinaryData::compA_png, BinaryData::compA_pngSize);
    satADrawable = juce::Drawable::createFromImageData(BinaryData::satA_png, BinaryData::satA_pngSize);

    saturToggleButton.onClick = [this]()
    {
        isSoftSaturMode = saturToggleButton.getToggleState();
        saturAnimating = true;
        startTimerHz(60);
    };
    saturToggleButton.onStateChange = [this]()
    {
        const bool newMode = saturToggleButton.getToggleState();
        if (newMode != isSoftSaturMode)
        {
            isSoftSaturMode = newMode;
            saturAnimating = true;
            startTimerHz(60);
        }
    };

    // === D-pad RIGHT toggle (background switch) ===
    dpadRightButton.setClickingTogglesState(true);
    dpadRightButton.setToggleState(false, juce::dontSendNotification);
    dpadRightButton.setAlpha(0.0f);
    dpadRightButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    dpadRightButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    dpadRightButton.onClick = [this]()
    {
        showAltBackground = dpadRightButton.getToggleState();
        auto setVisibleAlt = [this](bool showAlt)
        {
            // Versione B: mostra EQ + Output + slider comp/sat
            const bool isB = showAlt;

            compKnob.setVisible(true);
            compLabel.setVisible(true);
            toggleCompButton.setVisible(true);

            satKnob.setVisible(true);
            satLabel.setVisible(true);
            saturToggleButton.setVisible(true);

            // Output sempre visibile
            outKnob.setVisible(true);
            outLabel.setVisible(true);

            // EQ sempre visibile
            lowKnob.setVisible(true);
            lowLabelValue.setVisible(true);
            toneKnob.setVisible(true);
            toneLabelValue.setVisible(true);
        };
        setVisibleAlt(showAltBackground);
        resized();
        if (showAltBackground)
            startTimerHz(60);
        repaint();
    };
    addAndMakeVisible(dpadRightButton);

    // On/Off buttons (B)
    satOnOffButton.setClickingTogglesState(true);
    satOnOffButton.setAlpha(0.0f);
    satOnOffButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    satOnOffButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(satOnOffButton);

    compOnOffButton.setClickingTogglesState(true);
    compOnOffButton.setAlpha(0.0f);
    compOnOffButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    compOnOffButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(compOnOffButton);

    eqOnOffButton.setClickingTogglesState(true);
    eqOnOffButton.setAlpha(0.0f);
    eqOnOffButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    eqOnOffButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(eqOnOffButton);

    // === APVTS Attachments ===
    auto& apvts = audioProcessor.getAPVTS();
    compAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "comp", compKnob);
    lowAttachment  = std::make_unique<APVTS::SliderAttachment>(apvts, "lowCut", lowKnob);
    toneAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "tone", toneKnob);
    satAttachment  = std::make_unique<APVTS::SliderAttachment>(apvts, "satur", satKnob);
    outAttachment  = std::make_unique<APVTS::SliderAttachment>(apvts, "out", outKnob);
    compSoftAttachment = std::make_unique<APVTS::ButtonAttachment>(apvts, "compSoft", toggleCompButton);
    satSoftAttachment  = std::make_unique<APVTS::ButtonAttachment>(apvts, "satSoft", saturToggleButton);
    compOnAttachment   = std::make_unique<APVTS::ButtonAttachment>(apvts, "compOn", compOnOffButton);
    satOnAttachment    = std::make_unique<APVTS::ButtonAttachment>(apvts, "satOn", satOnOffButton);
    eqOnAttachment     = std::make_unique<APVTS::ButtonAttachment>(apvts, "eqOn", eqOnOffButton);

    // Sincronizza lo stato iniziale dei toggle con i parametri
    isSoftMode = toggleCompButton.getToggleState();
    isSoftSaturMode = saturToggleButton.getToggleState();

    compOnOffButton.onStateChange = [this]()
    {
        resized();
        repaint();
    };
    satOnOffButton.onStateChange = [this]()
    {
        resized();
        repaint();
    };
    eqOnOffButton.onStateChange = [this]()
    {
        resized();
        repaint();
    };

    // Stato iniziale visibilità (Versione A)
    dpadRightButton.setToggleState(false, juce::dontSendNotification);
}

EasyRecAudioProcessorEditor::~EasyRecAudioProcessorEditor() = default;

//==============================================================================
void EasyRecAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    if (showAltBackground)
    {
        if (backgroundImageAlt.isValid())
            g.drawImage(backgroundImageAlt, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
    }
    else
    {
        if (backgroundImage.isValid())
            g.drawImage(backgroundImage, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
    }

    if (!showAltBackground && isSoftMode && softHighlightDrawable && !compAnimating)
        softHighlightDrawable->drawWithin(g, softHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (!showAltBackground && !isSoftMode && hardHighlightDrawable && !compAnimating)
        hardHighlightDrawable->drawWithin(g, hardHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (compAnimating)
    {
        auto* drawable = isSoftMode ? softHighlightDrawable.get() : hardHighlightDrawable.get();
        if (drawable)
            drawable->drawWithin(g, currentCompHighlightRect, juce::RectanglePlacement::centred, 1.0f);
    }

    if (!showAltBackground && isSoftSaturMode && softSatHighlightDrawable && !saturAnimating)
        softSatHighlightDrawable->drawWithin(g, softSatHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (!showAltBackground && !isSoftSaturMode && hardSatHighlightDrawable && !saturAnimating)
        hardSatHighlightDrawable->drawWithin(g, hardSatHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (saturAnimating)
    {
        auto* drawable = isSoftSaturMode ? softSatHighlightDrawable.get() : hardSatHighlightDrawable.get();
        if (drawable)
            drawable->drawWithin(g, currentSaturHighlightRect, juce::RectanglePlacement::centred, 1.0f);
    }

    // Versione B: indicatori hard (satB/compB)
    if (showAltBackground)
    {
        const int satOffset = (int)std::round(std::sin(spritePhase) * spriteAmplitudePx);
        const int compOffset = (int)std::round(std::sin(spritePhase + juce::MathConstants<float>::halfPi) * spriteAmplitudePx);

        auto satRect = satBRect.translated(0, satOffset);
        auto compRect = compBRect.translated(0, compOffset);

        if (isSoftSaturMode)
        {
            if (satADrawable)
                satADrawable->drawWithin(g, satRect.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }
        else
        {
            if (satBDrawable)
                satBDrawable->drawWithin(g, satRect.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }

        if (isSoftMode)
        {
            if (compADrawable)
                compADrawable->drawWithin(g, compRect.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }
        else
        {
            if (compBDrawable)
                compBDrawable->drawWithin(g, compRect.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }

        // eqon.png visibile solo quando EQ attivo
        if (eqOnOffButton.getToggleState() && eqOnImage.isValid())
        {
            g.drawImageWithin(eqOnImage,
                              eqOnRect.getX(),
                              eqOnRect.getY(),
                              eqOnRect.getWidth(),
                              eqOnRect.getHeight(),
                              juce::RectanglePlacement::centred);
        }

        // Base sotto gli slider (buttonSlider.png)
        if (buttonSliderImage.isValid())
        {
            if (satOnOffButton.getToggleState())
            {
                g.drawImageWithin(buttonSliderImage,
                                  satSliderBaseRect.getX(),
                                  satSliderBaseRect.getY(),
                                  satSliderBaseRect.getWidth(),
                                  satSliderBaseRect.getHeight(),
                                  juce::RectanglePlacement::stretchToFit);
            }

            if (compOnOffButton.getToggleState())
            {
                g.drawImageWithin(buttonSliderImage,
                                  compSliderBaseRect.getX(),
                                  compSliderBaseRect.getY(),
                                  compSliderBaseRect.getWidth(),
                                  compSliderBaseRect.getHeight(),
                                  juce::RectanglePlacement::stretchToFit);
            }
        }
    }
}

void EasyRecAudioProcessorEditor::resized()
{
    compKnob.setBounds(344, 194, 37, 37);
    lowKnob.setBounds(419, 116, 37, 37);
    toneKnob.setBounds(490, 116, 37, 37);
    satKnob.setBounds(344, 282, 37, 37);
    outKnob.setBounds(489, 274, 38, 37);

    // Versione B: layout alternativo
    if (showAltBackground)
    {
        lowKnob.setBounds(443, 308, 37, 37);  // +1px dx
        toneKnob.setBounds(496, 308, 37, 37); // -1px sx
        outKnob.setBounds(377, 308, 38, 37);  // 1px a sinistra

        // Slider orizzontali per Compressor e Saturator
        compKnob.setSliderStyle(juce::Slider::LinearHorizontal);
        compKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        compKnob.setLookAndFeel(nullptr);
        compKnob.setBounds(437, 200, 100, 60); // +5px su
        compKnob.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("ff445E1A"));
        compKnob.setColour(juce::Slider::trackColourId, juce::Colour::fromString("ff82A942"));
        compKnob.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("ff82A942"));

        satKnob.setSliderStyle(juce::Slider::LinearHorizontal);
        satKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        satKnob.setLookAndFeel(nullptr);
        satKnob.setBounds(290, 89, 100, 60);  // +5px su
        satKnob.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("ff445E1A"));
        satKnob.setColour(juce::Slider::trackColourId, juce::Colour::fromString("ff82A942"));
        satKnob.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("ff82A942"));

        // buttonSlider.png subito sotto agli slider
        const int baseH = 12;
        satSliderBaseRect = { satKnob.getX() - 5, satKnob.getBottom() - 36, satKnob.getWidth(), baseH };
        compSliderBaseRect = { compKnob.getX() - 5, compKnob.getBottom() - 36, compKnob.getWidth(), baseH };

        // On/Off buttons
        satOnOffButton.setBounds(satKnob.getX() + 50, satKnob.getY() + 10, 15, 15); // -1px su
        compOnOffButton.setBounds(compKnob.getX() + 60, compKnob.getY() + 8, 15, 15); // -2px su
        eqOnOffButton.setBounds(toneKnob.getX() + 25, toneKnob.getY() - 22, 15, 15);

        // eqon.png centrato sul low knob (scalato 2x)
        const float scale = 2.7f;
        const int eqBaseW = lowKnob.getWidth();
        const int eqBaseH = lowKnob.getHeight();
        const int newW = (int)std::round(eqBaseW * scale);
        const int newH = (int)std::round(eqBaseH * scale);
        eqOnRect = { lowKnob.getX() + (eqBaseW - newW) / 2 + 26,
                     lowKnob.getY() + (eqBaseH - newH) / 2 + 2,
                     newW, newH };

        // Aree cliccabili Soft/Hard sui personaggi
        saturToggleButton.setBounds(418, 98, 90, 90);
        toggleCompButton.setBounds(265, 197, 80, 80);

        // Rettangoli per satB/compB (stessa posizione dei bottoni)
        satBRect = saturToggleButton.getBounds();
        compBRect = toggleCompButton.getBounds();

        // Visibilità in base a ON/OFF
        const bool satOn = satOnOffButton.getToggleState();
        const bool compOn = compOnOffButton.getToggleState();
        const bool eqOn = eqOnOffButton.getToggleState();
        satKnob.setVisible(satOn);
        satLabel.setVisible(satOn);
        compKnob.setVisible(compOn);
        compLabel.setVisible(compOn);
        lowKnob.setVisible(eqOn);
        toneKnob.setVisible(eqOn);
        lowLabelValue.setVisible(eqOn);
        toneLabelValue.setVisible(eqOn);
        satOnOffButton.setVisible(true);
        compOnOffButton.setVisible(true);
        eqOnOffButton.setVisible(true);
    }
    else
    {
        // Ripristina stile knob in interfaccia A
        compKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        compKnob.setLookAndFeel(&compKnobLookAndFeel);
        satKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        satKnob.setLookAndFeel(&satKnobLookAndFeel);

        satOnOffButton.setVisible(false);
        compOnOffButton.setVisible(false);
        eqOnOffButton.setVisible(false);
    }
    
    compLabel.setBounds(compKnob.getBounds().translated(1, 0));
    //lowLabelDescription.setBounds(lowKnob.getBounds().translated(1, 0));
    lowLabelValue.setBounds(lowKnob.getBounds().translated(1, 0));
    //toneLabelDescription.setBounds(toneKnob.getBounds().translated(1, 0));
    toneLabelValue.setBounds(toneKnob.getBounds().translated(1, 0));
    satLabel.setBounds(satKnob.getBounds().translated(1, 0));
    outLabel.setBounds(outKnob.getBounds().translated(1, 0));
    
    if (!showAltBackground)
    {
        toggleCompButton.setBounds(240, 195, 100, 30);
        saturToggleButton.setBounds(240, 283, 100, 30);
    }

    // D-pad RIGHT area (adjust if needed)
    dpadRightButton.setBounds(324, 427, 60, 80);

    if (showAltBackground)
    {
        satLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("ff2D2933"));
        compLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("ff2D2933"));
        satLabel.setBounds(satKnob.getX() + 25, satKnob.getBottom() - 25, satKnob.getWidth(), 16);
        compLabel.setBounds(compKnob.getX() + 25, compKnob.getBottom() - 25, compKnob.getWidth(), 16);
    }
    
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

    // Micro-movimento in loop
    spritePhase += spriteSpeed;
    if (spritePhase > juce::MathConstants<float>::twoPi)
        spritePhase -= juce::MathConstants<float>::twoPi;

    if (compAnimating)
        compAnimating = animate(currentCompHighlightRect, isSoftMode ? softHighlightArea.toFloat() : hardHighlightArea.toFloat());

    if (saturAnimating)
        saturAnimating = animate(currentSaturHighlightRect, isSoftSaturMode ? softSatHighlightArea.toFloat() : hardSatHighlightArea.toFloat());

    stillAnimating = compAnimating || saturAnimating;

    if (showAltBackground)
    {
        auto toOutOfTen = [](double v) -> int
        {
            return (int)juce::jlimit(0.0, 10.0, std::round(v * 10.0));
        };

        const int compVal = toOutOfTen(compKnob.getValue());
        const int satVal = toOutOfTen(satKnob.getValue());

        auto formatOutOfTen = [](int v) -> juce::String
        {
            if (v <= 0) return "min/10";
            if (v >= 10) return "max/10";
            return juce::String(v) + "/10";
        };

        compLabel.setText(formatOutOfTen(compVal), juce::dontSendNotification);
        satLabel.setText(formatOutOfTen(satVal), juce::dontSendNotification);
    }

    repaint();
}

void EasyRecAudioProcessorEditor::updateEQ()
{
    float lowFreq = (float)lowKnob.getValue();
    float toneVal = (float)toneKnob.getValue();

    audioProcessor.updateEQFilters(lowFreq, toneVal);
}
