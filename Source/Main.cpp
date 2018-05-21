/*==============================================================================

  Copyright 2018 by Tracktion Corporation.
  For more information visit www.tracktion.com

   You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   pluginval IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

 ==============================================================================*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"
#include "Validator.h"
#include "CommandLine.h"

//==============================================================================
class PluginValidatorApplication  : public JUCEApplication,
                                    private AsyncUpdater
{
public:
    //==============================================================================
    PluginValidatorApplication() = default;

    PropertiesFile& getAppPreferences()
    {
        jassert (propertiesFile); // Calling this from the child process?
        return *propertiesFile;
    }

    //==============================================================================
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        if (shouldPerformCommandLine (commandLine))
        {
            triggerAsyncUpdate();
            return;
        }

        if (invokeSlaveProcessValidator (commandLine))
            return;

        validator = std::make_unique<Validator>();
        propertiesFile.reset (getPropertiesFile());
        mainWindow = std::make_unique<MainWindow> (*validator, getApplicationName());
    }

    void shutdown() override
    {
        mainWindow.reset();
        validator.reset();
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const String&) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public DocumentWindow
    {
    public:
        MainWindow (Validator& v, String name)
            : DocumentWindow (name,
                              Desktop::getInstance().getDefaultLookAndFeel()
                                .findColour (ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (v), true);

            setResizable (true, false);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<Validator> validator;
    std::unique_ptr<PropertiesFile> propertiesFile;
    std::unique_ptr<MainWindow> mainWindow;

    static PropertiesFile* getPropertiesFile()
    {
        PropertiesFile::Options opts;
        opts.millisecondsBeforeSaving = 2000;
        opts.storageFormat = PropertiesFile::storeAsXML;

        opts.applicationName = "PluginValidator";
        opts.filenameSuffix = ".xml";
        opts.folderName = "PluginValidator";
        opts.osxLibrarySubFolder = "Application Support";

        return new PropertiesFile (opts.getDefaultFile(), opts);
    }

	void handleAsyncUpdate() override
	{
		if (performCommandLine (JUCEApplication::getCommandLineParameters()) != commandLineNotPerformed)
			quit();
    }
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (PluginValidatorApplication)

PropertiesFile& getAppPreferences()
{
    auto app = dynamic_cast<PluginValidatorApplication*> (PluginValidatorApplication::getInstance());
    return app->getAppPreferences();
}