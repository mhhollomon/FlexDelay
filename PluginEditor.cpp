/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FlexDelayAudioProcessorEditor::FlexDelayAudioProcessorEditor (FlexDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // =============== Main Level ===============================
    main_output_level_slider.setRange(-100, 20, 0.01);
    main_output_level_slider.setTextValueSuffix(" dB");
    main_output_level_slider.setSkewFactorFromMidPoint(0.0);
    main_output_level_slider.setValue(0.0);
    main_output_level_slider.setDoubleClickReturnValue(true, 0.0, juce::ModifierKeys::ctrlModifier);
    main_output_level_slider.onValueChange = [this] {audioProcessor.target_main_output_level = main_output_level_slider.getValue(); };
    addAndMakeVisible(main_output_level_slider);

    main_output_level_label.setText("Output Level", juce::dontSendNotification);
    main_output_level_label.attachToComponent(&main_output_level_slider, true);
    addAndMakeVisible(main_output_level_label);

    // === Wet Mix ==========================================
    wet_mix_slider.setRange(1, 100, 0.1);
    wet_mix_slider.setTextValueSuffix(" %");
    wet_mix_slider.setValue(50);
    wet_mix_slider.setDoubleClickReturnValue(true, 50.0, juce::ModifierKeys::ctrlModifier);
    wet_mix_slider.onValueChange = [this] {audioProcessor.target_wet_mix = wet_mix_slider.getValue(); };
    addAndMakeVisible(wet_mix_slider);

    wet_mix_label.setText("Wet/Dry", juce::dontSendNotification);
    wet_mix_label.attachToComponent(&wet_mix_slider, true);
    addAndMakeVisible(wet_mix_label);

    // === delay ==========================================
    delay_msec_slider.setRange(1, 2000, 0.1);
    delay_msec_slider.setTextValueSuffix(" msec");
    delay_msec_slider.setValue(200);
    delay_msec_slider.setDoubleClickReturnValue(true, 200.0, juce::ModifierKeys::ctrlModifier);
    delay_msec_slider.onValueChange = [this] {audioProcessor.target_delay_msec = delay_msec_slider.getValue(); };
    addAndMakeVisible(delay_msec_slider);

    delay_msec_label.setText("Delay", juce::dontSendNotification);
    delay_msec_label.attachToComponent(&delay_msec_slider, true);
    addAndMakeVisible(delay_msec_label);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

FlexDelayAudioProcessorEditor::~FlexDelayAudioProcessorEditor()
{
}

//==============================================================================
void FlexDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    //// fill the whole window white
    //g.fillAll(juce::Colours::white);

    //// set the current drawing colour to black
    //g.setColour(juce::Colours::black);

    //g.setFont (15.0f);
    //g.drawFittedText ("Flex Delay", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void FlexDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto sliderLeft = 120;
    auto slider_height = 20;
    main_output_level_slider.setBounds(sliderLeft, slider_height * 1, getWidth() - sliderLeft - 10, slider_height);
    wet_mix_slider.setBounds(sliderLeft, slider_height * 2, getWidth() - sliderLeft - 10, slider_height);
    delay_msec_slider.setBounds(sliderLeft, slider_height * 3, getWidth() - sliderLeft - 10, slider_height);
}
