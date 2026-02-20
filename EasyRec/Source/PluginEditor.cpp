#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    auto font = juce::Font(
        juce::FontOptions()
            .withName("")
            .withStyle("")
            .withTypeface(earlyGameBoyFont)
            .withHeight(9.0f));
    font.setBold(true);

    setSize (825, 660);

    // Timer per animazioni (micro-movimenti + intro)
    startTimerHz(60);
    
    // Background
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::Gameboy2_prova_png, BinaryData::Gameboy2_prova_pngSize);
    backgroundImageB = juce::ImageCache::getFromMemory(BinaryData::Gameboy_2_png, BinaryData::Gameboy_2_pngSize);
    catImage = juce::ImageCache::getFromMemory(BinaryData::cat_png, BinaryData::cat_pngSize);
    buttonSliderImage = juce::ImageCache::getFromMemory(BinaryData::buttonSlider_png, BinaryData::buttonSlider_pngSize);
    eqOnImage = juce::ImageCache::getFromMemory(BinaryData::bassknob_png, BinaryData::bassknob_pngSize);
    introImage = juce::ImageCache::getFromMemory(BinaryData::Gameboy_intro_png, BinaryData::Gameboy_intro_pngSize);
    introGamevoiceDrawable = juce::Drawable::createFromImageData(BinaryData::gamevoice_svg, BinaryData::gamevoice_svgSize);
    introNomiDrawable = juce::Drawable::createFromImageData(BinaryData::nomi_svg, BinaryData::nomi_svgSize);

    // Toggle schermata A/B (visibile per posizionamento)
    screenToggleButton.setButtonText("");
    screenToggleButton.setClickingTogglesState(false);
    screenToggleButton.setAlpha(0.0f);
    screenToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    screenToggleButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    screenToggleButton.onClick = [this]()
    {
        isScreenB = !isScreenB;
        resized();
        repaint();
    };
    addAndMakeVisible(screenToggleButton);

    screenToggleButtonLeft.setButtonText("");
    screenToggleButtonLeft.setClickingTogglesState(false);
    screenToggleButtonLeft.setAlpha(0.0f);
    screenToggleButtonLeft.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    screenToggleButtonLeft.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    screenToggleButtonLeft.onClick = [this]()
    {
        isScreenB = !isScreenB;
        resized();
        repaint();
    };
    addAndMakeVisible(screenToggleButtonLeft);

    // Bottone di ritorno (solo schermata 2)
    screenBackButton.setButtonText("");
    screenBackButton.setClickingTogglesState(false);
    screenBackButton.setAlpha(0.0f);
    screenBackButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    screenBackButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    screenBackButton.onClick = [this]()
    {
        isScreenB = false;
        resized();
        repaint();
    };
    addAndMakeVisible(screenBackButton);

    auto setupScreen2Slider = [this](juce::Slider& s)
    {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setRange(0.0, 1.0, 0.01);
        s.setValue(0.5);
        s.setLookAndFeel(nullptr);
        s.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("ff445E1A"));
        s.setColour(juce::Slider::trackColourId, juce::Colour::fromString("ff82A942"));
        s.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("ff82A942"));
        addAndMakeVisible(s);
    };

    auto setupScreen2Label = [this, font](juce::Label& l)
    {
        l.setFont(font);
        l.setColour(juce::Label::textColourId, juce::Colour::fromString("ff2D2933"));
        l.setJustificationType(juce::Justification::centredRight);
        l.setEditable(false, false, false);
        l.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(l);
    };

    setupScreen2Slider(roomKnob);
    setupScreen2Slider(churchKnob);
    setupScreen2Slider(slapKnob);
    setupScreen2Slider(eighthKnob);
    setupScreen2Label(roomLabel);
    setupScreen2Label(churchLabel);
    setupScreen2Label(slapLabel);
    setupScreen2Label(eighthLabel);
    roomLabel.setJustificationType(juce::Justification::centredLeft);
    churchLabel.setJustificationType(juce::Justification::centredLeft);
    slapLabel.setJustificationType(juce::Justification::centredRight);
    eighthLabel.setJustificationType(juce::Justification::centredRight);

    auto setupScreen2OnOff = [this](juce::TextButton& b)
    {
        b.setButtonText("");
        b.setClickingTogglesState(true);
        b.setAlpha(0.0f);
        b.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::textColourOnId, juce::Colours::transparentBlack);
        addAndMakeVisible(b);
    };

    setupScreen2OnOff(roomOnOffButton);
    setupScreen2OnOff(churchOnOffButton);
    setupScreen2OnOff(slapOnOffButton);
    setupScreen2OnOff(eighthOnOffButton);
    roomOnOffButton.setToggleState(false, juce::dontSendNotification);
    churchOnOffButton.setToggleState(false, juce::dontSendNotification);
    slapOnOffButton.setToggleState(false, juce::dontSendNotification);
    eighthOnOffButton.setToggleState(false, juce::dontSendNotification);

    // === COMP SLIDER ===
    compKnob.setSliderStyle(juce::Slider::LinearHorizontal);
    compKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    compKnob.setRange(0.0, 1.0, 0.01);
    compKnob.setValue(0.5);
    compKnob.setLookAndFeel(nullptr);
    addAndMakeVisible(compKnob);

    // === EQ KNOBS ===
    
    // === LOW KNOB ===
    lowKnobDrawable = juce::Drawable::createFromImageData(BinaryData::LowEq_Knob_svg, BinaryData::LowEq_Knob_svgSize);
    lowKnobLookAndFeel.knobImage = lowKnobDrawable.get();
    lowKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lowKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lowKnob.setRange(0.0, 1.0, 0.01);
    lowKnob.setValue(0.5);
    lowKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    lowKnob.setLookAndFeel(&lowKnobLookAndFeel);
    addAndMakeVisible(lowKnob);

    lowLabelDescription.setText("Low Cut", juce::dontSendNotification);
    lowLabelDescription.setFont(font);
    lowLabelDescription.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
    //addAndMakeVisible(lowLabelDescription);

    lowLabelValue.setFont(font);
    lowLabelValue.setColour(juce::Label::textColourId, juce::Colours::black);
    lowLabelValue.setJustificationType(juce::Justification::centred);
    lowLabelValue.setEditable(false, false, false);
    lowLabelValue.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(lowLabelValue);

    lowLabelValue.setText(formatValue((lowKnob.getValue() - 0.5f) * 20.0f), juce::dontSendNotification);

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


    // === SATURATION SLIDER ===
    satKnob.setSliderStyle(juce::Slider::LinearHorizontal);
    satKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    satKnob.setRange(0.0, 1.0, 0.01);
    satKnob.setValue(0.5);
    satKnob.setLookAndFeel(nullptr);
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
    setupLabel(lowLabelValue, lowKnob);
    setupLabel(toneLabelValue, toneKnob);
    setupLabel(satLabel, satKnob);
    setupLabel(outLabel, outKnob);


    addListener(compKnob, compLabel, formatValue, FormatterType::FormatValue);
    addListener(lowKnob, lowLabelValue, formatValue, FormatterType::FormatValue);
    addListener(toneKnob, toneLabelValue, formatValue, FormatterType::FormatValue);
    addListener(satKnob, satLabel, formatValue, FormatterType::FormatValue);
    addListener(outKnob, outLabel, formatValue, FormatterType::FormatValue);

    // Label slider comp/treble in scala -10..+10 con suffisso /10.
    auto updateBLabels = [this, formatMinMax]()
    {
        const float compDb = ((float) compKnob.getValue() - 0.5f) * 20.0f;
        const float trebleDb = ((float) satKnob.getValue() - 0.5f) * 20.0f;

        auto withSuffix = [formatMinMax](float valueDb) -> juce::String
        {
            auto base = formatMinMax(valueDb);
            if (base == "min")
                return "-10/10";
            if (base == "max")
                return "10/10";
            return base + "/10";
        };

        compLabel.setText(withSuffix(compDb), juce::dontSendNotification);
        satLabel.setText(withSuffix(trebleDb), juce::dontSendNotification);
    };

    compKnob.onValueChange = [updateBLabels]() { updateBLabels(); };
    satKnob.onValueChange = [updateBLabels]() { updateBLabels(); };
    updateBLabels();

    // Low EQ label: minimo -10, massimo max.
    auto updateLowLabel = [this, formatValue]()
    {
        const float lowDb = ((float) lowKnob.getValue() - 0.5f) * 20.0f;
        if (lowDb <= -9.95f)
            lowLabelValue.setText("-10", juce::dontSendNotification);
        else if (lowDb >= 9.95f)
            lowLabelValue.setText("+10", juce::dontSendNotification);
        else
            lowLabelValue.setText(formatValue(lowDb), juce::dontSendNotification);
    };
    lowKnob.onValueChange = [updateLowLabel]() { updateLowLabel(); };
    updateLowLabel();

    // Input label: mostra sempre la scala reale -10..+10 (senza min/max).
    auto updateInputLabel = [this, formatValue]()
    {
        const float inputDb = ((float) toneKnob.getValue() - 0.5f) * 20.0f;
        toneLabelValue.setText(formatValue(inputDb), juce::dontSendNotification);
    };
    toneKnob.onValueChange = [updateInputLabel]() { updateInputLabel(); };
    updateInputLabel();

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

    auto updateScreen2Label = [](juce::Slider& s, juce::Label& l)
    {
        const float value = ((float) s.getValue() - 0.5f) * 20.0f;
        const float rounded = std::round(value);
        if (std::abs(value - rounded) < 0.05f)
            l.setText(juce::String((int) rounded), juce::dontSendNotification);
        else
            l.setText(juce::String(value, 1), juce::dontSendNotification);
    };

    roomKnob.onValueChange = [this, updateScreen2Label]() { updateScreen2Label(roomKnob, roomLabel); };
    churchKnob.onValueChange = [this, updateScreen2Label]() { updateScreen2Label(churchKnob, churchLabel); };
    slapKnob.onValueChange = [this, updateScreen2Label]() { updateScreen2Label(slapKnob, slapLabel); };
    eighthKnob.onValueChange = [this, updateScreen2Label]() { updateScreen2Label(eighthKnob, eighthLabel); };
    updateScreen2Label(roomKnob, roomLabel);
    updateScreen2Label(churchKnob, churchLabel);
    updateScreen2Label(slapKnob, slapLabel);
    updateScreen2Label(eighthKnob, eighthLabel);
    
    toggleCompButton.setAlpha(0.0f);
    toggleCompButton.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
    toggleCompButton.setClickingTogglesState(true);
    toggleCompButton.setToggleState(isSoftMode, juce::dontSendNotification);
    addAndMakeVisible(toggleCompButton);

    toggleCompButton.onClick = [this]()
    {
        isSoftMode = toggleCompButton.getToggleState();
    };
    toggleCompButton.onStateChange = [this]()
    {
        const bool newMode = toggleCompButton.getToggleState();
        if (newMode != isSoftMode)
        {
            isSoftMode = newMode;
        }
    };

    // === TOGGLE SATURAZIONE ===
    saturToggleButton.setAlpha(0.0f);
    saturToggleButton.setClickingTogglesState(true);
    saturToggleButton.setToggleState(isSoftSaturMode, juce::dontSendNotification);
    addAndMakeVisible(saturToggleButton);

    compBDrawable = juce::Drawable::createFromImageData(BinaryData::compB_png, BinaryData::compB_pngSize);
    satBDrawable = juce::Drawable::createFromImageData(BinaryData::satB_png, BinaryData::satB_pngSize);
    compADrawable = juce::Drawable::createFromImageData(BinaryData::compA_png, BinaryData::compA_pngSize);
    satADrawable = juce::Drawable::createFromImageData(BinaryData::satA_png, BinaryData::satA_pngSize);

    saturToggleButton.onClick = [this]()
    {
        isSoftSaturMode = saturToggleButton.getToggleState();
    };
    saturToggleButton.onStateChange = [this]()
    {
        const bool newMode = saturToggleButton.getToggleState();
        if (newMode != isSoftSaturMode)
        {
            isSoftSaturMode = newMode;
        }
    };
    //saturToggleButton.setEnabled(false);


    // On/Off buttons (B)
    satOnOffButton.setClickingTogglesState(true);
    satOnOffButton.setAlpha(0.0f);
    satOnOffButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    satOnOffButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    satOnOffButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(satOnOffButton);

    compOnOffButton.setClickingTogglesState(true);
    compOnOffButton.setAlpha(0.0f);
    compOnOffButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    compOnOffButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    compOnOffButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(compOnOffButton);

    eqOnOffButton.setClickingTogglesState(true);
    eqOnOffButton.setAlpha(0.0f);
    eqOnOffButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    eqOnOffButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    eqOnOffButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(eqOnOffButton);

    // Animation ON/OFF 
    animOnOffButton.setClickingTogglesState(true);
    animOnOffButton.setAlpha(0.0f);
    animOnOffButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    animOnOffButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    animOnOffButton.setToggleState(true, juce::dontSendNotification);
    animOnOffButton.onStateChange = [this]()
    {
        animationsEnabled = animOnOffButton.getToggleState();
    };
    addAndMakeVisible(animOnOffButton);

    // Preset switch + display
    presetSwitchButton.setButtonText("");
    presetSwitchButton.setAlpha(0.0f);
    presetSwitchButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetSwitchButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    presetSwitchButton.onClick = [this]()
    {
        currentPresetIndex = (currentPresetIndex + 1) % 3;
        applyPreset(currentPresetIndex);
        updatePresetLabel();
    };
    addAndMakeVisible(presetSwitchButton);

    presetSwitchBackButton.setButtonText("");
    presetSwitchBackButton.setAlpha(0.0f);
    presetSwitchBackButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetSwitchBackButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    presetSwitchBackButton.onClick = [this]()
    {
        currentPresetIndex = (currentPresetIndex + 2) % 3; // -1 con wrap
        applyPreset(currentPresetIndex);
        updatePresetLabel();
    };
    addAndMakeVisible(presetSwitchBackButton);

    presetNameLabel.setFont(font);
    presetNameLabel.setJustificationType(juce::Justification::centred);
    presetNameLabel.setColour(juce::Label::backgroundColourId, juce::Colour::fromString("ff82A942"));
    presetNameLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("ff445E1A"));
    presetNameLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    presetNameLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(presetNameLabel);

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
    roomAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "room", roomKnob);
    churchAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "church", churchKnob);
    slapAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "slap", slapKnob);
    eighthAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "eighth", eighthKnob);
    roomOnAttachment = std::make_unique<APVTS::ButtonAttachment>(apvts, "roomOn", roomOnOffButton);
    churchOnAttachment = std::make_unique<APVTS::ButtonAttachment>(apvts, "churchOn", churchOnOffButton);
    slapOnAttachment = std::make_unique<APVTS::ButtonAttachment>(apvts, "slapOn", slapOnOffButton);
    eighthOnAttachment = std::make_unique<APVTS::ButtonAttachment>(apvts, "eighthOn", eighthOnOffButton);

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
    roomOnOffButton.onStateChange = [this]() { resized(); repaint(); };
    churchOnOffButton.onStateChange = [this]() { resized(); repaint(); };
    slapOnOffButton.onStateChange = [this]() { resized(); repaint(); };
    eighthOnOffButton.onStateChange = [this]() { resized(); repaint(); };

    // Evita flash all'apertura: durante l'intro UI parte invisibile (ma bottoni restano cliccabili)
    if (introActive)
    {
        const float uiAlpha = 0.0f;
        satKnob.setAlpha(uiAlpha);
        compKnob.setAlpha(uiAlpha);
        lowKnob.setAlpha(uiAlpha);
        toneKnob.setAlpha(uiAlpha);
        outKnob.setAlpha(uiAlpha);
        satLabel.setAlpha(uiAlpha);
        compLabel.setAlpha(uiAlpha);
        lowLabelValue.setAlpha(uiAlpha);
        toneLabelValue.setAlpha(uiAlpha);
        outLabel.setAlpha(uiAlpha);
    }

    resized();
    updatePresetLabel();
    repaint();

}

EasyRecAudioProcessorEditor::~EasyRecAudioProcessorEditor()
{
    lowKnob.setLookAndFeel(nullptr);
    toneKnob.setLookAndFeel(nullptr);
    outKnob.setLookAndFeel(nullptr);
}

//==============================================================================
void EasyRecAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    const auto& currentBg = isScreenB ? backgroundImageB : backgroundImage;
    if (currentBg.isValid())
        g.drawImage(currentBg, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);

    if (isScreenB)
    {
        g.setColour(juce::Colour::fromString("ff445E1A"));
        g.fillRect(254, 345, 265, 2);
    }

    const auto drawMeterBar = [this, &g](juce::Rectangle<float> meterBounds, float meterDb, bool withNumbers)
    {
        const auto dbToX = [&meterBounds](float db)
        {
            const float linearNorm = juce::jlimit(0.0f, 1.0f, juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f));
            constexpr float meterCurve = 2.2f;
            const float n = std::pow(linearNorm, meterCurve);
            return meterBounds.getX() + meterBounds.getWidth() * n;
        };

        g.setColour(juce::Colour::fromString("662A3A18"));
        g.fillRoundedRectangle(meterBounds, 2.0f);

        // Segmenti orizzontali separati tra un numero e l'altro.
        static constexpr std::array<float, 7> meterRanges { -60.0f, -40.0f, -20.0f, -10.0f, -6.0f, -3.0f, 0.0f };
        constexpr float segGap = 2.0f;
        for (size_t i = 0; i + 1 < meterRanges.size(); ++i)
        {
            const float lowerDb = meterRanges[i];
            const float upperDb = meterRanges[i + 1];
            const float xL = dbToX(lowerDb);
            const float xR = dbToX(upperDb);
            const auto seg = juce::Rectangle<float>(xL + segGap * 0.5f,
                                                    meterBounds.getY(),
                                                    juce::jmax(1.0f, (xR - xL) - segGap),
                                                    meterBounds.getHeight());

            const bool isClipSeg = (upperDb > -3.0f);
            const float segmentNorm = juce::jlimit(0.0f, 1.0f, (meterDb - lowerDb) / (upperDb - lowerDb));

            // Segmento spento (base).
            g.setColour(isClipSeg ? juce::Colour::fromString("663B1A1A")
                                  : juce::Colour::fromString("662A3A18"));
            g.fillRoundedRectangle(seg, 1.8f);

            // Segmento acceso in modo continuo (anche valori intermedi, es. -5 dB).
            if (segmentNorm > 0.0f)
            {
                const auto lit = seg.withWidth(juce::jmax(1.0f, seg.getWidth() * segmentNorm));
                g.setColour(isClipSeg ? juce::Colour::fromString("ffD94A4A")
                                      : juce::Colour::fromString("ffA8D34A"));
                g.fillRoundedRectangle(lit, 1.8f);
            }
        }

        if (withNumbers)
        {
            const juce::Font meterFont(
                juce::FontOptions()
                    .withName("")
                    .withStyle("")
                    .withTypeface(earlyGameBoyFont)
                    .withHeight(8.0f));
            g.setFont(meterFont);

            static constexpr std::array<float, 6> meterTicks { 0.0f, -3.0f, -6.0f, -10.0f, -20.0f, -40.0f };
            for (float db : meterTicks)
            {
                const float x = dbToX(db);
                g.setColour(juce::Colour::fromString("ff445E1A"));
                const juce::String label = (db == 0.0f) ? "0" : "-" + juce::String((int) std::abs(db));
                g.drawText(label, juce::Rectangle<int>((int) x - 16, (int) meterBounds.getY() - 13, 32, 12),
                           juce::Justification::centred);
            }
        }
    };

    const float meterAlpha = introActive ? juce::jlimit(0.0f, 1.0f, introImageFade) : 1.0f;
    if (meterAlpha > 0.001f)
    {
        juce::Graphics::ScopedSaveState meterState(g);
        g.setOpacity(meterAlpha);

        const auto inputBounds = juce::Rectangle<float>(240.0f, 356.0f, 250.0f, 12.0f);
        const auto outputBounds = juce::Rectangle<float>(240.0f, 382.0f, 250.0f, 12.0f);
        drawMeterBar(outputBounds, audioProcessor.getOutputMeterDb(), true);
        drawMeterBar(inputBounds, audioProcessor.getInputMeterDb(), false);

        const juce::Font meterNameFont(
            juce::FontOptions()
                .withName("")
                .withStyle("")
                .withTypeface(earlyGameBoyFont)
                .withHeight(9.0f));
        g.setFont(meterNameFont);
        g.setColour(juce::Colour::fromString("ff445E1A"));
        g.drawText("output", juce::Rectangle<int>((int) outputBounds.getRight() + 5, (int) outputBounds.getY() - 4, 60, 20),
                   juce::Justification::centredLeft);
        g.drawText("input", juce::Rectangle<int>((int) inputBounds.getRight() + 5, (int) inputBounds.getY() - 4, 60, 20),
                   juce::Justification::centredLeft);
    }

    if (isScreenB && catImage.isValid())
    {
        // Stessa animazione di compA (fase + halfPi, stessa ampiezza).
        const int catOffset = (int) std::round(std::sin(spritePhase + juce::MathConstants<float>::halfPi) * spriteAmplitudePx);
        const auto drawRect = catRect.translated(0, catOffset);
        g.drawImageWithin(catImage,
                          drawRect.getX(),
                          drawRect.getY(),
                          drawRect.getWidth(),
                          drawRect.getHeight(),
                          juce::RectanglePlacement::centred);
    }

    if (isScreenB && buttonSliderImage.isValid())
    {
        const auto isFxOn = [this](const char* paramID)
        {
            if (auto* v = audioProcessor.getAPVTS().getRawParameterValue(paramID))
                return v->load() >= 0.5f;
            return false;
        };

        auto drawSliderBase = [this, &g](const juce::Slider& s)
        {
            g.drawImageWithin(buttonSliderImage,
                              s.getX() - 5,
                              s.getY() + 29,
                              s.getWidth(),
                              12,
                              juce::RectanglePlacement::stretchToFit);
        };

        if (isFxOn("roomOn")) drawSliderBase(roomKnob);
        if (isFxOn("churchOn")) drawSliderBase(churchKnob);
        if (isFxOn("slapOn")) drawSliderBase(slapKnob);
        if (isFxOn("eighthOn")) drawSliderBase(eighthKnob);
    }

    if (!isScreenB)
    {
    // Soft/Hard highlight drawables disabled

    // Indicatori hard (satB/compB)
        const int satOffset = (int)std::round(std::sin(spritePhase) * spriteAmplitudePx);
        const int compOffset = (int)std::round(std::sin(spritePhase + juce::MathConstants<float>::halfPi) * spriteAmplitudePx);

        auto satRect = satBRect.translated(0, satOffset);
        auto compRect = compBRect.translated(0, compOffset);

        if (satOnOffButton.getToggleState())
        {
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
        }

        if (compOnOffButton.getToggleState())
        {
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
        }

        // bassknob.png visibile quando il controllo low (eqOn) e' attivo.
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

    // Intro overlay (in primo piano; nomi/gamevoice sopra l'intro)
    if (introActive)
    {
        const float t = juce::jlimit(0.0f, 1.0f, introProgress);
        // Dissolvenza: 0.9s effettivi
        const float fadeT = juce::jlimit(0.0f, 1.0f, t / 1.0f);
        const float alpha = 1.0f - fadeT; // dissolve

        if (introImage.isValid())
        {
            const float imageAlpha = 1.0f - juce::jlimit(0.0f, 1.0f, introImageFade);
            juce::Graphics::ScopedSaveState state(g);
            g.setOpacity(imageAlpha);
            g.drawImage(introImage, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
        }

        if (introGamevoiceDrawable)
        {
            const float gwScale = (1.0f / 6.0f) * 1.7f;
            auto bounds = getLocalBounds().toFloat().translated(-25.0f, -90.0f);
            auto scaled = bounds.withSizeKeepingCentre(bounds.getWidth() * gwScale, bounds.getHeight() * gwScale);
            introGamevoiceDrawable->drawWithin(g, scaled, juce::RectanglePlacement::centred, alpha);
        }

        if (introNomiDrawable)
        {
            const float nomiScale = (1.0f / 6.0f) * 1.2f;
            auto bounds = getLocalBounds().toFloat().translated(-25.0f, -55.0f);
            auto scaled = bounds.withSizeKeepingCentre(bounds.getWidth() * nomiScale, bounds.getHeight() * nomiScale);
            introNomiDrawable->drawWithin(g, scaled, juce::RectanglePlacement::centred, alpha);
        }

        // Intro: meter nascosto.
    }
}

void EasyRecAudioProcessorEditor::resized()
{
    const auto isFxOn = [this](const char* paramID)
    {
        if (auto* v = audioProcessor.getAPVTS().getRawParameterValue(paramID))
            return v->load() >= 0.5f;
        return false;
    };

    // EQ + Output
    lowKnob.setBounds(371, 309, 38, 38);
    toneKnob.setBounds(435, 309, 38, 38);
    outKnob.setBounds(496, 309, 38, 38);
    screenToggleButton.setBounds(320, 490, 50, 40);
    screenToggleButtonLeft.setBounds(248, 492, 50, 40);
    screenBackButton.setBounds(450, 285, 80, 50);
    catRect = { 326, 104, 120, 120 };
    roomKnob.setBounds(250, 95, 80, 70);
    churchKnob.setBounds(250, 185, 80, 70);
    slapKnob.setBounds(455, 95, 80, 70);
    eighthKnob.setBounds(455, 185, 80, 70);
    roomOnOffButton.setBounds(roomKnob.getX() + 105, roomKnob.getY(), 15, 15);
    churchOnOffButton.setBounds(churchKnob.getX() + 105, churchKnob.getY() + 58, 15, 15);
    slapOnOffButton.setBounds(slapKnob.getX() - 49, slapKnob.getY(), 15, 15);
    eighthOnOffButton.setBounds(eighthKnob.getX() - 49, eighthKnob.getY() + 58, 15, 15);
    roomLabel.setBounds(roomKnob.getX() - 3, roomKnob.getY() + 37, 90, 24);
    churchLabel.setBounds(churchKnob.getX() - 3, churchKnob.getY() + 9, 90, 24);
    slapLabel.setBounds(slapKnob.getX() - 18, slapKnob.getY() + 37, 90, 24);
    eighthLabel.setBounds(eighthKnob.getX() - 18, eighthKnob.getY() + 9, 90, 24);

    // Slider orizzontali per Compressor e Saturator
    compKnob.setSliderStyle(juce::Slider::LinearHorizontal);
    compKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    compKnob.setLookAndFeel(nullptr);
    compKnob.setBounds(435, 200, 100, 60);
    compKnob.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("ff445E1A"));
    compKnob.setColour(juce::Slider::trackColourId, juce::Colour::fromString("ff82A942"));
    compKnob.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("ff82A942"));

    satKnob.setSliderStyle(juce::Slider::LinearHorizontal);
    satKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    satKnob.setLookAndFeel(nullptr);
    satKnob.setBounds(288, 89, 100, 60);
    satKnob.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("ff445E1A"));
    satKnob.setColour(juce::Slider::trackColourId, juce::Colour::fromString("ff82A942"));
    satKnob.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("ff82A942"));

    // buttonSlider.png subito sotto agli slider
    const int baseH = 12;
    satSliderBaseRect = { satKnob.getX() - 5, satKnob.getBottom() - 36, satKnob.getWidth(), baseH };
    compSliderBaseRect = { compKnob.getX() - 5, compKnob.getBottom() - 36, compKnob.getWidth(), baseH };

    // On/Off buttons
    satOnOffButton.setBounds(satKnob.getX() + 57, satKnob.getY() + 6, 15, 15);
    compOnOffButton.setBounds(compKnob.getX() + 49, compKnob.getY() + 6, 15, 15);
    eqOnOffButton.setBounds(toneKnob.getX() - 35, toneKnob.getY() - 21, 15, 15);
    animOnOffButton.setBounds(505, 448, 52, 52);
    presetSwitchButton.setBounds(286, 446, 39, 46);
    presetSwitchBackButton.setBounds(286, 525, 39, 46);
    presetNameLabel.setBounds(340, 412, 100, 12);

    // bassknob.png centrato sul low knob
    const float scale = 1.25f;
    const int eqBaseW = lowKnob.getWidth();
    const int eqBaseH = lowKnob.getHeight();
    const int newW = (int)std::round(eqBaseW * scale);
    const int newH = (int)std::round(eqBaseH * scale);
    eqOnRect = { lowKnob.getX() + (eqBaseW - newW) / 2,
                 lowKnob.getY() + (eqBaseH - newH) / 2,
                 newW, newH };
    
    // Aree cliccabili Soft/Hard sui personaggi
    saturToggleButton.setBounds(418, 98, 90, 90);
    toggleCompButton.setBounds(265, 197, 80, 80);
    satBRect = saturToggleButton.getBounds();
    compBRect = toggleCompButton.getBounds();

    const bool showUi = !isScreenB;
    const bool lowOn = eqOnOffButton.getToggleState();
    const bool compOn = compOnOffButton.getToggleState();
    const bool trebleOn = satOnOffButton.getToggleState();

    satKnob.setVisible(showUi && trebleOn);
    satLabel.setVisible(showUi && trebleOn);
    compKnob.setVisible(showUi && compOn);
    compLabel.setVisible(showUi && compOn);
    lowKnob.setVisible(showUi && lowOn);
    toneKnob.setVisible(showUi);
    lowLabelValue.setVisible(showUi && lowOn);
    toneLabelValue.setVisible(showUi);
    outKnob.setVisible(showUi);
    outLabel.setVisible(showUi);
    satOnOffButton.setVisible(showUi);
    compOnOffButton.setVisible(showUi);
    eqOnOffButton.setVisible(showUi);
    toggleCompButton.setVisible(showUi);
    saturToggleButton.setVisible(showUi);
    animOnOffButton.setVisible(true);
    presetSwitchButton.setVisible(true);
    presetSwitchBackButton.setVisible(true);
    presetNameLabel.setVisible(true);
    screenToggleButton.setVisible(true);
    screenToggleButtonLeft.setVisible(true);
    screenBackButton.setVisible(isScreenB);
    roomKnob.setVisible(isScreenB && isFxOn("roomOn"));
    churchKnob.setVisible(isScreenB && isFxOn("churchOn"));
    slapKnob.setVisible(isScreenB && isFxOn("slapOn"));
    eighthKnob.setVisible(isScreenB && isFxOn("eighthOn"));
    roomLabel.setVisible(isScreenB && isFxOn("roomOn"));
    churchLabel.setVisible(isScreenB && isFxOn("churchOn"));
    slapLabel.setVisible(isScreenB && isFxOn("slapOn"));
    eighthLabel.setVisible(isScreenB && isFxOn("eighthOn"));
    roomOnOffButton.setVisible(isScreenB);
    churchOnOffButton.setVisible(isScreenB);
    slapOnOffButton.setVisible(isScreenB);
    eighthOnOffButton.setVisible(isScreenB);

    // Label placement
    lowLabelValue.setBounds(lowKnob.getBounds().translated(1, 0));
    toneLabelValue.setBounds(toneKnob.getBounds().translated(1, 0));
    outLabel.setBounds(outKnob.getBounds().translated(1, 0));
    satLabel.setBounds(satKnob.getX() + 25, satKnob.getBottom() - 24, satKnob.getWidth(), 16);
    compLabel.setBounds(compKnob.getX() + 25, compKnob.getBottom() - 24, compKnob.getWidth(), 16);
    satLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("ff2D2933"));
    compLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("ff2D2933"));
}



void EasyRecAudioProcessorEditor::timerCallback()
{
    // Micro-movimento in loop
    if (animationsEnabled)
    {
        spritePhase += spriteSpeed;
        if (spritePhase > juce::MathConstants<float>::twoPi)
            spritePhase -= juce::MathConstants<float>::twoPi;
    }

    // Intro progress (0.5s)
    if (introActive && !introPaused)
    {
        if (introDelayFrames > 0)
        {
            --introDelayFrames;
        }
        else if (introProgress < 1.0f)
        {
            constexpr float introSeconds = 0.7f;
            introProgress += (1.0f / (introSeconds * 60.0f)); // 0.7 secondi a 60fps
            if (introProgress >= 1.0f)
                introProgress = 1.0f;
        }
        else
        {
            // attende un attimo dopo la dissolvenza di nomi/gamevoice
            if (introHoldFrames > 0)
            {
                --introHoldFrames;
            }
            else if (introImageFade < 1.0f)
            {
                constexpr float imageFadeSeconds = 0.5f;
                introImageFade += (1.0f / (imageFadeSeconds * 60.0f));
                if (introImageFade >= 1.0f)
                    introImageFade = 1.0f;
            }
            else
            {
                introActive = false;
                resized();
            }
        }
    }

    // Fade-in UI durante la dissolvenza di gameboy_intro
    {
        const float uiAlpha = introActive ? juce::jlimit(0.0f, 1.0f, introImageFade) : 1.0f;
        satKnob.setAlpha(uiAlpha);
        compKnob.setAlpha(uiAlpha);
        lowKnob.setAlpha(uiAlpha);
        toneKnob.setAlpha(uiAlpha);
        outKnob.setAlpha(uiAlpha);
        satLabel.setAlpha(uiAlpha);
        compLabel.setAlpha(uiAlpha);
        lowLabelValue.setAlpha(uiAlpha);
        toneLabelValue.setAlpha(uiAlpha);
        outLabel.setAlpha(uiAlpha);
        presetNameLabel.setColour(juce::Label::textColourId,
                                  introActive ? juce::Colours::transparentBlack
                                              : juce::Colour::fromString("ff445E1A"));
        // On/off buttons restano invisibili
    }

    {
        auto fmt = [](float valueDb) -> juce::String
        {
            if (valueDb <= -9.95f) return "-10/10";
            if (valueDb >= 9.95f) return "10/10";
            juce::String v = (std::floor(valueDb) == valueDb) ? juce::String((int) valueDb)
                                                               : juce::String(valueDb, 1);
            return v + "/10";
        };

        const float compDb = ((float) compKnob.getValue() - 0.5f) * 20.0f;
        const float trebleDb = ((float) satKnob.getValue() - 0.5f) * 20.0f;
        compLabel.setText(fmt(compDb), juce::dontSendNotification);
        satLabel.setText(fmt(trebleDb), juce::dontSendNotification);
    }

    const bool nowDirty = !isCurrentStateMatchingPreset(currentPresetIndex);
    if (nowDirty != presetDirty)
    {
        presetDirty = nowDirty;
        updatePresetLabel();
    }

    repaint();
}

void EasyRecAudioProcessorEditor::updateEQ()
{
    const float bassDb = ((float) lowKnob.getValue() - 0.5f) * 20.0f;
    const float trebleDb = ((float) satKnob.getValue() - 0.5f) * 20.0f;

    audioProcessor.updateEQFilters(bassDb, trebleDb);
}

void EasyRecAudioProcessorEditor::applyPreset (int presetIndex)
{
    const auto p = getPresetValues(presetIndex);
    auto& apvts = audioProcessor.getAPVTS();

    auto setParam = [&apvts](const char* id, float norm)
    {
        if (auto* param = apvts.getParameter(id))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost(norm);
            param->endChangeGesture();
        }
    };

    auto setBool = [&setParam](const char* id, bool v) { setParam(id, v ? 1.0f : 0.0f); };

    setParam("tone", p.tone);
    setParam("comp", p.comp);
    setParam("lowCut", p.lowCut);
    setParam("satur", p.satur);
    setParam("out", p.out);

    setBool("eqOn", p.eqOn);
    setBool("compOn", p.compOn);
    setBool("satOn", p.satOn);
    setBool("compSoft", p.compSoft);
    setBool("satSoft", p.satSoft);

    setParam("room", p.room);
    setParam("church", p.church);
    setParam("slap", p.slap);
    setParam("eighth", p.eighth);

    setBool("roomOn", p.roomOn);
    setBool("churchOn", p.churchOn);
    setBool("slapOn", p.slapOn);
    setBool("eighthOn", p.eighthOn);

    presetDirty = false;
    resized();
    repaint();
}

void EasyRecAudioProcessorEditor::updatePresetLabel()
{
    const auto p = getPresetValues(currentPresetIndex);
    const juce::String suffix = presetDirty ? "*" : "";
    presetNameLabel.setText("\"" + juce::String(p.name) + suffix + "\"", juce::dontSendNotification);
}

EasyRecAudioProcessorEditor::PresetValues EasyRecAudioProcessorEditor::getPresetValues (int presetIndex) const
{
    static constexpr std::array<PresetValues, 3> presets
    {{
        { "Starter",      0.50f, 0.50f, 0.50f, 0.50f, 6.0f/9.0f, true, true, true,  true,  true,  0.50f, 0.50f, 0.50f, 0.50f, false, false, false, false },
        { "Capopalestra", 0.58f, 0.60f, 0.54f, 0.62f, 6.2f/9.0f, true, true, true,  false, false, 0.52f, 0.46f, 0.58f, 0.54f, true,  false, true,  true  },
        { "Campione",     0.64f, 0.68f, 0.57f, 0.66f, 6.4f/9.0f, true, true, true,  false, false, 0.56f, 0.50f, 0.62f, 0.58f, true,  true,  true,  true  }
    }};

    const int idx = juce::jlimit(0, (int) presets.size() - 1, presetIndex);
    return presets[(size_t) idx];
}

bool EasyRecAudioProcessorEditor::isCurrentStateMatchingPreset (int presetIndex) const
{
    const auto p = getPresetValues(presetIndex);
    const auto& apvts = audioProcessor.getAPVTS();
    constexpr float tol = 0.0005f;

    auto readFloat = [&apvts](const char* id)
    {
        if (auto* v = apvts.getRawParameterValue(id))
            return v->load();
        return 0.0f;
    };
    auto readBool = [&apvts](const char* id)
    {
        if (auto* v = apvts.getRawParameterValue(id))
            return v->load() >= 0.5f;
        return false;
    };
    auto near = [tol](float a, float b) { return std::abs(a - b) <= tol; };

    return near(readFloat("tone"), p.tone)
        && near(readFloat("comp"), p.comp)
        && near(readFloat("lowCut"), p.lowCut)
        && near(readFloat("satur"), p.satur)
        && near(readFloat("out"), p.out)
        && (readBool("eqOn") == p.eqOn)
        && (readBool("compOn") == p.compOn)
        && (readBool("satOn") == p.satOn)
        && (readBool("compSoft") == p.compSoft)
        && (readBool("satSoft") == p.satSoft)
        && near(readFloat("room"), p.room)
        && near(readFloat("church"), p.church)
        && near(readFloat("slap"), p.slap)
        && near(readFloat("eighth"), p.eighth)
        && (readBool("roomOn") == p.roomOn)
        && (readBool("churchOn") == p.churchOn)
        && (readBool("slapOn") == p.slapOn)
        && (readBool("eighthOn") == p.eighthOn);
}
