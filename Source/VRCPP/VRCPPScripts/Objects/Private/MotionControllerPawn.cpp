// Fill out your copyright notice in the Description page of Project Settings.

#include "VRCPP/VRCPPScripts/Objects/Public/MotionControllerPawn.h"
#include "VRCPP/VRCPPScripts/Objects/Public/HandMotionController.h"

#include "Public/MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Public/MotionControllerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Classes/InputCoreTypes.h"
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


	bPreMadeUpdate = true;
	bPreMadeBeginPlay = true;
	bUsePreMadeInput = true;

}

// Called when the game starts or when spawned
void AMotionControllerPawn::BeginPlay()
{
	Super::BeginPlay();

	if (!bPreMadeBeginPlay) return;

	PreBuiltBeginPlay();
}

// Called every frame
void AMotionControllerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bPreMadeUpdate) return;

	PreBuiltTick();
}

// Called to bind functionality to input
void AMotionControllerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("GrabLeft", IE_Pressed, this, &AMotionControllerPawn::PressGrabLeft);
	PlayerInputComponent->BindAction("GrabLeft", IE_Pressed, this, &AMotionControllerPawn::ReleaseGrabLeft);
	PlayerInputComponent->BindAction("GrabRight", IE_Pressed, this, &AMotionControllerPawn::PressGrabRight);
	PlayerInputComponent->BindAction("GrabRight", IE_Pressed, this, &AMotionControllerPawn::ReleaseGrabRight);


	//Could use if there is no need for pre built functions(Same goes with teleport)//
/*
	PlayerInputComponent->BindAction("GrabLeft", IE_Pressed, LeftController, &AHandMotionController::GrabActor);
	PlayerInputComponent->BindAction("GrabLeft", IE_Pressed, LeftController, &AHandMotionController::ReleaseActor);
	PlayerInputComponent->BindAction("GrabRight", IE_Pressed, RightController, &AHandMotionController::GrabActor);
	PlayerInputComponent->BindAction("GrabRight", IE_Pressed, RightController, &AHandMotionController::ReleaseActor);
*/
	////////////////////////////////////////////////////////
	PlayerInputComponent->BindAxis("MovementYAxis", this, &AMotionControllerPawn::DoMovementYAxis);
	PlayerInputComponent->BindAxis("MovementXAxis", this, &AMotionControllerPawn::DoMovementXAxis);


	PlayerInputComponent->BindAction("TeleportLeft", IE_Pressed, this, &AMotionControllerPawn::PressTeleportLeft);
	PlayerInputComponent->BindAction("TeleportLeft", IE_Released, this, &AMotionControllerPawn::ReleaseTeleportLeft);
	PlayerInputComponent->BindAction("TeleportRight", IE_Pressed, this, &AMotionControllerPawn::PressTeleportRight);
	PlayerInputComponent->BindAction("TeleportRight", IE_Released, this, &AMotionControllerPawn::ReleaseTeleportRight);

}

void AMotionControllerPawn::PreBuiltBeginPlay()
{

	DetectHeadset();
	SetupMotionControllers();

}

void AMotionControllerPawn::PreBuiltTick()
{

	if (LeftController->bIsTeleporterActive)
	{
		LeftController->TeleportRotation = GetRotationFromInput(InputComponent->GetAxisValue(FName("MotionControllerThumbLeft_Y")),
																InputComponent->GetAxisValue(FName("MotionControllerThumbLeft_X")),
																LeftController);
	}
	if (RightController->bIsTeleporterActive)
	{
		RightController->TeleportRotation = GetRotationFromInput(InputComponent->GetAxisValue(FName("MotionControllerThumbRight_Y")),
																 InputComponent->GetAxisValue(FName("MotionControllerThumbRight_X")),
																 RightController);
	}
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
	LeftController->Hand = EControllerHand::Left;
	LeftController->MotionController->SetTrackingSource(EControllerHand::Left);
	LeftController->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("LeftController"));
	LeftController->HandMesh->SetWorldScale3D(FVector(1, 1, -1));
	LeftController->OwnerPawn = this;

	RightController = GetWorld()->SpawnActor<AHandMotionController>(ControllerBlueprint);
	RightController->Hand = EControllerHand::Right;
	RightController->MotionController->SetTrackingSource(EControllerHand::Right);
	RightController->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RightController"));
	
	RightController->OwnerPawn = this;


}


void AMotionControllerPawn::PressGrabLeft()
{
	if (!bUsePreMadeInput) return;

	LeftController->GrabActor();
}

void AMotionControllerPawn::PressGrabRight()
{
	if (!bUsePreMadeInput) return;

	RightController->GrabActor();
}

void AMotionControllerPawn::ReleaseGrabLeft()
{
	if (!bUsePreMadeInput) return;

	LeftController->ReleaseActor();
}

void AMotionControllerPawn::ReleaseGrabRight()
{
	if (!bUsePreMadeInput) return;

	RightController->ReleaseActor();
}


// NotUsed
void AMotionControllerPawn::DoGrab(float Val, AHandMotionController* HandController)
{

}

void AMotionControllerPawn::PressTeleportLeft()
{
	if (!bUsePreMadeInput) return;

	LeftController->ActivateTeleporter();
	RightController->DisableTeleporter();
}

void AMotionControllerPawn::PressTeleportRight()
{
	if (!bUsePreMadeInput) return;

	RightController->ActivateTeleporter();
	LeftController->DisableTeleporter();
}

void AMotionControllerPawn::ReleaseTeleportLeft()
{
	if (!bUsePreMadeInput) return;

	if (LeftController->bIsTeleporterActive) DoTeleport(LeftController);
}

void AMotionControllerPawn::ReleaseTeleportRight()
{
	if (!bUsePreMadeInput) return;
	
	if (RightController->bIsTeleporterActive) DoTeleport(RightController);

}

void AMotionControllerPawn::DoMovementYAxis(float Val)
{
	FVector CameraForward = Camera->GetForwardVector();
	FVector Dir = FVector(CameraForward.X, CameraForward.Y, 0) * -Val;
	AddMovementInput(Dir);
}

void AMotionControllerPawn::DoMovementXAxis(float Val)
{

	FVector CameraRight = Camera->GetRightVector();
	FVector Dir = FVector(CameraRight.X, CameraRight.Y, 0) * Val;
	AddMovementInput(Dir);
}


void AMotionControllerPawn::DoTeleport(AHandMotionController* HandController)
{
	if (bIsTeleporting) return;

	if (HandController->bIsValidTeleportDestitination)
	{
		bIsTeleporting = true;


		//--(Use this code if you want camera fade (Have to use a timer [delay] which is not suggested)--//
		/*
		
		 APlayerCameraManager* Manager = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0);

		 Manager->StartCameraFade(0, 1, FadeOutDuration, TeleportFadeColor, false, true);

		//Error is expected but will still build
		GetWorldTimerManager().SetTimer(UnusedHandle, HandController, &AHandMotionController::DisableTeleporter, FadeOutDuration, true);

		FVector TPLocation;
		FRotator TPRotation;
		HandController->GetTeleportDestination(TPLocation, TPRotation);

		SetActorLocation(TPLocation);
		SetActorRotation(TPRotation);

		Manager->StartCameraFade(1, 0, FadeOutDuration, TeleportFadeColor, false, true);

		*/
		//--------------------------------------------------------------------------------//

		//--(Use this code if you dont want fade or timer for more efficient code)--//

		HandController->DisableTeleporter();

		FVector TPLocation;
		FRotator TPRotation;
		HandController->GetTeleportDestination(TPLocation, TPRotation);

		SetActorLocation(TPLocation);
		SetActorRotation(TPRotation);

		//--------------------------------------------------------------------------------//

		bIsTeleporting = false;
	}
	else
	{
		HandController->DisableTeleporter();
	}
}

FRotator AMotionControllerPawn::GetRotationFromInput(float UpAxis, float RightAxis, AHandMotionController* HandController)
{
	FTransform WristOrientation;
	WristOrientation.SetRotation(HandController->InitialControllerRotation.Quaternion());

	FTransform Result = UKismetMathLibrary::ConvertTransformToRelative(WristOrientation, HandController->MotionController->GetComponentTransform());

	FRotator NewHandRot = Result.Rotator();
	NewHandRot.Roll *= 3;

	Result.SetRotation(NewHandRot.Quaternion());
	NewHandRot = Result.Rotator();

	float newYaw = NewHandRot.Roll + GetActorRotation().Yaw;

	NewHandRot = FRotator(0, 0, newYaw);

	FRotator RotatedPawnRot;
	RotatedPawnRot.Yaw += GetActorRotation().Yaw;
	
	FVector DirectionVector = RotatedPawnRot.RotateVector(FVector(UpAxis, RightAxis, 0).GetSafeNormal());
	RotatedPawnRot = DirectionVector.Rotation();
	
	bool bIsThumbNearCenter = ((FMath::Abs(UpAxis) + FMath::Abs(RightAxis)) >= ThumbDeadzone);

	RotatedPawnRot = (bIsThumbNearCenter) ? RotatedPawnRot : GetActorRotation();

	return (bUseControllerRollToRotate) ? NewHandRot : RotatedPawnRot;
}