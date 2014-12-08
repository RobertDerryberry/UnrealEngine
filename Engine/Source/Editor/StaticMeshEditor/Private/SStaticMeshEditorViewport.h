// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PreviewScene.h"
#include "SEditorViewport.h"

/**
 * StaticMesh Editor Preview viewport widget
 */
class SStaticMeshEditorViewport : public SEditorViewport, public FGCObject
{
public:
	SLATE_BEGIN_ARGS( SStaticMeshEditorViewport ){}
		SLATE_ARGUMENT(TWeakPtr<IStaticMeshEditor>, StaticMeshEditor)
		SLATE_ARGUMENT(UStaticMesh*, ObjectToEdit)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SStaticMeshEditorViewport();
	
	// FGCObject interface
	virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;
	// End of FGCObject interface

	/** Constructs, destroys, and updates preview mesh components based on the preview static mesh's sockets. */
	void UpdatePreviewSocketMeshes();

	void RefreshViewport();
	
	/** Component for the preview static mesh. */
	UStaticMeshComponent* PreviewMeshComponent;

	/** Component for the preview static mesh. */
	TArray<UStaticMeshComponent*> SocketPreviewMeshComponents;

	/** 
	 *	Forces a specific LOD level onto the static mesh component.
	 *
	 *	@param	InForcedLOD			The desired LOD to be forced to.
	 */
	void ForceLODLevel(int32 InForcedLOD);

	/** Retrieves the static mesh component. */
	UStaticMeshComponent* GetStaticMeshComponent() const;

	/** 
	 *	Sets up the static mesh that the Static Mesh editor is viewing.
	 *
	 *	@param	InStaticMesh		The static mesh being viewed in the editor.
	 */
	void SetPreviewMesh(UStaticMesh* InStaticMesh);

	/**
	 *	Updates the preview mesh and other viewport specfic settings that go with it.
	 *
	 *	@param	InStaticMesh		The static mesh being viewed in the editor.
	 */
	void UpdatePreviewMesh(UStaticMesh* InStaticMesh);

	/** Retrieves the selected edge set. */
	TSet< int32 >& GetSelectedEdges();

	/** @return The editor viewport client */
	class FStaticMeshEditorViewportClient& GetViewportClient();

	/** Set the parent tab of the viewport for determining visibility */
	void SetParentTab( TSharedRef<SDockTab> InParentTab ) { ParentTab = InParentTab; }

	/** Struct defining the text and its style of each item in the overlay widget */
	struct FOverlayTextItem
	{
		explicit FOverlayTextItem(const FText& InText, const FName& InStyle = "TextBlock.ShadowedText")
			: Text(InText), Style(InStyle)
		{}

		FText Text;
		FName Style;
	};

	/** Specifies an array of text items which will be added to the viewport overlay */
	void PopulateOverlayText( const TArray<FOverlayTextItem>& TextItems );

protected:
	/** SEditorViewport interface */
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual EVisibility OnGetViewportContentVisibility() const override;
	virtual void BindCommands() override;
	virtual void OnFocusViewportToSelection() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

private:
	/** Determines the visibility of the viewport. */
	bool IsVisible() const;

	/** Callback for toggling the wireframe mode flag. */
	void SetViewModeWireframe();
	
	/** Callback for checking the wireframe mode flag. */
	bool IsInViewModeWireframeChecked() const;

	/** Callback for toggling the vertex color show flag. */
	void SetViewModeVertexColor();

	/** Callback for checking the vertex color show flag. */
	bool IsInViewModeVertexColorChecked() const;

	/** Callback for toggling the realtime preview flag. */
	void SetRealtimePreview();

	/** Callback for updating preview socket meshes if the static mesh or socket has been modified. */
	void OnObjectPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);
private:
	
	/** The parent tab where this viewport resides */
	TWeakPtr<SDockTab> ParentTab;

	/** Pointer back to the StaticMesh editor tool that owns us */
	TWeakPtr<IStaticMeshEditor> StaticMeshEditorPtr;

	/** The scene for this viewport. */
	FPreviewScene PreviewScene;

	/** Level viewport client */
	TSharedPtr<class FStaticMeshEditorViewportClient> EditorViewportClient;

	/** Static mesh being edited */
	UStaticMesh* StaticMesh;

	/** The currently selected view mode. */
	EViewModeIndex CurrentViewMode;

	/** Pointer to the vertical box into which the overlay text items are added */
	TSharedPtr<SVerticalBox> OverlayTextVerticalBox;
};
