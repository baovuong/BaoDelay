/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "VngIdLookAndFeel.h"

//==============================================================================
/**
*/
class BaoDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BaoDelayAudioProcessorEditor (BaoDelayAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~BaoDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BaoDelayAudioProcessor& audioProcessor;

    VngIdLookAndFeel vngIdLookAndFeel;

    juce::Label versionLabel;


    juce::AudioProcessorValueTreeState& valueTreeState;
    
    // Wet/Dry
    juce::Label mixLabel;
    juce::Slider mixKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    // Feedback
    juce::Label feedbackLabel;
    juce::Slider feedbackKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;

    // Time
    juce::Label timeLabel;
    juce::Slider timeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BaoDelayAudioProcessorEditor)
};
