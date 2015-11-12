/*
  ==============================================================================

    Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef ANDROIDSYNTHPROCESSOR_H_INCLUDED
#define ANDROIDSYNTHPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class AndroidSynthProcessor : public AudioProcessor,
                              public ChangeBroadcaster
{
public:
    AndroidSynthProcessor ()
        : isRecording (false),
          currentRecording (1, 1)
    {
        formatManager.registerBasicFormats();

        for (int i = 0; i < maxNumVoices; ++i)
            synth.addVoice (new SamplerVoice());

        loadNewSample (BinaryData::singing_ogg, BinaryData::singing_oggSize, "ogg");
    }

    //==============================================================================
    void startRecording ()
    {
        if (! isRecording)
        {
            samplesRecorded = 0;
            isRecording = true;
            currentRecording.clear();

            sendSynchronousChangeMessage();
        }
    }

    //==============================================================================
    void prepareToPlay (double sampleRate, int estimatedMaxSizeOfBuffer) override
    {
        ignoreUnused (estimatedMaxSizeOfBuffer);

        lastSampleRate = sampleRate;
        currentRecording.setSize (1, static_cast<int> (std::ceil (kMaxDurationOfRecording * lastSampleRate)));
        synth.setCurrentPlaybackSampleRate (lastSampleRate);
        reverb.setSampleRate (lastSampleRate);
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        if (isRecording)
        {
            int len = std::min (currentRecording.getNumSamples() - samplesRecorded, buffer.getNumSamples());
            currentRecording.copyFrom (0, samplesRecorded, buffer.getReadPointer (0), len, 1.0f);
            samplesRecorded += len;
            if (samplesRecorded >= currentRecording.getNumSamples())
            {
                isRecording = false;
                sendChangeMessage();
                MessageManager::callAsync ([this] { swapSamples(); });
            }
        }

        keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);
        buffer.clear();

        reverb.setParameters (reverbParameters);
        synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
        reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());
    }


    //==============================================================================
    void releaseResources() override                                            { currentRecording.setSize (1, 1); }

    //==============================================================================
    const String getInputChannelName (int channelIndex) const override          { return String (channelIndex + 1); }
    const String getOutputChannelName (int channelIndex) const override         { return String (channelIndex + 1); }
    bool isInputChannelStereoPair (int /*index*/) const override                { return false; }
    bool isOutputChannelStereoPair (int /*index*/) const override               { return false; }

    //==============================================================================
    bool acceptsMidi() const override                                           { return true; }
    bool producesMidi() const override                                          { return false; }
    bool silenceInProducesSilenceOut() const override                           { return false; }
    double getTailLengthSeconds() const override                                { return 0.0; }

    //==============================================================================
    AudioProcessorEditor* createEditor() override                               { return nullptr; }
    bool hasEditor() const override                                             { return false; }

    //==============================================================================
    const String getName() const override                                       { return "Android Synth"; }
    int getNumPrograms() override                                               { return 1; }
    int getCurrentProgram() override                                            { return 0; }
    void setCurrentProgram (int /*index*/) override                             {}
    const String getProgramName (int /*index*/) override                        { return "Default"; }

    //==============================================================================
    void changeProgramName (int /*index*/, const String& /*name*/) override     {}
    void getStateInformation (MemoryBlock&) override                            {}
    void setStateInformation (const void*, int) override                        {}

    //==============================================================================
    bool isRecording;
    Reverb::Parameters reverbParameters;
    MidiKeyboardState keyboardState;

private:
    //==============================================================================
    void loadNewSample (const void* data, int dataSize, const char* format)
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

    void swapSamples()
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
    }

    //==============================================================================
    static constexpr int maxNumVoices = 5;
    static constexpr double kMaxDurationOfRecording = 1.0;

    //==============================================================================
    AudioFormatManager formatManager;

    int samplesRecorded;
    double lastSampleRate;
    AudioBuffer<float> currentRecording;

    Reverb reverb;
    Synthesiser synth;
    SynthesiserSound::Ptr sound;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidSynthProcessor)
};

#endif // ANDROIDSYNTHPROCESSOR_H_INCLUDED
