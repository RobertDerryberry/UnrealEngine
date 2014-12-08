// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "IntroTutorialsPrivatePCH.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "Editor/MainFrame/Public/MainFrame.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "Editor/Kismet/Public/BlueprintEditorModule.h"
#include "../Plugins/Editor/EpicSurvey/Source/EpicSurvey/Public/IEpicSurveyModule.h"
#include "Editor/GameProjectGeneration/Public/GameProjectGenerationModule.h"
#include "Editor/UnrealEd/Public/EditorModes.h"
#include "Editor/UnrealEd/Public/SourceCodeNavigation.h"
#include "Editor/Matinee/Public/MatineeModule.h"

#include "EngineAnalytics.h"
#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProvider.h"
#include "Particles/ParticleSystem.h"
#include "EditorTutorialSettings.h"
#include "TutorialStateSettings.h"
#include "TutorialSettings.h"
#include "ISettingsModule.h"
#include "EditorTutorial.h"
#include "SEditorTutorials.h"
#include "STutorialsBrowser.h"
#include "STutorialNavigation.h"
#include "STutorialOverlay.h"
#include "TutorialStructCustomization.h"
#include "EditorTutorialDetailsCustomization.h"
#include "STutorialRoot.h"
#include "STutorialButton.h"
#include "ToolkitManager.h"
#include "BlueprintEditor.h"
#include "EngineBuildSettings.h"
#include "IDocumentation.h"
#include "SDockTab.h"
#include "ModuleManager.h"
#include "IntroTutorials.h"


#define LOCTEXT_NAMESPACE "IntroTutorials"

IMPLEMENT_MODULE(FIntroTutorials, IntroTutorials)


FIntroTutorials::FIntroTutorials()
	: CurrentObjectClass(nullptr)
	, ContentIntroCurve(nullptr)
{
	bDisableTutorials = false;

	bDesireResettingTutorialSeenFlagOnLoad = FParse::Param(FCommandLine::Get(), TEXT("ResetTutorials"));
}

FString FIntroTutorials::AnalyticsEventNameFromTutorial(UEditorTutorial* Tutorial)
{
	FString TutorialPath = Tutorial->GetOutermost()->GetFName().ToString();

	// strip off everything but the asset name
	FString RightStr;
	TutorialPath.Split( TEXT("/"), NULL, &RightStr, ESearchCase::IgnoreCase, ESearchDir::FromEnd );

	return RightStr;
}

TSharedRef<FExtender> FIntroTutorials::AddSummonBlueprintTutorialsMenuExtender(const TSharedRef<FUICommandList> CommandList, const TArray<UObject*> EditingObjects) const
{
	UObject* PrimaryObject = nullptr;
	if(EditingObjects.Num() > 0)
	{
		PrimaryObject = EditingObjects[0];
	}

	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"HelpBrowse",
		EExtensionHook::After,
		CommandList,
		FMenuExtensionDelegate::CreateRaw(this, &FIntroTutorials::AddSummonBlueprintTutorialsMenuExtension, PrimaryObject));

	return Extender;
}

void FIntroTutorials::StartupModule()
{
	// This code can run with content commandlets. Slate is not initialized with commandlets and the below code will fail.
	if (!bDisableTutorials && !IsRunningCommandlet())
	{
		// Add tutorial for main frame opening
		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		MainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &FIntroTutorials::MainFrameLoad);
		MainFrameModule.OnMainFrameSDKNotInstalled().AddRaw(this, &FIntroTutorials::HandleSDKNotInstalled);
		
		// Add menu option for level editor tutorial
		MainMenuExtender = MakeShareable(new FExtender);
		MainMenuExtender->AddMenuExtension("HelpBrowse", EExtensionHook::After, NULL, FMenuExtensionDelegate::CreateRaw(this, &FIntroTutorials::AddSummonTutorialsMenuExtension));
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MainMenuExtender);

		// Add menu option to blueprint editor as well
		FBlueprintEditorModule& BPEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>( "Kismet" );
		BPEditorModule.GetMenuExtensibilityManager()->GetExtenderDelegates().Add(FAssetEditorExtender::CreateRaw(this, &FIntroTutorials::AddSummonBlueprintTutorialsMenuExtender));

		// Add hook for when AddToCodeProject dialog window is opened
		FGameProjectGenerationModule::Get().OnAddCodeToProjectDialogOpened().AddRaw(this, &FIntroTutorials::OnAddCodeToProjectDialogOpened);

		// Add hook for New Project dialog window is opened
		//FGameProjectGenerationModule::Get().OnNewProjectProjectDialogOpened().AddRaw(this, &FIntroTutorials::OnNewProjectDialogOpened);

		FSourceCodeNavigation::AccessOnCompilerNotFound().AddRaw( this, &FIntroTutorials::HandleCompilerNotFound );
	
		// maybe reset all the "have I seen this once" flags
		if (bDesireResettingTutorialSeenFlagOnLoad)
		{
			GetMutableDefault<UTutorialStateSettings>()->ClearProgress();
		}
	}

	// Register to display our settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->RegisterSettings("Editor", "General", "Tutorials",
			LOCTEXT("EditorTutorialSettingsName", "Tutorials"),
			LOCTEXT("EditorTutorialSettingsDescription", "Control what tutorials are available in the Editor."),
			GetMutableDefault<UEditorTutorialSettings>()
		);

		SettingsModule->RegisterSettings("Project", "Engine", "Tutorials",
			LOCTEXT("TutorialSettingsName", "Tutorials"),
			LOCTEXT("TutorialSettingsDescription", "Control what tutorials are available in this project."),
			GetMutableDefault<UTutorialSettings>()
		);
	}

	// register details customizations
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("TutorialContent", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FTutorialContentCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("TutorialContentAnchor", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FTutorialContentAnchorCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("EditorTutorial", FOnGetDetailCustomizationInstance::CreateStatic(&FEditorTutorialDetailsCustomization::MakeInstance));

	UCurveFloat* ContentIntroCurveAsset = LoadObject<UCurveFloat>(nullptr, TEXT("/Engine/Tutorial/ContentIntroCurve.ContentIntroCurve"));
	if(ContentIntroCurveAsset)
	{
		ContentIntroCurveAsset->AddToRoot();
		ContentIntroCurve = ContentIntroCurveAsset;
	}

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TEXT("TutorialsBrowser"), FOnSpawnTab::CreateRaw(this, &FIntroTutorials::SpawnTutorialsBrowserTab))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FIntroTutorials::ShutdownModule()
{
	if (!bDisableTutorials && !IsRunningCommandlet())
	{
		FSourceCodeNavigation::AccessOnCompilerNotFound().RemoveAll( this );
	}

	if (BlueprintEditorExtender.IsValid() && FModuleManager::Get().IsModuleLoaded("Kismet"))
	{
		FBlueprintEditorModule& BPEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
		BPEditorModule.GetMenuExtensibilityManager()->RemoveExtender(BlueprintEditorExtender);
	}
	if (MainMenuExtender.IsValid() && FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
		LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MainMenuExtender);
	}
	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		MainFrameModule.OnMainFrameCreationFinished().RemoveAll(this);
	}

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Editor", "General", "Tutorials");
		SettingsModule->UnregisterSettings("Project", "Engine", "Tutorials");
	}

	if(FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("TutorialContent");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("TutorialWidgetReference");
		PropertyEditorModule.UnregisterCustomClassLayout("EditorTutorial");
	}

	if(ContentIntroCurve.IsValid())
	{
		ContentIntroCurve.Get()->RemoveFromRoot();
		ContentIntroCurve = nullptr;
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TEXT("TutorialsBrowser"));
}

void FIntroTutorials::AddSummonTutorialsMenuExtension(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("Tutorials", LOCTEXT("TutorialsLabel", "Tutorials"));
	MenuBuilder.AddMenuEntry(
		LOCTEXT("TutorialsMenuEntryTitle", "Tutorials"),
		LOCTEXT("TutorialsMenuEntryToolTip", "Opens up introductory tutorials covering the basics of using the Unreal Engine 4 Editor."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tutorials"),
		FUIAction(FExecuteAction::CreateRaw(this, &FIntroTutorials::SummonTutorialHome)));
	MenuBuilder.EndSection();
}

void FIntroTutorials::AddSummonBlueprintTutorialsMenuExtension(FMenuBuilder& MenuBuilder, UObject* PrimaryObject)
{
	MenuBuilder.BeginSection("Tutorials", LOCTEXT("TutorialsLabel", "Tutorials"));
	MenuBuilder.AddMenuEntry(
		LOCTEXT("BlueprintMenuEntryTitle", "Blueprint Overview"),
		LOCTEXT("BlueprintMenuEntryToolTip", "Opens up an introductory overview of Blueprints."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tutorials"),
		FUIAction(FExecuteAction::CreateRaw(this, &FIntroTutorials::SummonBlueprintTutorialHome, PrimaryObject, true)));

	if(PrimaryObject != nullptr)
	{
		UBlueprint* BP = Cast<UBlueprint>(PrimaryObject);
		if(BP != nullptr)
		{
			UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBlueprintType"), true);
			check(Enum);
			MenuBuilder.AddMenuEntry(
				FText::Format(LOCTEXT("BlueprintTutorialsMenuEntryTitle", "{0} Tutorial"), Enum->GetEnumText(BP->BlueprintType)),
				LOCTEXT("BlueprintTutorialsMenuEntryToolTip", "Opens up an introductory tutorial covering this particular part of the Blueprint editor."),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tutorials"),
				FUIAction(FExecuteAction::CreateRaw(this, &FIntroTutorials::SummonBlueprintTutorialHome, PrimaryObject, false)));
		}
	}

	MenuBuilder.EndSection();
}

void FIntroTutorials::MainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow)
{
	if (!bIsNewProjectWindow)
	{
		// install a root widget for the tutorial overlays to hang off
		if(InRootWindow.IsValid() && !TutorialRoot.IsValid())
		{
			InRootWindow->AddOverlaySlot()
				[
					SAssignNew(TutorialRoot, STutorialRoot)
				];
		}

		// See if we should show 'welcome' screen
		MaybeOpenWelcomeTutorial();
	}
}

void FIntroTutorials::SummonTutorialHome()
{
	SummonTutorialBrowser();
}

void FIntroTutorials::SummonBlueprintTutorialHome(UObject* Asset, bool bForceWelcome)
{
	UBlueprint* BP = CastChecked<UBlueprint>(Asset);

	FName Context("BlueprintOverview");
	if(!bForceWelcome)
	{
		Context = FBlueprintEditor::GetContextFromBlueprintType(BP->BlueprintType);
	}

	UEditorTutorial* AttractTutorial = nullptr;
	UEditorTutorial* LaunchTutorial = nullptr;
	FString BrowserFilter;
	GetDefault<UEditorTutorialSettings>()->FindTutorialInfoForContext(Context, AttractTutorial, LaunchTutorial, BrowserFilter);

	FIntroTutorials& IntroTutorials = FModuleManager::GetModuleChecked<FIntroTutorials>(TEXT("IntroTutorials"));

	if (LaunchTutorial != nullptr)
	{
		TSharedPtr<SWindow> ContextWindow;
		TSharedPtr< IToolkit > Toolkit = FToolkitManager::Get().FindEditorForAsset(Asset);
		if(ensure(Toolkit.IsValid()))
		{
			ContextWindow = FSlateApplication::Get().FindWidgetWindow(Toolkit->GetToolkitHost()->GetParentWidget());
			check(ContextWindow.IsValid());
		}

		const bool bRestart = true;
		IntroTutorials.LaunchTutorial(LaunchTutorial, bRestart, ContextWindow);
	}
}

bool FIntroTutorials::MaybeOpenWelcomeTutorial()
{
	if(FParse::Param(FCommandLine::Get(), TEXT("TestTutorialAlerts")) || !FEngineBuildSettings::IsInternalBuild())
	{
		// try editor startup tutorial
		TSubclassOf<UEditorTutorial> EditorStartupTutorialClass = LoadClass<UEditorTutorial>(NULL, *GetDefault<UEditorTutorialSettings>()->StartupTutorial.AssetLongPathname, NULL, LOAD_None, NULL);
		if(EditorStartupTutorialClass != nullptr)
		{
			UEditorTutorial* Tutorial = EditorStartupTutorialClass->GetDefaultObject<UEditorTutorial>();
			if (!GetDefault<UTutorialStateSettings>()->HaveSeenTutorial(Tutorial))
			{
				LaunchTutorial(Tutorial);
				return true;
			}
		}

		// Try project startup tutorial
		TSubclassOf<UEditorTutorial> ProjectStartupTutorialClass = LoadClass<UEditorTutorial>(NULL, *GetDefault<UTutorialSettings>()->StartupTutorial.AssetLongPathname, NULL, LOAD_None, NULL);
		if(ProjectStartupTutorialClass != nullptr)
		{
			UEditorTutorial* Tutorial = ProjectStartupTutorialClass->GetDefaultObject<UEditorTutorial>();
			if (!GetDefault<UTutorialStateSettings>()->HaveSeenTutorial(Tutorial))
			{
				LaunchTutorial(Tutorial);
				return true;
			}
		}
	}

	return false;
}

void FIntroTutorials::OnAddCodeToProjectDialogOpened()
{
	// @todo: add code to project dialog tutorial here?
//	MaybeOpenWelcomeTutorial(AddCodeToProjectWelcomeTutorial);
}

void FIntroTutorials::OnNewProjectDialogOpened()
{
	// @todo: new project dialog tutorial here?
//	MaybeOpenWelcomeTutorial(TemplateOverview);
}

void FIntroTutorials::HandleCompilerNotFound()
{
#if PLATFORM_WINDOWS
	LaunchTutorialByName( TEXT( "Engine/Tutorial/Installation/InstallingVisualStudioTutorial.InstallingVisualStudioTutorial" ) );
#elif PLATFORM_MAC
	LaunchTutorialByName( TEXT( "Engine/Tutorial/Installation/InstallingXCodeTutorial.InstallingXCodeTutorial" ) );
#else
	STUBBED("FIntroTutorials::HandleCompilerNotFound");
#endif
}

void FIntroTutorials::HandleSDKNotInstalled(const FString& PlatformName, const FString& InTutorialAsset)
{
	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InTutorialAsset);
	if(Blueprint)
	{
		LaunchTutorialByName( InTutorialAsset );
	}
	else
	{
		IDocumentation::Get()->Open( InTutorialAsset );
	}
}

EVisibility FIntroTutorials::GetHomeButtonVisibility() const
{
	if(CurrentObjectClass != nullptr)
	{
		return CurrentObjectClass->IsChildOf(UBlueprint::StaticClass()) ? EVisibility::Hidden : EVisibility::Visible;
	}
	
	return EVisibility::Visible;
}

FOnIsPicking& FIntroTutorials::OnIsPicking()
{
	return OnIsPickingDelegate;
}

FOnValidatePickingCandidate& FIntroTutorials::OnValidatePickingCandidate()
{
	return OnValidatePickingCandidateDelegate;
}

void FIntroTutorials::SummonTutorialBrowser()
{
	if(TutorialRoot.IsValid())
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>( TEXT("LevelEditor") );
		LevelEditorModule.GetLevelEditorTabManager()->InvokeTab(FTabId("TutorialsBrowser"));
	}
}

void FIntroTutorials::LaunchTutorial(const FString& TutorialAssetName)
{
	LaunchTutorialByName(TutorialAssetName);
}

void FIntroTutorials::LaunchTutorialByName(const FString& InAssetPath, bool bInRestart, TWeakPtr<SWindow> InNavigationWindow, FSimpleDelegate OnTutorialClosed, FSimpleDelegate OnTutorialExited)
{
	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InAssetPath);
	if (Blueprint && Blueprint->GeneratedClass)
	{
		LaunchTutorial(Blueprint->GeneratedClass->GetDefaultObject<UEditorTutorial>(), bInRestart, InNavigationWindow, OnTutorialClosed, OnTutorialExited);
	}
}

void FIntroTutorials::LaunchTutorial(UEditorTutorial* InTutorial, bool bInRestart, TWeakPtr<SWindow> InNavigationWindow, FSimpleDelegate OnTutorialClosed, FSimpleDelegate OnTutorialExited)
{
	if(TutorialRoot.IsValid())
	{
		if(!InNavigationWindow.IsValid())
		{
			IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
			InNavigationWindow = MainFrameModule.GetParentWindow();
		}

		TutorialRoot->LaunchTutorial(InTutorial, bInRestart, InNavigationWindow, OnTutorialClosed, OnTutorialExited);
	}
}

void FIntroTutorials::CloseAllTutorialContent()
{
	if (TutorialRoot.IsValid())
	{
		TutorialRoot->CloseAllTutorialContent();
	}
}

void FIntroTutorials::GoToPreviousStage()
{
	if (TutorialRoot.IsValid())
	{
		TutorialRoot->GoToPreviousStage();
	}
}

void FIntroTutorials::GoToNextStage(TWeakPtr<SWindow> InNavigationWindow)
{
	if (TutorialRoot.IsValid())
	{
		TutorialRoot->GoToNextStage(InNavigationWindow);
	}
}

TSharedRef<SWidget> FIntroTutorials::CreateTutorialsWidget(FName InContext, TWeakPtr<SWindow> InContextWindow) const
{
	return SNew(STutorialButton)
		.Context(InContext)
		.ContextWindow(InContextWindow);
}

float FIntroTutorials::GetIntroCurveValue(float InTime)
{
	if(ContentIntroCurve.IsValid())
	{
		return ContentIntroCurve.Get()->GetFloatValue(InTime);
	}

	return 1.0f;
}

TSharedRef<SDockTab> FIntroTutorials::SpawnTutorialsBrowserTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TAttribute<FText> Label = LOCTEXT("TutorialsBrowserTabLabel", "Tutorials");

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(Label)
		.ToolTip(IDocumentation::Get()->CreateToolTip(Label, nullptr, "Shared/TutorialsBrowser", "Tab"));	

	TSharedRef<STutorialsBrowser> TutorialsBrowser = SNew(STutorialsBrowser)
		.OnLaunchTutorial(FOnLaunchTutorial::CreateRaw(this, &FIntroTutorials::LaunchTutorial));

	NewTab->SetContent(TutorialsBrowser);

	return NewTab;
}

#undef LOCTEXT_NAMESPACE