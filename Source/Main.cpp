/*
  ==============================================================================

    Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"
#include "AndroidSynthProcessor.h"

//==============================================================================
class AndroidSynthApplication  : public  JUCEApplication
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

        String err = deviceManager.initialiseWithDefaultDevices (1, 1);
        jassert (err.isEmpty());

        deviceManager.addAudioCallback (&player);
        deviceManager.addMidiInputCallback (String(), &player);

        mainWindow = new MainWindow (player, getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;

        deviceManager.removeMidiInputCallback (String(), &player);
        deviceManager.removeAudioCallback (&player);

        player.setProcessor (nullptr);
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
        MainWindow (AudioProcessorPlayer& player, String name)  : DocumentWindow (name,
                                                                                 LookAndFeel::getDefaultLookAndFeel().
                                                                                 findColour (ResizableWindow::backgroundColourId),
                                                                                 DocumentWindow::allButtons)
        {
            MainContentComponent* comp;

            setUsingNativeTitleBar (true);
            setContentOwned (comp = new MainContentComponent (player), true);

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
    AudioDeviceManager deviceManager;
    AndroidSynthProcessor synthProcessor;
    AudioProcessorPlayer player;

    //==============================================================================
    ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (AndroidSynthApplication)
