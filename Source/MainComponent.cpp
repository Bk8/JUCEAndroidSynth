/*
  ==============================================================================

    Copyright (c) 2015 - ROLI Ltd.

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
      bluetoothButton ("Bluetooth MIDI"),
      isRecording (false),
      currentRecording (1, 1)
{
    keyboard.setLowestVisibleKey (0x30);
    keyboard.setKeyWidth (600/0x10);
    addAndMakeVisible (keyboard);

    recordButton.addListener (this);
    addAndMakeVisible (recordButton);

    if (! BluetoothMidiDevicePairingDialogue::isAvailable())
        bluetoothButton.setEnabled (false);

    bluetoothButton.addListener (this);
    addAndMakeVisible (bluetoothButton);

    setSize (600, 400);

    initialiseAudio();
}

//==============================================================================
MainContentComponent::~MainContentComponent()
{
    deviceManager.removeAudioCallback (this);
}

//==============================================================================
void MainContentComponent::initialiseAudio()
{
    for (int i = 0; i < maxNumVoices; ++i)
        synth.addVoice (new SamplerVoice());

    formatManager.registerBasicFormats();

    loadNewSample (BinaryData::singing_ogg, BinaryData::singing_oggSize, "ogg");

    String err = deviceManager.initialiseWithDefaultDevices (1, 1);
    jassert (err.isEmpty());

    startTimer (1000);

    deviceManager.addAudioCallback (this);
    deviceManager.addMidiInputCallback (String(), this);
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
    bluetoothButton.setBounds (r.withSizeKeepingCentre (buttonWidth, buttonHeight));
}

//==============================================================================
void MainContentComponent::buttonClicked (Button* button)
{
    if (button == &recordButton)
        recordButtonClicked();
    else if (button == &bluetoothButton)
        BluetoothMidiDevicePairingDialogue::open();
}

//==============================================================================
void MainContentComponent::recordButtonClicked()
{
    if (! isRecording)
    {
        recordButton.setEnabled (false);

        samplesRecorded = 0;
        currentRecording.clear();
        isRecording = true;
    }
}

//==============================================================================
void MainContentComponent::audioDeviceIOCallback (const float** inputChannelData,
                                                  int numInputChannels,
                                                  float** outputChannelData,
                                                  int numOutputChannels,
                                                  int numSamples)
{
    ignoreUnused (numInputChannels);

    AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);

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
    midiBuffer.clear();
}

//==============================================================================
void MainContentComponent::audioDeviceAboutToStart (AudioIODevice* device)
{
    lastSampleRate = device->getCurrentSampleRate();
    currentRecording.setSize (1, static_cast<int> (std::ceil (kMaxDurationOfRecording * device->getCurrentSampleRate())));
    synth.setCurrentPlaybackSampleRate (device->getCurrentSampleRate());
}

void MainContentComponent::handleIncomingMidiMessage (MidiInput* source,
                                                      const MidiMessage& message)
{
    ignoreUnused (source);

    midiBuffer.addEvent (message, 0);
}

//==============================================================================
void MainContentComponent::loadNewSample (const void* data, int dataSize, const char* format)
{
    MemoryInputStream* soundBuffer = new MemoryInputStream (data, static_cast<std::size_t> (dataSize), false);
    ScopedPointer<AudioFormatReader> formatReader (formatManager.findFormatForFileExtension (format)->createReaderFor (soundBuffer, true));

    BigInteger midiNotes;
    midiNotes.setRange (0, 126, true);
    SynthesiserSound::Ptr newSound = new SamplerSound ("Voice", *formatReader, midiNotes, 0x40, 0.0, 0.0, 10.0);

    synth.removeSound (0);
    sound = newSound;
    synth.addSound (sound);
}

void MainContentComponent::playNewSample()
{
    MemoryBlock mb;
    MemoryOutputStream* stream = new MemoryOutputStream (mb, true);


    {
        ScopedPointer<AudioFormatWriter> writer (formatManager.findFormatForFileExtension ("wav")->createWriterFor (stream, lastSampleRate, 1, 16,
                                                                                                                    StringPairArray(), 0));
        writer->writeFromAudioSampleBuffer (currentRecording, 0, currentRecording.getNumSamples());
        writer->flush();
        stream->flush();
    }

    loadNewSample (mb.getData(), static_cast<int> (mb.getSize()), "wav");

    recordButton.setEnabled (true);
}

void MainContentComponent::timerCallback()
{
    StringArray newDevices = MidiInput::getDevices();

    for (int i = 0; i < lastMidiDevices.size(); ++i)
        if (newDevices.indexOf (lastMidiDevices[i]) < 0)
            deviceManager.setMidiInputEnabled (lastMidiDevices[i], false);

    for (int i = 0; i < newDevices.size(); ++i)
        if (lastMidiDevices.indexOf (newDevices[i]) < 0)
            deviceManager.setMidiInputEnabled (newDevices[i], true);

    lastMidiDevices = newDevices;
}
