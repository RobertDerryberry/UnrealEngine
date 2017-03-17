// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "MeshEditorModule.h"
#include "EditorModeRegistry.h"
#include "ModuleManager.h"
#include "MeshEditorMode.h"
#include "MeshEditorStyle.h"
#include "EditorModeManager.h"
#include "ISettingsModule.h"
#include "MeshEditorSettings.h"
#include "MultiBoxBuilder.h"
#include "MultiBoxExtender.h"
#include "IVREditorModule.h"
#include "VREditorMode.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "MeshEditor"

class FMeshEditorModule : public IModuleInterface
{
public:
	FMeshEditorModule()
	{
	}

	// FModuleInterface overrides
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

protected:

	/** Stores the FEditorMode ID of the associated editor mode */
	static inline FEditorModeID GetEditorModeID()
	{
		static FEditorModeID MeshEditorFeatureName = FName( TEXT( "MeshEditor" ) );
		return MeshEditorFeatureName;
	}

	/** Adds items to the VR Radial menu for mesh editing mode */
	void FillVRRadialMenuModes( class FMenuBuilder& MenuBuilder );

	/** Changes the editor mode to the given ID */
	void OnMeshEditModeButtonClicked( EEditableMeshElementType InMode );

	/** Checks whether the editor mode for the given ID is active*/
	ECheckBoxState IsMeshEditModeButtonChecked( EEditableMeshElementType InMode );

	/** Should the mesh edit button be enabled */
	bool IsMeshEditModeButtonEnabled( EEditableMeshElementType InMode );


	/** Menu extension for the VR Editor's 'Modes' menu */
	TSharedPtr<const class FExtensionBase> VRRadialMenuModesExtension;

};


void FMeshEditorModule::StartupModule()
{
	FMeshEditorStyle::Initialize();

	FEditorModeRegistry::Get().RegisterMode<FMeshEditorMode>(
		GetEditorModeID(),
		LOCTEXT( "ModeName", "Mesh Editor" ),
		FSlateIcon( FMeshEditorStyle::GetStyleSetName(), "LevelEditor.MeshEditorMode", "LevelEditor.MeshEditorMode.Small" ),
		true,
		600
		);

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" );
	if( SettingsModule )
	{
		// Designer settings
		SettingsModule->RegisterSettings( "Editor", "ContentEditors", "MeshEditor",
										  LOCTEXT("MeshEditorSettingsName", "Mesh Editor"),
										  LOCTEXT("MeshEditorSettingsDescription", "Configure options for the Mesh Editor."),
										  GetMutableDefault<UMeshEditorSettings>()
		);
	}

	{
		FExtender& RadialMenuExtender = *IVREditorModule::Get().GetRadialMenuExtender();
		VRRadialMenuModesExtension = RadialMenuExtender.AddMenuExtension(
			"Modes",
			EExtensionHook::After,
			nullptr, // No UI commands needed for switching modes.  They're all handled directly by callback
			FMenuExtensionDelegate::CreateRaw( this, &FMeshEditorModule::FillVRRadialMenuModes ) );
	}
}


void FMeshEditorModule::ShutdownModule()
{
	{
		if( IVREditorModule::IsAvailable() )
		{
			FExtender& RadialMenuExtender = *IVREditorModule::Get().GetRadialMenuExtender();
			RadialMenuExtender.RemoveExtension( VRRadialMenuModesExtension.ToSharedRef() );
		}
		VRRadialMenuModesExtension = nullptr;
	}

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" );
	if( SettingsModule )
	{
		SettingsModule->UnregisterSettings( "Editor", "ContentEditors", "MeshEditor" );
	}

	FEditorModeRegistry::Get().UnregisterMode(GetEditorModeID());

	FMeshEditorStyle::Shutdown();
}


void FMeshEditorModule::FillVRRadialMenuModes( FMenuBuilder& MenuBuilder )
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("Mesh", "Mesh"),
		FText(),
		FSlateIcon(FMeshEditorStyle::GetStyleSetName(), "MeshEditorMode.MeshEditMode"),
		FUIAction
		(
			FExecuteAction::CreateRaw(this, &FMeshEditorModule::OnMeshEditModeButtonClicked, EEditableMeshElementType::Any),
			FCanExecuteAction::CreateRaw(this, &FMeshEditorModule::IsMeshEditModeButtonEnabled, EEditableMeshElementType::Any ),
			FGetActionCheckState::CreateRaw(this, &FMeshEditorModule::IsMeshEditModeButtonChecked, EEditableMeshElementType::Any)
			),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
		);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("Polygon", "Polygon"),
		FText(),
		FSlateIcon(FMeshEditorStyle::GetStyleSetName(), "MeshEditorMode.PolygonEditMode"),
		FUIAction
		(
			FExecuteAction::CreateRaw(this, &FMeshEditorModule::OnMeshEditModeButtonClicked, EEditableMeshElementType::Polygon),
			FCanExecuteAction::CreateRaw(this, &FMeshEditorModule::IsMeshEditModeButtonEnabled, EEditableMeshElementType::Polygon ),
			FGetActionCheckState::CreateRaw(this, &FMeshEditorModule::IsMeshEditModeButtonChecked, EEditableMeshElementType::Polygon)
			),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
		);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("Edge", "Edge"),
		FText(),
		FSlateIcon(FMeshEditorStyle::GetStyleSetName(), "MeshEditorMode.EdgeEditMode"),
		FUIAction
		(
			FExecuteAction::CreateRaw(this, &FMeshEditorModule::OnMeshEditModeButtonClicked, EEditableMeshElementType::Edge),
			FCanExecuteAction::CreateRaw(this, &FMeshEditorModule::IsMeshEditModeButtonEnabled, EEditableMeshElementType::Edge ),
			FGetActionCheckState::CreateRaw(this, &FMeshEditorModule::IsMeshEditModeButtonChecked, EEditableMeshElementType::Edge)
			),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
		);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("Vertex", "Vertex"),
		FText(),
		FSlateIcon(FMeshEditorStyle::GetStyleSetName(), "MeshEditorMode.VertexEditMode"),
		FUIAction
		(
			FExecuteAction::CreateRaw(this, &FMeshEditorModule::OnMeshEditModeButtonClicked, EEditableMeshElementType::Vertex),
			FCanExecuteAction::CreateRaw(this, &FMeshEditorModule::IsMeshEditModeButtonEnabled, EEditableMeshElementType::Vertex ),
			FGetActionCheckState::CreateRaw(this, &FMeshEditorModule::IsMeshEditModeButtonChecked, EEditableMeshElementType::Vertex)
			),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
		);
}


void FMeshEditorModule::OnMeshEditModeButtonClicked(EEditableMeshElementType InMode)
{
	// *Important* - activate the mode first since FEditorModeTools::DeactivateMode will
	// activate the default mode when the stack becomes empty, resulting in multiple active visible modes.
	GLevelEditorModeTools().ActivateMode(GetEditorModeID());

	// Find and disable any other 'visible' modes since we only ever allow one of those active at a time.
	TArray<FEdMode*> ActiveModes;
	GLevelEditorModeTools().GetActiveModes(ActiveModes);
	for (FEdMode* Mode : ActiveModes)
	{
		if (Mode->GetID() != GetEditorModeID() && Mode->GetModeInfo().bVisible)
		{
			GLevelEditorModeTools().DeactivateMode(Mode->GetID());
		}
	}

	FMeshEditorMode* MeshEditorMode = static_cast<FMeshEditorMode*>( GLevelEditorModeTools().FindMode( GetEditorModeID() ) );
	if (MeshEditorMode != nullptr)
	{
		MeshEditorMode->SetMeshElementSelectionMode(InMode);

		UVREditorMode* VREditorMode = IVREditorModule::Get().GetVRMode();
		if( VREditorMode != nullptr )
		{
			VREditorMode->RefreshRadialMenuActionsSubmenu();
		}
	}
}

ECheckBoxState FMeshEditorModule::IsMeshEditModeButtonChecked(EEditableMeshElementType InMode)
{
	bool bMeshModeActive = false;

	FMeshEditorMode* MeshEditorMode = static_cast<FMeshEditorMode*>( GLevelEditorModeTools().FindMode( GetEditorModeID() ) );
	if( MeshEditorMode != nullptr )
	{
		bMeshModeActive = MeshEditorMode->GetMeshElementSelectionMode() == InMode;
	}
	return bMeshModeActive ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}


bool FMeshEditorModule::IsMeshEditModeButtonEnabled( EEditableMeshElementType InMode )
{
	return true;
}

IMPLEMENT_MODULE( FMeshEditorModule, MeshEditor )

#undef LOCTEXT_NAMESPACE
