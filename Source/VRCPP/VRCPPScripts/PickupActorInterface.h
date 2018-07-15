// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupActorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPickupActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class VRCPP_API IPickupActorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Drop();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Pickup(USceneComponent* AttachTo);


};
