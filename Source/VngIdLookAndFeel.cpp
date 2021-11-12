/*
  ==============================================================================

    VngIdLookAndFeel.cpp
    Created: 4 Nov 2021 10:16:24pm
    Author:  bvuong

  ==============================================================================
*/

#include "VngIdLookAndFeel.h"

VngIdLookAndFeel::VngIdLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colour::fromRGB(110, 255, 127));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromRGB(40,40,40));
    setColour(juce::Slider::rotarySliderFillColourId, findColour(juce::Slider::rotarySliderOutlineColourId).brighter(0.05));
}

void VngIdLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto dialRadius = radius * 0.9;

    // === full circle ===
    g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
    g.fillEllipse(rx, ry, rw, rw);

    // === dial center ===
    juce::Path dialCenter;

    auto anglePadding = juce::MathConstants<float>::pi * 0.05;
    dialCenter.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, 0);
    g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
    g.fillPath(dialCenter);


    // === pointer ===
    juce::Path pointerLevel;
    pointerLevel.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0);
    g.setColour(findColour(juce::Slider::thumbColourId));
    g.fillPath(pointerLevel);


    g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
    g.fillEllipse(centreX - dialRadius, centreY - dialRadius, 2*dialRadius, 2*dialRadius);
}
