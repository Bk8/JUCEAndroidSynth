/*
  ==============================================================================

    Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"


//==============================================================================
class AndroidSynthApplication  : public JUCEApplication
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
        mainWindow = new MainWindow (getApplicationName());
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
        MainWindow (String name)  : DocumentWindow (name,
                                                    Colours::lightgrey,
                                                    DocumentWindow::allButtons)
        {
            MainContentComponent* comp;

            setUsingNativeTitleBar (true);
            setContentOwned (comp = new MainContentComponent(), true);

           #if JUCE_ANDROID
            setFullScreen (true);
           #else
            centreWithSize (getWidth(), getHeight());
           #endif

            glContext.attachTo (*comp);
            setVisible (true);
        }

        ~MainWindow()
        {
            glContext.detach();
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        OpenGLContext glContext;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (AndroidSynthApplication)
