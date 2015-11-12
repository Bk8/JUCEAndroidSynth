/*
  ==============================================================================

    Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"
#include "AndroidSynthProcessor.h"

//==============================================================================
class AndroidSynthApplication  : public  JUCEApplication,
                                 private Timer
{
public:
    //==============================================================================
    AndroidSynthApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        ignoreUnused (commandLine);

        player.setProcessor (&synthProcessor);

        deviceManager = new AudioDeviceManager();
        String err = deviceManager->initialiseWithDefaultDevices (1, 1);
        jassert (err.isEmpty());

        deviceManager->addAudioCallback (&player);
        deviceManager->addMidiInputCallback (String(), &player);

        mainWindow = new MainWindow (synthProcessor, getApplicationName());

        startTimer (1000);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
        ignoreUnused (commandLine);
    }

    //==============================================================================
    class MainWindow    : public DocumentWindow
    {
    public:
        MainWindow (AndroidSynthProcessor& synth, String name)  : DocumentWindow (name,
                                                    LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                                                    DocumentWindow::allButtons)
        {
            MainContentComponent* comp;

            setUsingNativeTitleBar (true);
            setContentOwned (comp = new MainContentComponent (synth), true);

           #if JUCE_ANDROID
            setFullScreen (true);
           #else
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        ~MainWindow()
        {
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    //==============================================================================
    void timerCallback() override
    {
        StringArray newDevices = MidiInput::getDevices();

        for (int i = 0; i < lastMidiDevices.size(); ++i)
            if (newDevices.indexOf (lastMidiDevices[i]) < 0)
                deviceManager->setMidiInputEnabled (lastMidiDevices[i], false);

        for (int i = 0; i < newDevices.size(); ++i)
            if (lastMidiDevices.indexOf (newDevices[i]) < 0)
                deviceManager->setMidiInputEnabled (newDevices[i], true);

        lastMidiDevices = newDevices;
    }

    //==============================================================================
    AndroidSynthProcessor synthProcessor;
    AudioProcessorPlayer player;
    ScopedPointer<AudioDeviceManager> deviceManager;
    StringArray lastMidiDevices;

    //==============================================================================
    ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (AndroidSynthApplication)
