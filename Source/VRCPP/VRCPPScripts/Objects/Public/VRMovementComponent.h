// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VRMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRCPP_API UVRMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

		UVRMovementComponent();
protected:

public:


	//-------------- Teleport Variables --------------//

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
		bool bIsTeleporterActive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
		bool bIsValidTeleportDestitination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
		bool bLastFrameValidDestination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
		FVector TeleportDestination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
		FRotator TeleportRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
		float TeleportLaunchVelocity;

	///////////////////// Functions //////////////////////

	//-------------- Base Functions --------------//



	//-------------- SetDefaults Functions --------------//


	UFUNCTION(BlueprintCallable, DisplayName = "SetMovementDefaults", Category = "VRCPP Smooth Movement")
		void SetMovementDefaults(float NewMaxStepHeight, float NewWalkableFloorAngle, float NewGroundFriction, float NewMaxWalkSpeed, float NewBreakingSpeed);


	//-------------- Teleportation Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "ActivateTeleporter", Category = "VRCPP Teleportation")
		void ActivateTeleporterForActor(AActor* Owner, UStaticMeshComponent* RoomScaleMesh = nullptr);

	UFUNCTION(BlueprintCallable, DisplayName = "DisableTeleporter", Category = "VRCPP Teleportation")
		void DisableTeleporter(AActor* ActorToTeleport, FVector NewTeleportLocation, FRotator NewTeleportRotation, 
			                   float TeleportationHeightOffset, bool bAutoActivateTeleport = true, UStaticMeshComponent* TeleportCylinder = nullptr, UStaticMeshComponent* RoomScaleMesh = nullptr);

	UFUNCTION(BlueprintCallable, DisplayName = "UpdateTeleportationMeshes", Category = "VRCPP Teleportation")
		void UpdateTeleportationMeshes(FVector NewMeshLocation, FRotator NewMeshRotation, bool bVisibility, UStaticMeshComponent* TeleportCylinder = nullptr, UStaticMeshComponent* RoomScaleMesh = nullptr);

	UFUNCTION(BlueprintCallable, DisplayName = "TraceTeleportDestination", Category = "VRCPP Teleportation")
		void TraceTeleportDestination(FVector Start, FVector Direction, ECollisionChannel TraceChannel,  bool& OutSuccess, 
									  FVector& OutNavMeshLocation, FVector& OutTraceLocation, 
									  float TeleportTraceDistance = 500.0f, bool bDrawLineTrace = false, float LineThickness = 1, 
									  FColor TraceColor = FColor::Red, bool bChangeLineTraceColorOnHit = false, FColor TraceHitColor = FColor::Green);

	UFUNCTION(BlueprintCallable, DisplayName = "ExecuteTeleport", Category = "VRCPP Teleportation")
		void ExecuteTeleport(AActor* ActorToTeleport, FVector NewLocation, FRotator NewRotation, float ZOffset = 0);


	//-------------- Smooth Movement Functions --------------//


	UFUNCTION(BlueprintCallable, DisplayName = "CharacterBaseMovement", Category = "VRCPP Smooth Movement")
		void CharacterBaseMovement(ACharacter* Character, FVector ForwardOrientation, FVector RightOrientation, float XAxisValue, float YAxisValue, float Speed, bool bForce);

};
