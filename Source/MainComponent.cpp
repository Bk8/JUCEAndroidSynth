/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================


//==============================================================================
double MainContentComponent::kMaxDurationOfRecording = 1.0f;

//==============================================================================
MainContentComponent::MainContentComponent()
    : keyboard (keyboardState, MidiKeyboardComponent::horizontalKeyboard),
      recordButton ("Record"),
      stopButton ("Stop"),
      isRecording (false),
      currentRecording (1, 1)
{
    initialiseAudio();
    
    keyboard.setLowestVisibleKey (0x30);
    keyboard.setKeyWidth (600/0x10);
    addAndMakeVisible (keyboard);

    recordButton.addListener (this);
    addAndMakeVisible (recordButton);
    
    stopButton.addListener (this);
    addAndMakeVisible (stopButton);
    
    setSize (600, 400);

    String err = deviceManager.initialiseWithDefaultDevices (1, 1);
    jassert (err.isEmpty());

    deviceManager.addAudioCallback (this);
}

//==============================================================================
MainContentComponent::~MainContentComponent()
{
    deviceManager.removeAudioCallback (this);
}

//==============================================================================
void MainContentComponent::initialiseAudio()
{
    // TODO add more voices
    AudioFormatManager fm;
    fm.registerBasicFormats();
    
    MemoryInputStream* soundBuffer = new MemoryInputStream (BinaryData::singing_ogg, BinaryData::singing_oggSize, false);
    ScopedPointer<AudioFormatReader> formatReader (fm.findFormatForFileExtension ("ogg")->createReaderFor (soundBuffer, true));
    BigInteger midiNotes;
    midiNotes.setRange (0, 126, true);
    sound = new SamplerSound ("Voice", *formatReader, midiNotes, 0x40, 0.0, 0.0, 10.0);
    
    synth.addVoice (new SamplerVoice());
    synth.addSound (sound);

}

//==============================================================================
void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

//==============================================================================
void MainContentComponent::resized()
{
    const int buttonWidth = 150;
    const int buttonHeight = 50;
    
    Rectangle<int> r = getLocalBounds();
    keyboard.setBounds (r.removeFromBottom (proportionOfHeight (0.5)));
    recordButton.setBounds (r.removeFromLeft (proportionOfWidth (0.5)).withSizeKeepingCentre (buttonWidth, buttonHeight));
    stopButton.setBounds (r.withSizeKeepingCentre (buttonWidth, buttonHeight));
}

//==============================================================================
void MainContentComponent::buttonClicked (Button* button)
{
    if (button)
        recordButtonClicked();
    else if (button == &stopButton)
        stopButtonClicked();
}

//==============================================================================
void MainContentComponent::recordButtonClicked()
{
    if (! isRecording)
    {
        samplesRecorded = 0;
        currentRecording.clear();
        isRecording = true;
    }
}

//==============================================================================
void MainContentComponent::stopButtonClicked()
{
    // TODO
}

//==============================================================================
void MainContentComponent::audioDeviceIOCallback (const float** inputChannelData,
                                                  int numInputChannels,
                                                  float** outputChannelData,
                                                  int numOutputChannels,
                                                  int numSamples)
{
    AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
    MidiBuffer midiBuffer;

    if (isRecording)
    {
        int len = std::min (currentRecording.getNumSamples() - samplesRecorded, numSamples);
        currentRecording.copyFrom (0, samplesRecorded, inputChannelData[0], len, 1.0f);
        samplesRecorded += len;
        if (samplesRecorded >= currentRecording.getNumSamples())
        {
            isRecording = false;
            MessageManager::callAsync ([this] { playNewSample(); });
        }
    }
    
    keyboardState.processNextMidiBuffer (midiBuffer, 0, numSamples, true);

    buffer.clear();
    synth.renderNextBlock (buffer, midiBuffer, 0, numSamples);                           
}

//==============================================================================
void MainContentComponent::audioDeviceAboutToStart (AudioIODevice* device)
{
    lastSampleRate = device->getCurrentSampleRate();
    currentRecording.setSize (1, kMaxDurationOfRecording * device->getCurrentSampleRate());
    synth.setCurrentPlaybackSampleRate (device->getCurrentSampleRate());
}

//==============================================================================
void MainContentComponent::playNewSample()
{
    MemoryBlock mb;
    MemoryOutputStream* stream = new MemoryOutputStream (mb, true);
    AudioFormatManager fm;

    fm.registerBasicFormats();
    StringPairArray empty;
    {
        ScopedPointer<AudioFormatWriter> writer (fm.findFormatForFileExtension ("wav")->createWriterFor (stream, lastSampleRate, 1, 16, empty, 0));
        writer->writeFromAudioSampleBuffer (currentRecording, 0, currentRecording.getNumSamples());
        writer->flush();
        stream->flush();
    }

    MemoryInputStream* soundBuffer = new MemoryInputStream (mb, false);
    ScopedPointer<AudioFormatReader> formatReader (fm.findFormatForFileExtension ("wav")->createReaderFor (soundBuffer, true));    
    BigInteger midiNotes;
    midiNotes.setRange (0, 126, true);    
    SynthesiserSound::Ptr newSound = new SamplerSound ("UserRecording", *formatReader, midiNotes, 0x40, 0.0, 0.0, 10.0);

    synth.removeSound (0);
    sound = newSound;
    synth.addSound (newSound);
}
