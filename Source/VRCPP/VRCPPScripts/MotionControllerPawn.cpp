// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionControllerPawn.h"
#include "HandMotionController.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"

// Sets default values
AMotionControllerPawn::AMotionControllerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VROrigin = CreateDefaultSubobject<USceneComponent>(FName("VROrigin"));
	VROrigin->AttachTo(RootComponent);
	RootComponent = VROrigin;

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->AttachTo(VROrigin);
	Camera->bUsePawnControlRotation = false;

	FadeOutDuration = 0.1f;
	FadeInDuration = 0.2f;

	ThumbDeadzone = 0.7f;

	DefaultPlayerHeight = 180.0f;

}

// Called when the game starts or when spawned
void AMotionControllerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	DetectHeadset();
	SetupMotionControllers();

}

// Called every frame
void AMotionControllerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMotionControllerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("GrabLeft", IE_Pressed, this, &AMotionControllerPawn::DoGrabLeft);
	PlayerInputComponent->BindAction("GrabRight", IE_Pressed, this, &AMotionControllerPawn::DoGrabRight);


	PlayerInputComponent->BindAction("TeleportLeft", IE_Pressed, this, &AMotionControllerPawn::DoTeleportLeft);
	PlayerInputComponent->BindAction("TeleportRight", IE_Pressed, this, &AMotionControllerPawn::DoTeleportRight);

}


void AMotionControllerPawn::DetectHeadset()
{
	// - Detect and setup player height for different platforms (Oculus, PS4, and Vive) - //

	// Detect current headset name
	FName HMDName = UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName();

	HMDType = (HMDName == "Oculus Rift") ? EHMDType::HMDE_OculusRift : HMDType;
	HMDType = (HMDName == "Vive") ? EHMDType::HMDE_Vive : HMDType;
	HMDType = (HMDName == "PSVR") ? EHMDType::HMDE_PSVR : HMDType;

	// Set Tracking origin for player height
	switch (HMDType)
	{
	case EHMDType::HMDE_OculusRift:
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
		break;
	case EHMDType::HMDE_Vive:
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
		break;
	case EHMDType::HMDE_PSVR:
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
		VROrigin->AddLocalOffset(FVector(0, 0, DefaultPlayerHeight));
		bUseControllerRotationRoll = true;
		break;
	}
}

void AMotionControllerPawn::SetupMotionControllers()
{
	// - Spawn and attach both motion controllers - //

	// Create new Zero transform
	FTransform NewTransform;
	NewTransform.SetIdentity();

	//Spawn new controllers and assign to pre made Left and right controller values
	FActorSpawnParameters SpawnParams;
	LeftController = GetWorld()->SpawnActor<AHandMotionController>(ControllerBlueprint);
	LeftController->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	LeftController->SetActorScale3D(FVector::OneVector);

	RightController = GetWorld()->SpawnActor<AHandMotionController>(ControllerBlueprint);
	RightController->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	RightController->SetActorScale3D(FVector::OneVector);

}


//TODO: Finish up functions after controller script is made
void AMotionControllerPawn::DoGrabLeft()
{
	if (!bUsePreMadeInput) return;
}

void AMotionControllerPawn::DoGrabRight()
{
	if (!bUsePreMadeInput) return;
}

void AMotionControllerPawn::DoGrab(float Val, AHandMotionController* HandController)
{

}

void AMotionControllerPawn::DoTeleportLeft()
{
	if (!bUsePreMadeInput) return;
}

void AMotionControllerPawn::DoTeleportRight()
{
	if (!bUsePreMadeInput) return;
}

void AMotionControllerPawn::DoTeleport(float Val, AHandMotionController* HandController)
{

}