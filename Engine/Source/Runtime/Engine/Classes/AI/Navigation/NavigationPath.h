// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "AI/Navigation/NavigationTypes.h"
#include "NavigationPath.generated.h"

class UNavigationPath;
struct FNavigationPath;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNavigationPathUpdated, UNavigationPath*, AffectedPath, TEnumAsByte<ENavPathEvent::Type>, PathEvent);

/**
 *	UObject wrapper for FNavigationPath
 */

UCLASS(BlueprintType)
class ENGINE_API UNavigationPath : public UObject
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FOnNavigationPathUpdated PathUpdatedNotifier;

	UPROPERTY(BlueprintReadOnly, Category = Navigation)
	TArray<FVector> PathPoints;

	UPROPERTY(BlueprintReadOnly, Category = Navigation)
	TEnumAsByte<ENavigationOptionFlag::Type> RecalculateOnInvalidation;

private:	
	uint32 bIsValid : 1;
	uint32 bDebugDrawingEnabled : 1;
	FColor DebugDrawingColor;

protected:
	FNavPathSharedPtr SharedPath;

	FNavigationPath::FPathObserverDelegate::FDelegate PathObserver;

public:

	// UObject begin
	virtual void BeginDestroy() override;
	// UObject end

	UFUNCTION(BlueprintCallable, Category = "AI|Debug")
	FString GetDebugString() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Debug")
	void EnableDebugDrawing(bool bShouldDrawDebugData, FLinearColor PathColor = FLinearColor::White);

	/** if enabled path will request recalculation if it gets invalidated due to a change to underlying navigation */
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	void EnableRecalculationOnInvalidation(TEnumAsByte<ENavigationOptionFlag::Type> DoRecalculation);

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	float GetPathLength() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	float GetPathCost() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	bool IsPartial() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	bool IsValid() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	bool IsStringPulled() const;
		
	void SetPath(FNavPathSharedPtr NewSharedPath);
	FNavPathSharedPtr GetPath() { return SharedPath; }

protected:
	void DrawDebug(UCanvas* Canvas, APlayerController*);
	void OnPathEvent(FNavigationPath* Path, ENavPathEvent::Type PathEvent);

	void SetPathPointsFromPath(FNavigationPath& NativePath);
};
