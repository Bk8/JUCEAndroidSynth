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
                               public ChangeListener,
                               public ButtonListener,
                               public Slider::Listener
{
public:
    //==========================================================================
    MainContentComponent ()
        : keyboard (keyboardState, MidiKeyboardComponent::horizontalKeyboard),
          recordButton ("Record"),
          roomSizeSlider (Slider::LinearHorizontal, Slider::NoTextBox)
    {
        keyboard.setLowestVisibleKey (0x30);
        keyboard.setKeyWidth (600/0x10);
        addAndMakeVisible (keyboard);

        recordButton.addListener (this);
        addAndMakeVisible (recordButton);

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
        roomSizeSlider.setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
    }

    //==========================================================================
    void buttonClicked (Button* button) override
    {
        if (button == &recordButton)
        {
            // ....
        }
    }

    void sliderValueChanged (Slider*) override
    {
    }

    //==========================================================================
    void changeListenerCallback (ChangeBroadcaster*) override
    {
    }
private:
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
