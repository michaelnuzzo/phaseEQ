/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhaseEQAudioProcessorEditor::PhaseEQAudioProcessorEditor (PhaseEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1000, 800);

    int padding = 50;
    window.setSize(getWidth()-padding, getHeight()*1/2);
    window.setLeft(padding*3);
    window.setTop(padding);

    freqs.resize(window.getWidth());
    for(int i = 0; i < freqs.size(); i++)
    {
        freqs.set(i, audioProcessor.getSampleRate()/2.f * (1.0f - std::exp (std::log (1.0f - (float) i / (float) freqs.size()) * 0.1f)));
    }
    mags.resize(window.getWidth());
    mags.fill(0);
    phases.resize(window.getWidth());
    phases.fill(0);
}

PhaseEQAudioProcessorEditor::~PhaseEQAudioProcessorEditor()
{
}

//==============================================================================
void PhaseEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour(juce::Colours::lightgrey);

    plot(g);
}

void PhaseEQAudioProcessorEditor::plot(juce::Graphics& g)
{
    g.drawRect(window);
    audioProcessor.getFreqResponse(freqs.getRawDataPointer(), mags.getRawDataPointer(), freqs.size());
    audioProcessor.getPhaseResponse(freqs.getRawDataPointer(), phases.getRawDataPointer(), freqs.size());

    int w  = window.getWidth() - 1;  // window width
    int h  = window.getHeight() - 1; // window height
    int l  = window.getX();          // window left
    int r  = l + w;                  // window right
    int b  = window.getY();          // window bottom
    int t  = b + h;                  // window top
    float yMax = 10.f;
    float yMin = -10.f;

    {
        g.setColour(juce::Colours::lightgreen);
        auto valToCoord = [&](float v)
        {
            return juce::jlimit(b, t, b + int(juce::jmap(juce::Decibels::gainToDecibels(v), yMax, yMin, 0.f, float(h))));
        };

        for (int i = 1; i < w; i++)
        {
            int x1 = i + l;
            int y1 = valToCoord(mags[i]);
            int y2 = valToCoord(mags[i + 1]);

            if(y1 < y2)
            {
                std::swap(y1, y2);
            }
            g.fillRect(x1, y2, 1, 1 + (y1 - y2));
        }
    }
    {
        g.setColour(juce::Colours::dodgerblue);
        auto valToCoord = [&](float v)
        {
            return juce::jlimit(b, t, b + int(juce::jmap(juce::radiansToDegrees(v), -180.f, 180.f, 0.f, float(h))));
        };

        for (int i = 1; i < w; i++)
        {
            int x1 = i + l;
            int y1 = valToCoord(phases[i]);
            int y2 = valToCoord(phases[i + 1]);

            if(y1 < y2)
            {
                std::swap(y1, y2);
            }
            g.fillRect(x1, y2, 1, 1 + (y1 - y2));
        }
    }


}

void PhaseEQAudioProcessorEditor::resized()
{
}
