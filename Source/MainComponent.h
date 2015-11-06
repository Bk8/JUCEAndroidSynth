/*
  ==============================================================================

    Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class MainContentComponent   : public Component,
                               public AudioIODeviceCallback,
                               public ButtonListener
{
public:
    //==========================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;

    //==========================================================================
    void audioDeviceIOCallback (const float** inputChannelData,
                                int numInputChannels,
                                float** outputChannelData,
                                int numOutputChannels,
                                int numSamples) override;
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    void audioDeviceStopped() override {}

    void playNewSample();


private:
    //==========================================================================
    void initialiseAudio();
    void recordButtonClicked();
    void stopButtonClicked();

    static double kMaxDurationOfRecording;

    AudioDeviceManager deviceManager;

    MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboard;
    Synthesiser synth;
    SynthesiserSound::Ptr sound;

    TextButton recordButton, stopButton;
    bool isRecording;
    int samplesRecorded;
    double lastSampleRate;
    AudioBuffer<float> currentRecording;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
