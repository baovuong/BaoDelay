/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define MAX_TIME 1 // in seconds

//==============================================================================
BaoDelayAudioProcessor::BaoDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      parameters(*this, nullptr, juce::Identifier("BaoDelay"),
                 {std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 1.0f, 0.5f),
                  std::make_unique<juce::AudioParameterFloat>("time", "Time", 0.0f, 1.0f, 0.5f),
                  std::make_unique<juce::AudioParameterFloat>("mix", "Wet/Dry", 0.0f, 1.0f, 0.5f)})
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
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int BaoDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BaoDelayAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String BaoDelayAudioProcessor::getProgramName(int index)
{
    return {};
}

void BaoDelayAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================
void BaoDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    auto delayBufferSize = sampleRate * MAX_TIME * 2.0;
    delayBuffer.setSize(getTotalNumOutputChannels(), (int)delayBufferSize);
}

void BaoDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BaoDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void BaoDelayAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    const int bufferSize = buffer.getNumSamples();
    const int delayBufferSize = delayBuffer.getNumSamples();

    float time = *timeParameter;
    float mix = *mixParameter;
    float feedback = *feedbackParameter;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *channelData = buffer.getWritePointer(channel);

        const float* bufferData = buffer.getReadPointer(channel);
        const float* delayBufferData = delayBuffer.getReadPointer(channel);

        fillDelayBuffer(channel, bufferSize, delayBufferSize, bufferData, mix);
        readFromDelayBuffer(buffer, channel, bufferSize, delayBufferSize, bufferData, delayBufferData, time);
    }
    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}

void BaoDelayAudioProcessor::readFromDelayBuffer(juce::AudioBuffer<float> &buffer, int channel, int bufferSize, int delayBufferSize, const float* bufferData, const float* delayBufferData, float delayTime)
{
    const int readPosition = (int)(delayBufferSize + writePosition - (getSampleRate() * delayTime)) % delayBufferSize;

    if (delayBufferSize > bufferSize + readPosition)
    {
        buffer.addFrom(channel, 0, delayBufferData + readPosition, bufferSize);
    }
    else
    {
        const int bufferRemaining = delayBufferSize - readPosition;
        buffer.addFrom(channel, 0, delayBufferData + readPosition, bufferRemaining);
        buffer.addFrom(channel, bufferRemaining, delayBufferData, bufferSize - bufferRemaining);
    }
}

void BaoDelayAudioProcessor::fillDelayBuffer(int channel, int bufferSize, int delayBufferSize, const float *bufferData, float gain)
{
    // check to see if main buffer copies to delay buffer without needing to wrap...
    if (delayBufferSize > bufferSize + writePosition)
    {
        // copy main buffer contents to delay buffer
        delayBuffer.copyFromWithRamp(channel, writePosition, bufferData, bufferSize, gain, gain);
    }
    else
    {
        // Determine how much space is left at the end of the delay buffer
        auto numSamplesToEnd = delayBufferSize - writePosition;

        // Copy that amount of contents to the end...
        delayBuffer.copyFromWithRamp(channel, writePosition, bufferData, numSamplesToEnd, gain, gain);

        // Calculate how much contents is remaining to copy
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;

        // Copy remaining amount to beginning of delay buffer
        delayBuffer.copyFromWithRamp(channel, 0, bufferData + numSamplesToEnd, numSamplesAtStart, gain, gain);
    }
}

//==============================================================================
bool BaoDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *BaoDelayAudioProcessor::createEditor()
{
    return new BaoDelayAudioProcessorEditor(*this, parameters);
}

//==============================================================================
void BaoDelayAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BaoDelayAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new BaoDelayAudioProcessor();
}
