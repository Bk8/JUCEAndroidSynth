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
                               public ButtonListener,
                               public Slider::Listener,
                               private Timer
{
public:
    //==========================================================================
    MainContentComponent (AudioProcessorPlayer& processorPlayer, bool isLowLatency)
        :   player (processorPlayer),
            keyboard (keyboardState, MidiKeyboardComponent::horizontalKeyboard),
            recordButton ("Record"), bluetoothButton ("Bluetooth"),
            roomSizeSlider (Slider::LinearHorizontal, Slider::NoTextBox)
    {
        keyboardState.addListener (&processorPlayer.getMidiMessageCollector());

        roomSizeSlider.setValue (getParameterValue ("roomSize"), NotificationType::dontSendNotification);

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

        Colour proAudioIconColour = findColour (isLowLatency ? TextButton::buttonOnColourId : TextButton::buttonColourId);
        proAudioIcon.setFill (FillType (proAudioIconColour));

        setSize (600, 400);
        startTimer (100);
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
            setParameterValue ("isRecording", 1.0f);
        }
        else if (button == &bluetoothButton)
        {
            BluetoothMidiDevicePairingDialogue::open();
        }
    }

    void sliderValueChanged (Slider*) override
    {
        setParameterValue ("roomSize", roomSizeSlider.getValue());
    }

private:
    //==========================================================================
    void timerCallback() override
    {
        bool isRecordingNow = (getParameterValue ("isRecording") >= 0.5f);

        recordButton.setEnabled (! isRecordingNow);
        roomSizeSlider.setValue (getParameterValue ("roomSize"), NotificationType::dontSendNotification);
    }

    //==========================================================================
    AudioProcessorParameter* getParameter (const String& paramId)
    {
        if (AudioProcessor* processor = player.getCurrentProcessor())
        {
            const OwnedArray<AudioProcessorParameter>& params = processor->getParameters();

            for (int i = 0; i < params.size(); ++i)
            {
                if (AudioProcessorParameterWithID* param = dynamic_cast<AudioProcessorParameterWithID*> (params[i]))
                {
                    if (param->paramID == paramId)
                        return param;
                }
            }
        }

        return nullptr;
    }

    //==========================================================================
    float getParameterValue (const String& paramId)
    {
        if (AudioProcessorParameter* param = getParameter (paramId))
            return param->getValue();

        return 0.0f;
    }

    void setParameterValue (const String& paramId, float value)
    {
        if (AudioProcessorParameter* param = getParameter (paramId))
            param->setValueNotifyingHost (value);
    }

    //==========================================================================
    AudioProcessorPlayer& player;

    //==========================================================================
    MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboard;
    TextButton recordButton, bluetoothButton;
    Slider roomSizeSlider;
    DrawablePath proAudioIcon;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
