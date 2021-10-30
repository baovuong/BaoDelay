/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BaoDelayAudioProcessor::BaoDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, juce::Identifier("BaoDelay"),
                       {
                            std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 1.0f, 0.5f),
                            std::make_unique<juce::AudioParameterFloat>("time", "Time", 0.0f, 1.0f, 0.5f),
                            std::make_unique<juce::AudioParameterFloat>("mix", "Wet/Dry", 0.0f, 1.0f, 0.5f)
                       })
#endif
{
    mixParameter = parameters.getRawParameterValue("mix");
    feedbackParameter = parameters.getRawParameterValue("feedback");
    timeParameter = parameters.getRawParameterValue("time");
}

BaoDelayAudioProcessor::~BaoDelayAudioProcessor()
{
}

//==============================================================================
const juce::String BaoDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BaoDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BaoDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BaoDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BaoDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BaoDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BaoDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BaoDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BaoDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void BaoDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BaoDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void BaoDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BaoDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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

void BaoDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    std::cout << "Here we go!" << std::endl;

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    int d = *timeParameter*buffer.getNumSamples();
    std::cout << "d: " << d << std::endl;
    int a = *mixParameter;
    std::cout << "a: " << a << std::endl;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        const float* inBuffer = buffer.getReadPointer(channel, 0);
        float* outBuffer = buffer.getWritePointer (channel, 0);

        for (int sample = 0; sample < buffer.getNumSamples(); sample++) {
            int offset = (sample - d) % buffer.getNumSamples();
            outBuffer[sample] = inBuffer[sample] + (sample >= d ? a*inBuffer[offset] : 0.f);
        }

    }
}

//==============================================================================
bool BaoDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BaoDelayAudioProcessor::createEditor()
{
    return new BaoDelayAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void BaoDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BaoDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BaoDelayAudioProcessor();
}
