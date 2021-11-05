/*
  ==============================================================================

    VngIdLookAndFeel.h
    Created: 4 Nov 2021 10:16:24pm
    Author:  bvuong

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class VngIdLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VngIdLookAndFeel();

    // overrides
    void drawRotarySlider (juce::Graphics &g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider);
};