/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FlexDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FlexDelayAudioProcessorEditor (FlexDelayAudioProcessor&);
    ~FlexDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FlexDelayAudioProcessor& audioProcessor;
    
    juce::Slider main_output_level_slider;
    juce::Label  main_output_level_label;

    juce::Slider wet_mix_slider;
    juce::Label  wet_mix_label;

    juce::Slider delay_msec_slider;
    juce::Label  delay_msec_label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlexDelayAudioProcessorEditor)
};
