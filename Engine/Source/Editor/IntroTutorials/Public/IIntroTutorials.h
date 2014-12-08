// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

class UEditorTutorial;

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class IIntroTutorials : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IIntroTutorials& Get()
	{
		return FModuleManager::LoadModuleChecked< IIntroTutorials >( "IntroTutorials" );
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "IntroTutorials" );
	}

	/**
	 * Launch a tutorial with the specified asset.
	 *
	 * @param TutorialAssetName The name of the tutorial asset.
	 */
	virtual void LaunchTutorial(const FString& TutorialAssetName) = 0;

	/**
	 * Launch a tutorial immediately, bypassing the tutorial browser.
	 * 
	 * @param	Tutorial	The tutorial to launch
	 * @param	bRestart	Whether to restart the tutorial or resume from where we left off last time.
	 * @param	InNavigationWindow	Optional window to launch the tutorial from - this is where navigation will be displayed.
	 */
	virtual void LaunchTutorial(UEditorTutorial* Tutorial, bool bRestart = true, TWeakPtr<SWindow> InNavigationWindow = nullptr, FSimpleDelegate OnTutorialClosed = FSimpleDelegate(), FSimpleDelegate OnTutorialExited = FSimpleDelegate()) = 0;

	/**
 	 *  Close all tutorial content, including the browser.
	 */
	virtual void CloseAllTutorialContent() = 0;

	/**
	 * Create a widget that allows access to the tutorial for the current context.
	 * @param	InContext			The name of the context this widget is attached to (e.g. "LevelEditor")
	 * @param	InContextWindow		The window that the context is attached to (e.g. the main window, or an asset editor tab)
	 * @return a widget used to access context-sensitive tutorials
	 */
	virtual TSharedRef<SWidget> CreateTutorialsWidget(FName InContext, TWeakPtr<SWindow> InContextWindow = nullptr) const = 0;
};

