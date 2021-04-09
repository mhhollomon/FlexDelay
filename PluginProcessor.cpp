/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "utils.h"

//==============================================================================
FlexDelayAudioProcessor::FlexDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
}

FlexDelayAudioProcessor::~FlexDelayAudioProcessor() {
}

//==============================================================================
const juce::String FlexDelayAudioProcessor::getName() const {
	return JucePlugin_Name;
}

bool FlexDelayAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool FlexDelayAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool FlexDelayAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double FlexDelayAudioProcessor::getTailLengthSeconds() const {
	return 0.0;
}

int FlexDelayAudioProcessor::getNumPrograms() {
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
				// so this should be at least 1, even if you're not really implementing programs.
}

int FlexDelayAudioProcessor::getCurrentProgram() {
	return 0;
}

void FlexDelayAudioProcessor::setCurrentProgram(int index) {
}

const juce::String FlexDelayAudioProcessor::getProgramName(int index) {
	return {};
}

void FlexDelayAudioProcessor::changeProgramName(int index, const juce::String& newName) {
}

void FlexDelayAudioProcessor::calculate_scale_factor() {
	scale_factor = utils::db_to_factor(current_main_output_level);
}


//==============================================================================
void FlexDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
	// Use this method as the place to do any pre-playback
	// initialisation that you need..

	current_main_output_level = target_main_output_level;

	calculate_scale_factor();

	current_delay_msec = target_delay_msec;

	DBG("setting delay to " << current_delay_msec << " msec in prepare\n");
	delay_element.set_delay(current_delay_msec, sampleRate);
}


//==============================================================================
void FlexDelayAudioProcessor::releaseResources() {
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FlexDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void FlexDelayAudioProcessor::delay(int channel, const std::vector<double> &input, std::vector<double>& output)  {

	// Yea, I know. But this will get more complicated once there are multiple chains of delays.
	delay_element.do_delay(channel, input, output);
}

//==============================================================================
void FlexDelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();
	auto num_samples = buffer.getNumSamples();

	auto input_buffer = std::vector<double>();

	std::vector<std::vector<double>> wets;
	wets.resize(totalNumOutputChannels);

	// If the user has moved the slider, let the processor know.
	auto local_delay = target_delay_msec;
	if (local_delay != current_delay_msec) {
		current_delay_msec = local_delay;
		DBG("setting delay TARGET to " << current_delay_msec << " msec in process\n");
		delay_element.change_delay(current_delay_msec);
	}


	for (int channel = 0; channel < totalNumInputChannels; ++channel) {
		auto* channel_data = buffer.getWritePointer(channel);
		input_buffer.clear();
		input_buffer.insert(input_buffer.end(), channel_data, channel_data + num_samples);
		delay(channel, input_buffer, wets[channel]);
	}

	for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel) {
		wets[channel].resize(num_samples, 0.0);
	}

	auto local_target_level = target_main_output_level;
	auto wet_level = target_wet_mix / 100.0;
	auto dry_level = 1 - wet_level;

	if (local_target_level != current_main_output_level) {
		auto level_delta = (local_target_level - current_main_output_level)/ num_samples;
		for (size_t i = 0; i < num_samples; ++i) {
			current_main_output_level += level_delta;
			calculate_scale_factor();
			for (int channel = 0; channel < totalNumInputChannels; ++channel) {
				auto* channel_data = buffer.getWritePointer(channel);

				// Add the wet and dry together then scale.
				channel_data[i] = std::tanh(wet_level * wets[channel][i] + dry_level * channel_data[i]) * scale_factor;

			}
			// Overkill for now, but in the future, we might have channel 0 input data be delayed into output channel 5 (e.g.)
			// I would hope that the user would do that kind of thing by routing in the DAW, but we might as well
			// do something kind a right.
			for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel) {
				auto* channel_data = buffer.getWritePointer(channel);
				channel_data[i] = std::tanh(wet_level * wets[channel][i]) * scale_factor;
			}

		}
	} else {
		for (int channel = 0; channel < totalNumInputChannels; ++channel) {
			auto* channel_data = buffer.getWritePointer(channel);
			for (size_t i = 0; i < num_samples; ++i) {
				channel_data[i] = (wet_level * wets[channel][i] + dry_level *channel_data[i]) * scale_factor;
			}
		}
	}
}

//==============================================================================
bool FlexDelayAudioProcessor::hasEditor() const {
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FlexDelayAudioProcessor::createEditor() {
	return new FlexDelayAudioProcessorEditor(*this);
}

//==============================================================================
void FlexDelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void FlexDelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
	return new FlexDelayAudioProcessor();
}
