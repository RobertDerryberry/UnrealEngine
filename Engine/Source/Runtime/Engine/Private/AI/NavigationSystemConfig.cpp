// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "AI/NavigationSystemConfig.h"
#include "AI/NavigationSystemBase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"


UNavigationSystemConfig::UNavigationSystemConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (GEngine && *GEngine->NavigationSystemClass != nullptr)
	{
		NavigationSystemClass = GEngine->NavigationSystemClassName;
	}
}

UNavigationSystemBase* UNavigationSystemConfig::CreateAndConfigureNavigationSystem(UWorld& World) const
{
	UNavigationSystemBase* NavSys = nullptr;

	NavigationSystemClass.TryLoad();
	TSubclassOf<UNavigationSystemBase> NavSysClass = NavigationSystemClass.ResolveClass();
	
	if (NavSysClass && (bCreateOnClient == true || World.GetNetMode() != NM_Client))
	{
		NavSys = NewObject<UNavigationSystemBase>(&World, NavSysClass);
		if (NavSys)
		{
			NavSys->Configure(*this);
		}
	}

	return NavSys;	
}

#if WITH_EDITOR
void UNavigationSystemConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	static const FName NAME_NavigationSystemClass = GET_MEMBER_NAME_CHECKED(UNavigationSystemConfig, NavigationSystemClass);

	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		FName PropName = PropertyChangedEvent.Property->GetFName();
		if (PropName == NAME_NavigationSystemClass)
		{
			if (NavigationSystemClass.IsValid() == false)
			{
				NavigationSystemClass = *GEngine->NavigationSystemClass;
			}
			else
			{
				NavigationSystemClass.TryLoad();
				TSubclassOf<UNavigationSystemBase> NavSysClass = NavigationSystemClass.ResolveClass();
				if (NavSysClass)
				{
					//bCreateOnClient = NavSysClass->GetDefaultObject<UNavigationSystem>()->ShouldAllowClientSideNavigation();
				}
			}
		}
	}
}
#endif // WITH_EDITOR

//----------------------------------------------------------------------//
// 
//----------------------------------------------------------------------//
UNullNavSysConfig::UNullNavSysConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NavigationSystemClass.Reset();
}
