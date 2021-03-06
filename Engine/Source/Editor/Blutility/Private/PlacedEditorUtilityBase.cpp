// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PlacedEditorUtilityBase.h"
#include "Engine/Selection.h"
#include "Editor.h"
#include "LevelEditorViewport.h"

/////////////////////////////////////////////////////
// APlacedEditorUtilityBase

APlacedEditorUtilityBase::APlacedEditorUtilityBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	HelpText = TEXT("Please fill out the help text");
}

void APlacedEditorUtilityBase::TickActor(float DeltaSeconds, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	FEditorScriptExecutionGuard ScriptGuard;
	Super::TickActor(DeltaSeconds, TickType, ThisTickFunction);
}

TArray<AActor*> APlacedEditorUtilityBase::GetSelectionSet()
{
	TArray<AActor*> Result;

#if WITH_EDITOR
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			Result.Add(Actor);
		}
	}
#endif //WITH_EDITOR

	return Result;
}

bool APlacedEditorUtilityBase::GetLevelViewportCameraInfo(FVector& CameraLocation, FRotator& CameraRotation)
{
	bool RetVal = false;
	CameraLocation = FVector::ZeroVector;
	CameraRotation = FRotator::ZeroRotator;

#if WITH_EDITOR
	for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
	{
		if (LevelVC && LevelVC->IsPerspective())
		{
			CameraLocation = LevelVC->GetViewLocation();
			CameraRotation = LevelVC->GetViewRotation();
			RetVal = true;

			break;
		}
	}
#endif //WITH_EDITOR

	return RetVal;
}


void APlacedEditorUtilityBase::SetLevelViewportCameraInfo(FVector CameraLocation, FRotator CameraRotation)
{

#if WITH_EDITOR
	for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
	{
		if (LevelVC && LevelVC->IsPerspective())
		{
			LevelVC->SetViewLocation(CameraLocation);
			LevelVC->SetViewRotation(CameraRotation);

			break;
		}
	}
#endif //WITH_EDITOR
}

void APlacedEditorUtilityBase::ClearActorSelectionSet()
{
	GEditor->GetSelectedActors()->DeselectAll();
	GEditor->NoteSelectionChange();
}

void APlacedEditorUtilityBase::SelectNothing()
{
	GEditor->SelectNone(true, true, false);
}

void APlacedEditorUtilityBase::SetActorSelectionState(AActor* Actor, bool bShouldBeSelected)
{
	GEditor->SelectActor(Actor, bShouldBeSelected, /*bNotify=*/ false);
}

AActor* APlacedEditorUtilityBase::GetActorReference(FString PathToActor)
{
#if WITH_EDITOR
	return Cast<AActor>(StaticFindObject(AActor::StaticClass(), GEditor->GetEditorWorldContext().World(), *PathToActor, false));
#else
	return nullptr;
#endif //WITH_EDITOR
}
