/*
  ==============================================================================

    VngIdLookAndFeel.cpp
    Created: 4 Nov 2021 10:16:24pm
    Author:  bvuong

  ==============================================================================
*/

#include "VngIdLookAndFeel.h"

#define PI_OVER_2 1.57079632679

VngIdLookAndFeel::VngIdLookAndFeel()
{
}

void VngIdLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle) - PI_OVER_2;

    g.setColour(juce::Colour::fromHSV(0, 0, 0.15, 1));
    g.fillEllipse(centreX - radius, centreY - radius, rw, rw);



    // debug things
    g.setColour(juce::Colours::green);

    auto pointerRadius = radius * 0.1;
    auto pointerX = centreX + (radius - pointerRadius - 1) * std::cos(angle);
    auto pointerY = centreY + (radius - pointerRadius - 1) * std::sin(angle);

    g.fillEllipse(pointerX - pointerRadius, pointerY - pointerRadius, pointerRadius*2, pointerRadius*2);
}
