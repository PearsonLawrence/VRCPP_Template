// Fill out your copyright notice in the Description page of Project Settings.

#include "VRCPP/VRCPPScripts/Objects/Public/HandMotionController.h"
#include "VRCPP/VRCPPScripts/Objects/Public/MotionControllerPawn.h"
#include "Public/MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "SteamVRChaperoneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "VRCPP/VRCPPScripts/Objects/Public/HandAnimInstance.h"
// Sets default values
UHandMotionController::UHandMotionController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	
/*
	HandMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HandMesh");
	
	HandMesh->SetRelativeRotation(FRotator(0, 0, 90.0f));
	HandMesh->SetRelativeLocation(FVector(-0, 0, 0));
	*//*
	ArcDirection = CreateDefaultSubobject<UArrowComponent>("ArcDirection");
	ArcDirection->SetupAttachment(this);
	ArcDirection->SetRelativeLocation(FVector(14.18f, 0.86f, -4.32f));
	

	ArcSpline = CreateDefaultSubobject<USplineComponent>("ArcSpline");
	ArcSpline->SetupAttachment(this);
	ArcSpline->SetRelativeLocation(FVector(12.53f, -1.76f, 2.55f));

	GrabSphere = CreateDefaultSubobject<USphereComponent>("GrabSphere");
	GrabSphere->SetupAttachment(this);
	GrabSphere->SetRelativeLocation(FVector(14.29f, 0.22f, 1.48f));

	ArcEndPoint = CreateDefaultSubobject<UStaticMeshComponent>("ArcEndPoint");
	ArcEndPoint->SetupAttachment(this);
	ArcEndPoint->SetRelativeScale3D(FVector(.15f));
	ArcEndPoint->SetVisibility(false);

	TeleportCylinder = CreateDefaultSubobject<UStaticMeshComponent>("TeleportCylinder");
	TeleportCylinder->SetupAttachment(this);
	TeleportCylinder->SetRelativeScale3D(FVector(.75f, .75f, 1.0f));

	Ring = CreateDefaultSubobject<UStaticMeshComponent>("Ring");
	Ring->SetupAttachment(TeleportCylinder);
	Ring->SetRelativeScale3D(FVector(.5f, .5f, .15f));

	Arrow = CreateDefaultSubobject<UStaticMeshComponent>("Arrow");
	Arrow->SetupAttachment(TeleportCylinder);

	RoomScaleMesh = CreateDefaultSubobject<UStaticMeshComponent>("RoomScaleMesh");
	RoomScaleMesh->SetupAttachment(Arrow);

	SteamVRChaperone = CreateDefaultSubobject<USteamVRChaperoneComponent>("SteamVRChaperone");
*/

	bPreMadeUpdate = true;
	bPreMadeBeginPlay = true;

}



void UHandMotionController::PreBuiltTick()
{
	UpdateRoomScaleOutline();

	if (HandAnimBP) Cast<UHandAnimInstance>(HandAnimBP)->GripState = GripState;

	//UpdateGripState();
	//UpdateTeleportationArc();

}

void UHandMotionController::PreBuiltBeginPlay()
{
	SetupRoomScaleOutline();
	OwnerPawn->TeleportCylinder->SetVisibility(false, true);
	OwnerPawn->RoomScaleMesh->SetVisibility(false);

	//if (Hand == EControllerHand::Left) HandMesh->SetWorldScale3D(FVector(1.0f, 1.0f, -1.0f));

}

void UHandMotionController::UpdateGripState()
{

	//----------- Handle GripState -----------//

	if (AttachedActor || bWantsToGrip)
	{
		GripState = EGripState::Grab;
	}
	else
	{
		AActor* TempActor = GetActorNearHand();

		GripState = (TempActor) ? EGripState::CanGrab : (bWantsToGrip) ? EGripState::Grab : EGripState::Open;
	};

//	if (GripState == EGripState::Grab) { OwnerPawn->HandMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); }
	//else { OwnerPawn->HandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); }

	/////////////////////////////////////////////////////


}


AActor* UHandMotionController::GetActorNearHand()
{
	AActor* NearestOverlappingActor = nullptr;
	float NearestOverlapDistance = 10000.0f;

	TArray<AActor*> OverlappingActors;
	TArray<FHitResult> Results;
	FCollisionShape sphere;
	sphere.SetSphere(20);
	GetWorld()->SweepMultiByChannel(Results, GetComponentLocation(), GetComponentLocation(), FQuat::Identity, ECollisionChannel::ECC_EngineTraceChannel1, sphere);
	
	//OwnerPawn->GrabSphere->GetOverlappingActors(OverlappingActors);

	for(int32 i = 0; i < Results.Num(); i++)
	{
		bool bDoesUseGrab = UKismetSystemLibrary::DoesImplementInterface(Results[i].GetActor(), UPickupActorInterface::StaticClass());
		
		float DistanceBetweenActors = FVector(Results[i].GetActor()->GetActorLocation() - GetComponentLocation()).Size();

		if (bDoesUseGrab && DistanceBetweenActors < NearestOverlapDistance)
		{
			NearestOverlappingActor = OverlappingActors[i];
			NearestOverlapDistance = DistanceBetweenActors;
		}
	}

	return NearestOverlappingActor;
}

void UHandMotionController::ReleaseActor()
{
	bWantsToGrip = false;

	if (AttachedActor == nullptr) return;

	if (AttachedActor->K2_GetRootComponent()->GetAttachParent() == this && AttachedActor->GetClass()->ImplementsInterface(UPickupActorInterface::StaticClass()))
	{
		RumbleController(HapticType, .3);
		IPickupActorInterface::Execute_Drop(AttachedActor);
	}

	AttachedActor = nullptr;
}

void UHandMotionController::GrabActor()
{
	bWantsToGrip = true;
	AActor* TempActor = GetActorNearHand();

	if (TempActor == nullptr) return;
	
	AttachedActor = TempActor;

	if (AttachedActor->GetClass()->ImplementsInterface(UPickupActorInterface::StaticClass()))
	{
		IPickupActorInterface::Execute_Pickup(AttachedActor, this);

		RumbleController(HapticType, .3);
	}
	else
	{
		ReleaseActor();
	}
}

void UHandMotionController::SetupRoomScaleOutline()
{
	float ChaperoneMeshHeight = 70;

	TArray<FVector> Bounds = OwnerPawn->SteamVRChaperone->GetBounds();
	FVector OutRect;
	FRotator OutRot;
	float OutYSideLength, OutXSideLength;
	UKismetMathLibrary::MinimumAreaRectangle(GetWorld(), Bounds, FVector(0, 0, 1), OutRect, OutRot,OutXSideLength,OutYSideLength);

	bIsRoomScale = (OutXSideLength < 100) && (OutYSideLength < 100);

	if (bIsRoomScale)
	{
		OwnerPawn->RoomScaleMesh->SetWorldScale3D(FVector(OutXSideLength, OutYSideLength, ChaperoneMeshHeight));
		OwnerPawn->RoomScaleMesh->SetRelativeRotation(OutRot);
	}
}

void UHandMotionController::UpdateRoomScaleOutline()
{
	if (!OwnerPawn->RoomScaleMesh->IsVisible()) return;

	FRotator TempRot;
	FVector TempPosition;

	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(TempRot, TempPosition);

	TempRot.Roll = 0;
	TempRot.Pitch = 0;

	TempPosition.Z = 0;

	FVector Result = TempRot.UnrotateVector(TempPosition);


	OwnerPawn->RoomScaleMesh->SetRelativeLocation(TempPosition);
}

void UHandMotionController::RumbleController(UHapticFeedbackEffect_Base* HFeedback, float Intensity)
{
	if (HFeedback == nullptr) return;


	GetWorld()->GetFirstPlayerController()->PlayHapticEffect(HFeedback, Hand, Intensity);
}