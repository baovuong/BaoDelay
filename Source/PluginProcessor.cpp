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

    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    float time = *timeParameter;
    float mix = *mixParameter;
    float feedback = *feedbackParameter;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *channelData = buffer.getWritePointer(channel);

        fillDelayBuffer(channel, bufferSize, delayBufferSize, channelData, mix);
        readFromDelayBuffer(channel, bufferSize, delayBufferSize, buffer, delayBuffer, time, mix);
    }
    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}

void BaoDelayAudioProcessor::readFromDelayBuffer(int channel, int bufferSize, int delayBufferSize, juce::AudioBuffer<float> &buffer, juce::AudioBuffer<float>& delayBuffer, float delayTime, float gain)
{
    auto readPosition = writePosition - (getSampleRate() * delayTime);

    if (readPosition < 0)
    {
        readPosition += delayBufferSize;
    }

    if (readPosition + bufferSize < delayBufferSize)
    {
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), bufferSize, gain, gain);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - readPosition;
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, gain, gain);

        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        buffer.addFromWithRamp(channel, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0), numSamplesAtStart, gain, gain);
    }
}

void BaoDelayAudioProcessor::fillDelayBuffer(int channel, int bufferSize, int delayBufferSize, float *channelData, float gain)
{
    // check to see if main buffer copies to delay buffer without needing to wrap...
    if (delayBufferSize > bufferSize + writePosition)
    {
        // copy main buffer contents to delay buffer
        delayBuffer.copyFromWithRamp(channel, writePosition, channelData, bufferSize, gain, gain);
    }
    else
    {
        // Determine how much space is left at the end of the delay buffer
        auto numSamplesToEnd = delayBufferSize - writePosition;

        // Copy that amount of contents to the end...
        delayBuffer.copyFromWithRamp(channel, writePosition, channelData, numSamplesToEnd, gain, gain);

        // Calculate how much contents is remaining to copy
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;

        // Copy remaining amount to beginning of delay buffer
        delayBuffer.copyFromWithRamp(channel, 0, channelData + numSamplesToEnd, numSamplesAtStart, gain, gain);
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
