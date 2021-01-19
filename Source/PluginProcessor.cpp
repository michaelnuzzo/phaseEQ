/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhaseEQAudioProcessor::PhaseEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                    parameters(*this, nullptr, "Parameters", createParameters())
#endif
{
}

PhaseEQAudioProcessor::~PhaseEQAudioProcessor()
{
}

//==============================================================================
const juce::String PhaseEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhaseEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhaseEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PhaseEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PhaseEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhaseEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhaseEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhaseEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PhaseEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void PhaseEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PhaseEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumInputChannels();

    filter.prepare(spec);

    setUpdate(true);
}

void PhaseEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhaseEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PhaseEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if(requiresUpdate)
    {
        updateParameters();
    }

    auto block = juce::dsp::AudioBlock<float>(buffer);
    filter.process(juce::dsp::ProcessContextReplacing<float>(block));
}

void PhaseEQAudioProcessor::updateParameters()
{
    setUpdateGUI(false);
    float freq = *parameters.getRawParameterValue("FREQ");
    float gain = *parameters.getRawParameterValue("GAIN");
    float q = *parameters.getRawParameterValue("Q");
    int filterChoice = *parameters.getRawParameterValue("FILTERS");

    switch(filterChoice)
    {
        case 0:
            *filter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), freq, q, juce::Decibels::decibelsToGain(gain));
            break;
        case 1:
            *filter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), freq, q);
            break;
        case 2:
            *filter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), freq, q);
            break;
        case 3:
            *filter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(getSampleRate(), freq, q);
            break;
        case 4:
            *filter.state = *juce::dsp::IIR::Coefficients<float>::makeNotch(getSampleRate(), freq, q);
            break;
        case 5:
            *filter.state = *juce::dsp::IIR::Coefficients<float>::makeAllPass(getSampleRate(), freq, q);
            break;
        case 6:
            *filter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), freq, q, juce::Decibels::decibelsToGain(gain));
            break;
        case 7:
            *filter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), freq, q, juce::Decibels::decibelsToGain(gain));
            break;
        default:
            jassert(false);
    }

    setUpdateGUI(true);
    setUpdate(false);
}

juce::AudioProcessorValueTreeState::ParameterLayout PhaseEQAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FREQ"   , "Freq"   , juce::NormalisableRange<float>(30.f , 20000.f, 0.001f, 0.2f), 1000.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN"   , "Gain"   , juce::NormalisableRange<float>(-10.f, 10.f   , 0.001f      ), 0.f   ));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("Q"      , "Q"      , juce::NormalisableRange<float>(0.1f , 18.f   , 0.001f      ), .707f ));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("FILTERS", "Filters", filtersList, 0));
    return { params.begin(), params.end() };
}

//==============================================================================
bool PhaseEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PhaseEQAudioProcessor::createEditor()
{
    return new PhaseEQAudioProcessorEditor (*this);
}

//==============================================================================
void PhaseEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml(parameters.state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PhaseEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if(xmlState.get() != nullptr)
    {
        if(xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.state = juce::ValueTree::fromXml(*xmlState);
        }
    }
    setUpdate(true);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhaseEQAudioProcessor();
}
