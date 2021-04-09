/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "StereoDelayElement.h"

//==============================================================================
/**
*/
class FlexDelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FlexDelayAudioProcessor();
    ~FlexDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    double target_main_output_level = 0.0;
    double target_wet_mix = 50.0;
    double target_delay_msec = 200;

private:
    //==============================================================================
    double current_main_output_level = 0.0;
    double scale_factor = 1.0;
    StereoDelayElement delay_element;

    double current_delay_msec = 200;
    int sample_rate_ = 100;
    int delay_samples = 100;

    void calculate_scale_factor();

    void delay(int channel, const std::vector<double>& input, std::vector<double>& output);

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlexDelayAudioProcessor)
};
