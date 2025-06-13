#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EasyRecAudioProcessorEditor::EasyRecAudioProcessorEditor (EasyRecAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
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
    lowKnobDrawable = juce::Drawable::createFromImageData(BinaryData::LowEq_Knob_svg, BinaryData::LowEq_Knob_svgSize);
    lowKnobLookAndFeel.knobImage = lowKnobDrawable.get();
    lowKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lowKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lowKnob.setRange(0.0, 1.0, 0.01);
    lowKnob.setValue(0.5);
    lowKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    lowKnob.setLookAndFeel(&lowKnobLookAndFeel);
    addAndMakeVisible(lowKnob);

    toneKnobDrawable = juce::Drawable::createFromImageData(BinaryData::ToneEq_Knob_svg, BinaryData::ToneEq_Knob_svgSize);
    toneKnobLookAndFeel.knobImage = toneKnobDrawable.get();
    toneKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    toneKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    toneKnob.setRange(0.0, 1.0, 0.01);
    toneKnob.setValue(0.5);
    toneKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.74f, true);
    toneKnob.setLookAndFeel(&toneKnobLookAndFeel);
    addAndMakeVisible(toneKnob);

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

    // === TOGGLE COMP ===
    softHighlightDrawable = juce::Drawable::createFromImageData(BinaryData::Soft_Comp_svg, BinaryData::Soft_Comp_svgSize);
    hardHighlightDrawable = juce::Drawable::createFromImageData(BinaryData::Hard_Comp_svg, BinaryData::Hard_Comp_svgSize);
    
    toggleCompButton.setAlpha(0.0f);
    toggleCompButton.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
    toggleCompButton.setClickingTogglesState(true);
    toggleCompButton.setToggleState(isSoftMode, juce::dontSendNotification);

    toggleCompButton.onClick = [this]()
    {
        isSoftMode = !isSoftMode;
        toggleCompButton.setToggleState(isSoftMode, juce::dontSendNotification);
        repaint();
    };

    addAndMakeVisible(toggleCompButton);

    softHighlightArea = { 253, 190, 37, 41 };
    hardHighlightArea = { 289, 190, 37, 41 };

    // === TOGGLE SATURAZIONE ===
    saturToggleButton.setAlpha(0.0f); // invisibile ma cliccabile
    saturToggleButton.setClickingTogglesState(true);
    saturToggleButton.setToggleState(isSoftSaturMode, juce::dontSendNotification);
    saturToggleButton.onClick = [this]()
    {
        isSoftSaturMode = !isSoftSaturMode;
        saturToggleButton.setToggleState(isSoftSaturMode, juce::dontSendNotification);
        repaint();
    };
    addAndMakeVisible(saturToggleButton);

    softSatHighlightArea = { 254, 278, 37, 41 };   // es. posizione satur soft
    hardSatHighlightArea = { 289, 278, 37, 41 };   // es. posizione satur hard

    softSatHighlightDrawable = juce::Drawable::createFromImageData(BinaryData::Soft_Satur_svg,BinaryData::Soft_Satur_svgSize);
    hardSatHighlightDrawable = juce::Drawable::createFromImageData(BinaryData::Hard_Satur_svg,BinaryData::Hard_Satur_svgSize);
}


EasyRecAudioProcessorEditor::~EasyRecAudioProcessorEditor() = default;

//==============================================================================
void EasyRecAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    
    if (backgroundImage.isValid())
    {
        g.drawImage(backgroundImage,
                    getLocalBounds().toFloat(),
                    juce::RectanglePlacement::stretchToFit);
    }
    
    // Disegna highlight SVG in base al toggle
    if (isSoftMode && softHighlightDrawable)
        softHighlightDrawable->drawWithin(g, softHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (!isSoftMode && hardHighlightDrawable)
        hardHighlightDrawable->drawWithin(g, hardHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    
    // Highlight saturazione soft/hard
    if (isSoftSaturMode && softSatHighlightDrawable)
        softSatHighlightDrawable->drawWithin(g, softSatHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
    else if (!isSoftSaturMode && hardSatHighlightDrawable)
        hardSatHighlightDrawable->drawWithin(g, hardSatHighlightArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
}

void EasyRecAudioProcessorEditor::resized()
{
    compKnob.setBounds(344, 194, 37, 37);
    lowKnob.setBounds(419, 116, 37, 37);
    toneKnob.setBounds(490, 116, 37, 37);
    deeKnob.setBounds(489, 194, 37, 37);
    satKnob.setBounds(344, 282, 37, 37);
    outKnob.setBounds(463, 273, 32, 32);
    
    // Posizione di toggle
    toggleCompButton.setBounds(240, 195, 100, 30);
    saturToggleButton.setBounds(240, 283, 100, 30);

}
