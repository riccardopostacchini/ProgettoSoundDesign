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
    bassKnobDrawable = juce::Drawable::createFromImageData(BinaryData::LowEq_Knob_svg, BinaryData::LowEq_Knob_svgSize);
    bassKnobLookAndFeel.knobImage = bassKnobDrawable.get();
    bassKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    bassKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    bassKnob.setRange(0.0, 1.0, 0.01);
    bassKnob.setValue(0.5);
    bassKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    bassKnob.setLookAndFeel(&bassKnobLookAndFeel);
    addAndMakeVisible(bassKnob);

    bassLabelDescription.setText("Low Cut", juce::dontSendNotification);
    bassLabelDescription.setFont(font);
    bassLabelDescription.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
    //addAndMakeVisible(bassLabelDescription);

    bassLabelValue.setFont(font);
    bassLabelValue.setColour(juce::Label::textColourId, juce::Colours::black);
    bassLabelValue.setJustificationType(juce::Justification::centred);
    bassLabelValue.setEditable(false, false, false);
    bassLabelValue.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(bassLabelValue);

    bassLabelValue.setText(formatValue((bassKnob.getValue() - 0.5f) * 20.0f), juce::dontSendNotification);

    // === INPUT KNOB ===
    inputKnobDrawable = juce::Drawable::createFromImageData(BinaryData::ToneEq_Knob_svg, BinaryData::ToneEq_Knob_svgSize);
    inputKnobLookAndFeel.knobImage = inputKnobDrawable.get();
    inputKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    inputKnob.setRange(0.0, 1.0, 0.01);
    inputKnob.setValue(0.5);
    inputKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    inputKnob.setLookAndFeel(&inputKnobLookAndFeel);
    addAndMakeVisible(inputKnob);

    inputLabelDescription.setText("Tone", juce::dontSendNotification);
    inputLabelDescription.setFont(font);
    inputLabelDescription.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
    //addAndMakeVisible(inputLabelDescription);

    inputLabelValue.setFont(font);
    inputLabelValue.setColour(juce::Label::textColourId, juce::Colour::fromString("ff82A942"));
    inputLabelValue.setJustificationType(juce::Justification::centred);
    inputLabelValue.setEditable(false, false, false);
    inputLabelValue.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(inputLabelValue);

    inputLabelValue.setText(formatValue((inputKnob.getValue() - 0.5f) * 20.0f), juce::dontSendNotification);


    // === TREBLE SLIDER ===
    trebleKnob.setSliderStyle(juce::Slider::LinearHorizontal);
    trebleKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    trebleKnob.setRange(0.0, 1.0, 0.01);
    trebleKnob.setValue(0.5);
    trebleKnob.setLookAndFeel(nullptr);
    addAndMakeVisible(trebleKnob);

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
                displayedValue = slider.getValue(); // valore raw del controllo
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
    setupLabel(bassLabelValue, bassKnob);
    setupLabel(inputLabelValue, inputKnob);
    setupLabel(trebleLabel, trebleKnob);
    setupLabel(outLabel, outKnob);


    addListener(compKnob, compLabel, formatValue, FormatterType::FormatValue);
    addListener(bassKnob, bassLabelValue, formatValue, FormatterType::FormatValue);
    addListener(inputKnob, inputLabelValue, formatValue, FormatterType::FormatValue);
    addListener(trebleKnob, trebleLabel, formatValue, FormatterType::FormatValue);
    addListener(outKnob, outLabel, formatValue, FormatterType::FormatValue);

    // Label slider comp/treble in scala -10..+10 con suffisso /10.
    auto updateBLabels = [this, formatMinMax]()
    {
        const float compDb = ((float) compKnob.getValue() - 0.5f) * 20.0f;
        const float trebleDb = ((float) trebleKnob.getValue() - 0.5f) * 20.0f;

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
        trebleLabel.setText(withSuffix(trebleDb), juce::dontSendNotification);
    };

    compKnob.onValueChange = [updateBLabels]() { updateBLabels(); };
    trebleKnob.onValueChange = [updateBLabels]() { updateBLabels(); };
    updateBLabels();

    // Bass EQ label: minimo -10, massimo max.
    auto updateBassLabel = [this, formatValue]()
    {
        const float bassDb = ((float) bassKnob.getValue() - 0.5f) * 20.0f;
        if (bassDb <= -9.95f)
            bassLabelValue.setText("-10", juce::dontSendNotification);
        else if (bassDb >= 9.95f)
            bassLabelValue.setText("+10", juce::dontSendNotification);
        else
            bassLabelValue.setText(formatValue(bassDb), juce::dontSendNotification);
    };
    bassKnob.onValueChange = [updateBassLabel]() { updateBassLabel(); };
    updateBassLabel();

    // Input label: mostra sempre la scala reale -10..+10 (senza min/max).
    auto updateInputLabel = [this, formatValue]()
    {
        const float inputDb = ((float) inputKnob.getValue() - 0.5f) * 20.0f;
        inputLabelValue.setText(formatValue(inputDb), juce::dontSendNotification);
    };
    inputKnob.onValueChange = [updateInputLabel]() { updateInputLabel(); };
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

    // === TOGGLE TREBLE ===
    trebleModeToggleButton.setAlpha(0.0f);
    trebleModeToggleButton.setClickingTogglesState(true);
    trebleModeToggleButton.setToggleState(isSoftTrebleMode, juce::dontSendNotification);
    addAndMakeVisible(trebleModeToggleButton);

    compBDrawable = juce::Drawable::createFromImageData(BinaryData::compB_png, BinaryData::compB_pngSize);
    trebleHardDrawable = juce::Drawable::createFromImageData(BinaryData::satB_png, BinaryData::satB_pngSize);
    compADrawable = juce::Drawable::createFromImageData(BinaryData::compA_png, BinaryData::compA_pngSize);
    trebleSoftDrawable = juce::Drawable::createFromImageData(BinaryData::satA_png, BinaryData::satA_pngSize);

    trebleModeToggleButton.onClick = [this]()
    {
        isSoftTrebleMode = trebleModeToggleButton.getToggleState();
    };
    trebleModeToggleButton.onStateChange = [this]()
    {
        const bool newMode = trebleModeToggleButton.getToggleState();
        if (newMode != isSoftTrebleMode)
        {
            isSoftTrebleMode = newMode;
        }
    };
    //trebleModeToggleButton.setEnabled(false);


    // On/Off buttons (B)
    trebleOnOffButton.setClickingTogglesState(true);
    trebleOnOffButton.setAlpha(0.0f);
    trebleOnOffButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    trebleOnOffButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    trebleOnOffButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(trebleOnOffButton);

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
        currentPresetIndex = (currentPresetIndex + 1) % 5;
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
        currentPresetIndex = (currentPresetIndex + 4) % 5; // -1 con wrap
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
    bassAttachment  = std::make_unique<APVTS::SliderAttachment>(apvts, "lowCut", bassKnob);
    inputAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "tone", inputKnob);
    trebleAttachment  = std::make_unique<APVTS::SliderAttachment>(apvts, "satur", trebleKnob);
    outAttachment  = std::make_unique<APVTS::SliderAttachment>(apvts, "out", outKnob);
    compSoftAttachment = std::make_unique<APVTS::ButtonAttachment>(apvts, "compSoft", toggleCompButton);
    trebleSoftAttachment  = std::make_unique<APVTS::ButtonAttachment>(apvts, "satSoft", trebleModeToggleButton);
    compOnAttachment   = std::make_unique<APVTS::ButtonAttachment>(apvts, "compOn", compOnOffButton);
    trebleOnAttachment    = std::make_unique<APVTS::ButtonAttachment>(apvts, "satOn", trebleOnOffButton);
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
    isSoftTrebleMode = trebleModeToggleButton.getToggleState();

    compOnOffButton.onStateChange = [this]()
    {
        resized();
        repaint();
    };
    trebleOnOffButton.onStateChange = [this]()
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
        trebleKnob.setAlpha(uiAlpha);
        compKnob.setAlpha(uiAlpha);
        bassKnob.setAlpha(uiAlpha);
        inputKnob.setAlpha(uiAlpha);
        outKnob.setAlpha(uiAlpha);
        trebleLabel.setAlpha(uiAlpha);
        compLabel.setAlpha(uiAlpha);
        bassLabelValue.setAlpha(uiAlpha);
        inputLabelValue.setAlpha(uiAlpha);
        outLabel.setAlpha(uiAlpha);
    }

    resized();
    updatePresetLabel();
    repaint();

}

EasyRecAudioProcessorEditor::~EasyRecAudioProcessorEditor()
{
    bassKnob.setLookAndFeel(nullptr);
    inputKnob.setLookAndFeel(nullptr);
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

        auto satRect = trebleModeRect.translated(0, satOffset);
        auto compRect = compBRect.translated(0, compOffset);

        if (trebleOnOffButton.getToggleState())
        {
            if (isSoftTrebleMode)
            {
                if (trebleSoftDrawable)
                    trebleSoftDrawable->drawWithin(g, satRect.toFloat(), juce::RectanglePlacement::centred, 1.0f);
            }
            else
            {
                if (trebleHardDrawable)
                    trebleHardDrawable->drawWithin(g, satRect.toFloat(), juce::RectanglePlacement::centred, 1.0f);
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
            if (trebleOnOffButton.getToggleState())
            {
                g.drawImageWithin(buttonSliderImage,
                                  trebleSliderBaseRect.getX(),
                                  trebleSliderBaseRect.getY(),
                                  trebleSliderBaseRect.getWidth(),
                                  trebleSliderBaseRect.getHeight(),
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
    bassKnob.setBounds(371, 309, 38, 38);
    inputKnob.setBounds(435, 309, 38, 38);
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

    trebleKnob.setSliderStyle(juce::Slider::LinearHorizontal);
    trebleKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    trebleKnob.setLookAndFeel(nullptr);
    trebleKnob.setBounds(288, 89, 100, 60);
    trebleKnob.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("ff445E1A"));
    trebleKnob.setColour(juce::Slider::trackColourId, juce::Colour::fromString("ff82A942"));
    trebleKnob.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("ff82A942"));

    // buttonSlider.png subito sotto agli slider
    const int baseH = 12;
    trebleSliderBaseRect = { trebleKnob.getX() - 5, trebleKnob.getBottom() - 36, trebleKnob.getWidth(), baseH };
    compSliderBaseRect = { compKnob.getX() - 5, compKnob.getBottom() - 36, compKnob.getWidth(), baseH };

    // On/Off buttons
    trebleOnOffButton.setBounds(trebleKnob.getX() + 57, trebleKnob.getY() + 6, 15, 15);
    compOnOffButton.setBounds(compKnob.getX() + 49, compKnob.getY() + 6, 15, 15);
    eqOnOffButton.setBounds(inputKnob.getX() - 35, inputKnob.getY() - 21, 15, 15);
    animOnOffButton.setBounds(505, 448, 52, 52);
    presetSwitchButton.setBounds(286, 446, 39, 46);
    presetSwitchBackButton.setBounds(286, 525, 39, 46);
    presetNameLabel.setBounds(340, 412, 100, 12);

    // bassknob.png centrato sul low knob
    const float scale = 1.25f;
    const int eqBaseW = bassKnob.getWidth();
    const int eqBaseH = bassKnob.getHeight();
    const int newW = (int)std::round(eqBaseW * scale);
    const int newH = (int)std::round(eqBaseH * scale);
    eqOnRect = { bassKnob.getX() + (eqBaseW - newW) / 2,
                 bassKnob.getY() + (eqBaseH - newH) / 2,
                 newW, newH };
    
    // Aree cliccabili Soft/Hard sui personaggi
    trebleModeToggleButton.setBounds(418, 98, 90, 90);
    toggleCompButton.setBounds(265, 197, 80, 80);
    trebleModeRect = trebleModeToggleButton.getBounds();
    compBRect = toggleCompButton.getBounds();

    const bool showUi = !isScreenB;
    const bool bassOn = eqOnOffButton.getToggleState();
    const bool compOn = compOnOffButton.getToggleState();
    const bool trebleOn = trebleOnOffButton.getToggleState();

    trebleKnob.setVisible(showUi && trebleOn);
    trebleLabel.setVisible(showUi && trebleOn);
    compKnob.setVisible(showUi && compOn);
    compLabel.setVisible(showUi && compOn);
    bassKnob.setVisible(showUi && bassOn);
    inputKnob.setVisible(showUi);
    bassLabelValue.setVisible(showUi && bassOn);
    inputLabelValue.setVisible(showUi);
    outKnob.setVisible(showUi);
    outLabel.setVisible(showUi);
    trebleOnOffButton.setVisible(showUi);
    compOnOffButton.setVisible(showUi);
    eqOnOffButton.setVisible(showUi);
    toggleCompButton.setVisible(showUi);
    trebleModeToggleButton.setVisible(showUi);
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
    bassLabelValue.setBounds(bassKnob.getBounds().translated(1, 0));
    inputLabelValue.setBounds(inputKnob.getBounds().translated(1, 0));
    outLabel.setBounds(outKnob.getBounds().translated(1, 0));
    trebleLabel.setBounds(trebleKnob.getX() + 25, trebleKnob.getBottom() - 24, trebleKnob.getWidth(), 16);
    compLabel.setBounds(compKnob.getX() + 25, compKnob.getBottom() - 24, compKnob.getWidth(), 16);
    trebleLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("ff2D2933"));
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
        trebleKnob.setAlpha(uiAlpha);
        compKnob.setAlpha(uiAlpha);
        bassKnob.setAlpha(uiAlpha);
        inputKnob.setAlpha(uiAlpha);
        outKnob.setAlpha(uiAlpha);
        trebleLabel.setAlpha(uiAlpha);
        compLabel.setAlpha(uiAlpha);
        bassLabelValue.setAlpha(uiAlpha);
        inputLabelValue.setAlpha(uiAlpha);
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
        const float trebleDb = ((float) trebleKnob.getValue() - 0.5f) * 20.0f;
        compLabel.setText(fmt(compDb), juce::dontSendNotification);
        trebleLabel.setText(fmt(trebleDb), juce::dontSendNotification);
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
    const float bassDb = ((float) bassKnob.getValue() - 0.5f) * 20.0f;
    const float trebleDb = ((float) trebleKnob.getValue() - 0.5f) * 20.0f;

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

    setParam("comp", p.comp);
    setParam("lowCut", p.eqBass);
    setParam("satur", p.eqTreble);

    setBool("eqOn", p.bassEqOn);
    setBool("compOn", p.compOn);
    setBool("satOn", p.trebleOn);
    setBool("compSoft", p.compSoft);
    setBool("satSoft", p.trebleSoft);

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

EasyRecAudioProcessorEditor::PresetDefinition EasyRecAudioProcessorEditor::getPresetValues (int presetIndex) const
{
    // Riga preset:
    // { "Nome", input, comp, eqBass, eqTreble, output, bassEqOn, compOn, trebleOn, compSoft, trebleSoft, room, church, slap, eighth, roomOn, churchOn, slapOn, eighthOn }
    static constexpr std::array<PresetDefinition, 5> presets
    {{
        { "Starter",      0.5f, 0.50f, 0.50f, 0.50f, 6.0f/9.0f, true, true, true,  true,  true,  0.50f, 0.50f, 0.50f, 0.50f, true, false, true, false },
        { "Evo",      0.5f, 0.8f, 0.4f, 0.9f, 6.0f/9.0f, true, true, true,  false,  false,  0.60f, 0.50f, 0.50f, 0.25f, true, false, false, true },
        { "Capopalestra", 0.5f, 0.65f, 0.35f, 0.65f, 6.2f/9.0f, true, true, true,  true, true, 0.30f, 0.50f, 0.60f, 0.50f, true,  false, true,  false  },
        { "Campione",     0.5f, 0.9f, 0.5f, 0.25f, 6.4f/9.0f, true, true, true,  true, false, 0.50f, 0.30f, 0.5f, 0.5f, false,  true,  true,  false  },
        { "Rivale",     0.5f, 0.85f, 0.35f, 0.8f, 6.4f/9.0f, true, true, true,  false, true, 0.50f, 0.65f, 0.50f, 0.8f, false,  true,  false,  true  }
    }};

    if (presets.empty())
        return PresetDefinition{};

    int idx = presetIndex;
    if (idx < 0)
        idx = 0;

    const int maxIndex = static_cast<int> (presets.size()) - 1;
    if (idx > maxIndex)
        idx = maxIndex;

    return presets[static_cast<size_t> (idx)];
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

    return near(readFloat("comp"), p.comp)
        && near(readFloat("lowCut"), p.eqBass)
        && near(readFloat("satur"), p.eqTreble)
        && (readBool("eqOn") == p.bassEqOn)
        && (readBool("compOn") == p.compOn)
        && (readBool("satOn") == p.trebleOn)
        && (readBool("compSoft") == p.compSoft)
        && (readBool("satSoft") == p.trebleSoft)
        && near(readFloat("room"), p.room)
        && near(readFloat("church"), p.church)
        && near(readFloat("slap"), p.slap)
        && near(readFloat("eighth"), p.eighth)
        && (readBool("roomOn") == p.roomOn)
        && (readBool("churchOn") == p.churchOn)
        && (readBool("slapOn") == p.slapOn)
        && (readBool("eighthOn") == p.eighthOn);
}
