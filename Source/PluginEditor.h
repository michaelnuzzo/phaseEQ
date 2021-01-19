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
class PhaseEQAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer
{
public:
    PhaseEQAudioProcessorEditor (PhaseEQAudioProcessor&);
    ~PhaseEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void plot(juce::Graphics& g);


private:
    void timerCallback() override;
    PhaseEQAudioProcessor& audioProcessor;
    juce::Rectangle<int> window;
    juce::Array<double> freqs;
    juce::Array<double> mags;
    juce::Array<double> phases;
    juce::Slider freqKnob, gainKnob, qKnob;
    juce::ComboBox filtersList;
    juce::Label freqLabel, gainLabel, qLabel, filtersLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttachment, gainAttachment, qAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filtersAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaseEQAudioProcessorEditor)
};
