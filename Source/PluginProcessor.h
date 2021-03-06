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
class PhaseEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PhaseEQAudioProcessor();
    ~PhaseEQAudioProcessor() override;

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

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    juce::AudioProcessorValueTreeState& getParameters() {return parameters;}

    /* my functions */
    inline void setUpdate(bool v) {requiresUpdate = v;}
    inline void setUpdateGUI(bool v) {guiNeedsUpdate = v;}
    inline bool checkForUpdates() {return guiNeedsUpdate;}
    inline void getFreqResponse(double * freqArray, double * mags, size_t n) {filter.state->getMagnitudeForFrequencyArray(freqArray, mags, n, getSampleRate());}
    inline void getPhaseResponse(double * freqArray, double * phs, size_t n) {filter.state->getPhaseForFrequencyArray(freqArray, phs, n, getSampleRate());}
    inline juce::StringArray getFiltersList() {return filtersList;}

    void updateParameters();

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter;
    juce::StringArray filtersList {"Peak", "Low Pass", "High Pass", "Band Pass", "Notch", "All Pass", "Low Shelf", "High Shelf"};
    bool requiresUpdate;
    bool guiNeedsUpdate;
    juce::AudioProcessorValueTreeState parameters;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaseEQAudioProcessor)
};
