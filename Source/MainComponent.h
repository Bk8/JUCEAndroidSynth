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
                               public MidiInputCallback,
                               public ButtonListener,
                               private Timer
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

    //==========================================================================
    void handleIncomingMidiMessage (MidiInput* source,
                                    const MidiMessage& message) override;

private:
    static constexpr int maxNumVoices = 5;

    //==========================================================================
    void initialiseAudio();
    void playNewSample();
    void loadNewSample (const void* data, int dataSize, const char* format);
    void recordButtonClicked();

    //==========================================================================
    void timerCallback() override;

    static double kMaxDurationOfRecording;

    AudioDeviceManager deviceManager;
    AudioFormatManager formatManager;

    MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboard;
    Synthesiser synth;
    SynthesiserSound::Ptr sound;

    TextButton recordButton, bluetoothButton;
    bool isRecording;
    int samplesRecorded;
    double lastSampleRate;
    AudioBuffer<float> currentRecording;

    MidiBuffer midiBuffer;
    StringArray lastMidiDevices;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
