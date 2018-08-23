// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "VRCPPScripts/HandMotionController.h"
#include "HandAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class VRCPP_API UHandAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	EGripState GripState;
	
};
