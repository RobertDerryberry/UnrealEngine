// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"

#include "PaperTileMapActor.generated.h"

/**
 * An instance of a UPaperTileMap in a level.
 *
 * This actor is created when you drag a tile map asset from the content browser into the level, and
 * it is just a thin wrapper around a UPaperTileMapRenderComponent that actually references the asset.
 */
UCLASS()
class PAPER2D_API APaperTileMapActor : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY(Category=TileMapActor, VisibleAnywhere, BlueprintReadOnly, meta=(ExposeFunctionCategories="Sprite,Rendering,Physics,Components|Sprite", AllowPrivateAccess="true"))
	class UPaperTileMapRenderComponent* RenderComponent;
public:

	// AActor interface
#if WITH_EDITOR
	virtual bool GetReferencedContentObjects(TArray<UObject*>& Objects) const override;
#endif
	// End of AActor interface

	/** Returns RenderComponent subobject **/
	FORCEINLINE class UPaperTileMapRenderComponent* GetRenderComponent() const { return RenderComponent; }
};
