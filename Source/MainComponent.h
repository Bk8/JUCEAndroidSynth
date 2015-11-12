/*
  ==============================================================================

    Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AndroidSynthProcessor.h"

//==============================================================================
class MainContentComponent   : public Component,
                               public ChangeListener,
                               public ButtonListener,
                               public Slider::Listener
{
public:
    //==========================================================================
    MainContentComponent (AndroidSynthProcessor& androidSynth)
        :   synth (androidSynth),
            keyboard (synth.keyboardState, MidiKeyboardComponent::horizontalKeyboard),
            recordButton ("Record"), bluetoothButton ("Bluetooth"),
            roomSizeSlider (Slider::LinearHorizontal, Slider::NoTextBox)
    {
        synth.addChangeListener (this);
        roomSizeSlider.setValue (synth.reverbParameters.roomSize, NotificationType::dontSendNotification);

        keyboard.setLowestVisibleKey (0x30);
        keyboard.setKeyWidth (600/0x10);
        addAndMakeVisible (keyboard);

        recordButton.addListener (this);
        addAndMakeVisible (recordButton);

        bluetoothButton.addListener (this);
        addAndMakeVisible (bluetoothButton);
        bluetoothButton.setEnabled (BluetoothMidiDevicePairingDialogue::isAvailable());

        roomSizeSlider.addListener (this);
        roomSizeSlider.setRange (0.0, 1.0);
        addAndMakeVisible (roomSizeSlider);

        Path proAudioPath;
        proAudioPath.loadPathFromData (BinaryData::proaudio_path, BinaryData::proaudio_pathSize);
        proAudioIcon.setPath (proAudioPath);
        addAndMakeVisible (proAudioIcon);

        Colour proAudioIconColour = findColour (TextButton::buttonColourId);
        proAudioIcon.setFill (FillType (proAudioIconColour));

        setSize (600, 400);
    }

    ~MainContentComponent()
    {
        synth.removeChangeListener (this);
    }

    //==========================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        Rectangle<int> r = getLocalBounds();
        keyboard.setBounds (r.removeFromBottom (proportionOfHeight (0.5)));

        int guiElementAreaHeight = r.getHeight() / 3;

        proAudioIcon.setTransformToFit (r.removeFromLeft (proportionOfWidth (0.25))
                                        .withSizeKeepingCentre (guiElementAreaHeight, guiElementAreaHeight)
                                        .toFloat(),
                                        RectanglePlacement::fillDestination);

        int margin = guiElementAreaHeight / 4;
        r.reduce (margin, margin);

        int buttonHeight = guiElementAreaHeight - margin;

        recordButton.setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
        bluetoothButton.setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
        roomSizeSlider.setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
    }

    //==========================================================================
    void buttonClicked (Button* button) override
    {
        if (button == &recordButton)
        {
            recordButton.setEnabled (false);
            synth.startRecording();
        } else if (button == &bluetoothButton)
            BluetoothMidiDevicePairingDialogue::open();
    }

    void sliderValueChanged (Slider*) override
    {
        synth.reverbParameters.roomSize = roomSizeSlider.getValue();
    }

    //==========================================================================
    void changeListenerCallback (ChangeBroadcaster*) override
    {
        recordButton.setEnabled (! synth.isRecording);
    }
private:
    //==========================================================================
    AndroidSynthProcessor& synth;

    //==========================================================================
    MidiKeyboardComponent keyboard;
    TextButton recordButton, bluetoothButton;
    Slider roomSizeSlider;
    DrawablePath proAudioIcon;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
