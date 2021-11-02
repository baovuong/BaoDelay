/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class BaoDelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    BaoDelayAudioProcessor();
    ~BaoDelayAudioProcessor() override;

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

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    
    float previousMix;
    float previousTime;
    float previousFeedback;

    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* timeParameter = nullptr;
    std::atomic<float>* feedbackParameter = nullptr;

    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;

    void fillDelayBuffer(int channel, int bufferSize, int delayBufferSize, const float* bufferData, float gain);
    void readFromDelayBuffer(juce::AudioBuffer<float> &buffer, int channel, int bufferSize, int delayBufferSize, const float* bufferData, const float* delayBufferData, float delayTime);
    void feedbackDelay(int channel, int bufferSize, int delayBufferSize, const float* bufferData, float* dryBufferData, float gain);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BaoDelayAudioProcessor)
};
