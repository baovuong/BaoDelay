/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BaoDelayAudioProcessorEditor::BaoDelayAudioProcessorEditor (BaoDelayAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 500);
    setLookAndFeel(&vngIdLookAndFeel);

    // === Mix ===
    addAndMakeVisible(mixKnob);
    mixKnob.setRange(0.0, 0.1, 0.01);
    mixKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mixKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 200, 20);
    mixAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "mix", mixKnob));

    addAndMakeVisible(mixLabel);
    mixLabel.setText("Dry/Wet", juce::NotificationType::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    
    // === Feedback ===
    addAndMakeVisible(feedbackKnob);
    feedbackKnob.setRange(0.0, 0.1, 0.01);
    feedbackKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    feedbackKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    feedbackAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "feedback", feedbackKnob));

    addAndMakeVisible(feedbackLabel);
    feedbackLabel.setText("Feedback", juce::NotificationType::dontSendNotification);
    feedbackLabel.setJustificationType(juce::Justification::centred);

    // === Time ===
    addAndMakeVisible(timeKnob);
    timeKnob.setRange(0.0, 0.1, 0.01);
    timeKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    timeKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    timeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "time", timeKnob));

    addAndMakeVisible(timeLabel);
    timeLabel.setText("Time", juce::NotificationType::dontSendNotification);
    timeLabel.setJustificationType(juce::Justification::centred);
}

BaoDelayAudioProcessorEditor::~BaoDelayAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
	mixAttachment.reset();
	feedbackAttachment.reset();
	timeAttachment.reset();
}

//==============================================================================
void BaoDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
}

void BaoDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    juce::Rectangle<int> area = getLocalBounds();
    area.reduce(20,30);
    
    int spacing = 0.1;

    mixKnob.setBounds(area.removeFromRight(area.getWidth()/2));
    mixLabel.setBounds(mixKnob.getX(), mixKnob.getY() + 60, mixKnob.getWidth(), 20);
    
    juce::Rectangle<int> feedbackArea = area.removeFromBottom(area.getHeight()/2);
    feedbackArea.reduce(10, 10);
    feedbackKnob.setBounds(feedbackArea);
    feedbackLabel.setBounds(feedbackKnob.getX(), feedbackKnob.getY()-20, feedbackKnob.getWidth(), 20);
    
    area.reduce(10, 10);
    timeKnob.setBounds(area);
    timeLabel.setBounds(timeKnob.getX(), timeKnob.getY()-20, timeKnob.getWidth(), 20);
}
