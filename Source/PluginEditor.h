/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PhaseEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PhaseEQAudioProcessorEditor (PhaseEQAudioProcessor&);
    ~PhaseEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void plot(juce::Graphics& g);


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PhaseEQAudioProcessor& audioProcessor;
    juce::Rectangle<int> window;
    juce::Array<double> freqs;
    juce::Array<double> mags;
    juce::Array<double> phases;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaseEQAudioProcessorEditor)
};
