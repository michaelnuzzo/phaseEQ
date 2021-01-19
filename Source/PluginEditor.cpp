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
    startTimerHz(60);

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

    /* initialize parameters */

    freqKnob.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::lightgrey);
    freqKnob.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    freqKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 100, 20);
    freqKnob.setTextValueSuffix(" Hz");
    freqKnob.onValueChange = [this] {audioProcessor.setUpdate(true);};
    freqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getParameters(),"FREQ",freqKnob);
    freqLabel.setText("Frequency", juce::dontSendNotification);
    freqLabel.setJustificationType(juce::Justification::horizontallyCentred);
    freqLabel.attachToComponent(&freqKnob, true);
    addAndMakeVisible(freqKnob);

    gainKnob.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::lightgrey);
    gainKnob.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    gainKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 100, 20);
    gainKnob.setTextValueSuffix(" dB");
    gainKnob.onValueChange = [this] {audioProcessor.setUpdate(true);};
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getParameters(),"GAIN",gainKnob);
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::horizontallyCentred);
    gainLabel.attachToComponent(&gainKnob, true);
    addAndMakeVisible(gainKnob);

    qKnob.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::lightgrey);
    qKnob.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    qKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 100, 20);
    qKnob.onValueChange = [this] {audioProcessor.setUpdate(true);};
    qAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getParameters(),"Q",qKnob);
    qLabel.setText("Q", juce::dontSendNotification);
    qLabel.setJustificationType(juce::Justification::horizontallyCentred);
    qLabel.attachToComponent(&qKnob, true);
    addAndMakeVisible(qKnob);

    filtersList.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::lightgrey);
    filtersList.onChange = [this] {audioProcessor.setUpdate(true);};
    auto items = audioProcessor.getFiltersList();
    for(int i = 0; i < items.size(); i++)
        filtersList.addItem(items[i], i+1);
    filtersAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.getParameters(),"FILTERS",filtersList);
    filtersLabel.setText("Filters", juce::dontSendNotification);
    filtersLabel.setJustificationType(juce::Justification::horizontallyCentred);
    filtersLabel.attachToComponent(&filtersList, true);
    addAndMakeVisible(filtersList);


    /* set positions */
    int spacing = 60;
    int gap = spacing*3/4;

//    freqKnob.setBounds(window);
    freqKnob.setBounds(getWidth()/3, getHeight()-spacing*4-gap, 400, 50);
    gainKnob.setBounds(getWidth()/3, getHeight()-spacing*3-gap, 400, 50);
    qKnob.setBounds(getWidth()/3, getHeight()-spacing*2-gap, 400, 50);
    filtersList.setBounds(100, getHeight()-spacing*4-gap, 100, 25);
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
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(window);
}

void PhaseEQAudioProcessorEditor::resized()
{
}

void PhaseEQAudioProcessorEditor::timerCallback()
{
    if(audioProcessor.checkForUpdates())
    {
        repaint();
        audioProcessor.setUpdateGUI(false);
    }
}
