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

	UpdateGripState();
	UpdateTeleportationArc();

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

void UHandMotionController::UpdateTeleportationArc()
{

	//----------- Handle Teleportation Arc -----------//
	ClearArc();

	if (!bIsTeleporterActive) return;

	bool OutSuccess;
	TArray<FVector> OutPoints;
	FVector OutNavMeshLocation = FVector::ZeroVector;
	FVector OutTraceLocation;

	TraceTeleportDestination(OutSuccess, OutPoints, OutNavMeshLocation, OutTraceLocation);

	OwnerPawn->TeleportCylinder->SetVisibility(bIsValidTeleportDestitination, true);


	FHitResult hit;

	GetWorld()->LineTraceSingleByObjectType(hit, OutNavMeshLocation + FVector(0, 0, -200), OutNavMeshLocation, FCollisionObjectQueryParams::AllStaticObjects);

	FVector Result = UKismetMathLibrary::SelectVector(hit.ImpactPoint, OutNavMeshLocation, hit.bBlockingHit);

	OwnerPawn->TeleportCylinder->SetWorldLocation(Result);



	if (((bIsValidTeleportDestitination && !bLastFrameValidDestination) || (!bIsValidTeleportDestitination && bLastFrameValidDestination)) && HapticType)
		RumbleController(HapticType, .3);

	bLastFrameValidDestination = OutSuccess;
	bIsValidTeleportDestitination = bLastFrameValidDestination;
	UpdateArcSpline(OutSuccess, OutPoints);
	UpdateArcEndpoint(OutTraceLocation, OutSuccess);

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

	for(int i = 0; i < Results.Num(); i++)
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

void UHandMotionController::ActivateTeleporter()
{
	bIsTeleporterActive = true;
	OwnerPawn->TeleportCylinder->SetVisibility(true, true);

	OwnerPawn->RoomScaleMesh->SetVisibility(bIsRoomScale);

	InitialControllerRotation = GetComponentRotation();
	
}


void UHandMotionController::DisableTeleporter()
{
	if (!bIsTeleporterActive) return;

	bIsTeleporterActive = false;

	OwnerPawn->TeleportCylinder->SetVisibility(false, true);
	OwnerPawn->ArcEndPoint->SetVisibility(false);
	OwnerPawn->RoomScaleMesh->SetVisibility(false);

}

//TODO: make sure function works
void UHandMotionController::TraceTeleportDestination(bool& OutSuccess, TArray<FVector>& OutTracePoints, FVector& OutNavMeshLocation, FVector& OutTraceLocation)
{
	TArray<FVector> TempTracePoints;
	FVector ArcStartPos = OwnerPawn->ArcDirection->GetComponentLocation();
	FVector ArcLaunchVelocity = OwnerPawn->ArcDirection->GetForwardVector();

	ArcLaunchVelocity *= TeleportLaunchVelocity;

	// Predict Projectile Path

	FPredictProjectilePathParams PredictParams(0.0f, ArcStartPos, ArcLaunchVelocity * TeleportLaunchVelocity, 4.0f, UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	FPredictProjectilePathResult PredictResult;
	const bool DidPredictPath = UGameplayStatics::PredictProjectilePath(GetWorld(), PredictParams, PredictResult);

	FVector ProjectedTraceHitLocation = PredictResult.HitResult.Location;

	TempTracePoints.Empty();
	for (FPredictProjectilePathPointData Point : PredictResult.PathData)
	TempTracePoints.Push(Point.Location);

	OutTracePoints = TempTracePoints;
	OutTraceLocation = ProjectedTraceHitLocation;

	float ProjectNavExtends = 500;
	UNavigationSystem* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystem>(GetWorld());

	UNavigationSystemV1* NavigationSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());

    FVector ProjectedLocation = NavigationSystem->ProjectPointToNavigation(GetWorld(), ProjectedTraceHitLocation, (ANavigationData *)0, 0, FVector(ProjectNavExtends));

	FNavLocation NavLoc;
	FVector QueryingExtent = FVector(50.0f, 50.0f, 250.0f);
	FNavAgentProperties NavAgentProps;

	bool bProjectedLocationValid = NavigationSystem->ProjectPointToNavigation(GetComponentLocation(), NavLoc, QueryingExtent, (ANavigationData*)0, 0);

	OutSuccess = (PredictResult.HitResult.bBlockingHit && bProjectedLocationValid) ? true : false;
	OutNavMeshLocation = ProjectedLocation;
}

void UHandMotionController::ClearArc()
{
	for (int i = 0; i < SplineMeshes.Num(); i++)
	{
		SplineMeshes[i]->DestroyComponent();
		SplineMeshes.RemoveAt(i);
	}

	OwnerPawn->ArcSpline->ClearSplinePoints();
}

void UHandMotionController::UpdateArcSpline(bool FoundValidLocation, TArray<FVector> SplinePoints)
{
	if (!FoundValidLocation)
	{

		TArray<FVector> nullarray;
		SplinePoints = nullarray;

		SplinePoints.Add(OwnerPawn->ArcDirection->GetComponentLocation());
		SplinePoints.Add(OwnerPawn->ArcDirection->GetComponentLocation() + (OwnerPawn->ArcDirection->GetForwardVector() * 20.0f));
	}

	for (int i = 0; i < SplinePoints.Num(); i++)
	{
		OwnerPawn->ArcSpline->AddSplinePoint(SplinePoints[i], ESplineCoordinateSpace::Local);
	}
	OwnerPawn->ArcSpline->SetSplinePointType(SplinePoints.Num() - 1, ESplinePointType::CurveClamped);

	for (int i = 0; i < OwnerPawn->ArcSpline->GetNumberOfSplinePoints() - 2; i++)
	{
		USplineMeshComponent* tempMesh = CreateDefaultSubobject<USplineMeshComponent>("Mesh");

		SplineMeshes.Add(tempMesh);
		SplineMeshes[SplineMeshes.Num() - 1]->SetStartAndEnd(SplinePoints[i], OwnerPawn->ArcSpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local),
			SplinePoints[i + 1], OwnerPawn->ArcSpline->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local));
	}
}

void UHandMotionController::UpdateArcEndpoint(FVector NewLocation, bool ValidLocationFound)
{
	OwnerPawn->ArcEndPoint->SetVisibility(ValidLocationFound && bIsTeleporterActive);
	OwnerPawn->ArcEndPoint->SetWorldLocation(NewLocation);

	FRotator TempRot;
	FVector TempPosition;

	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(TempRot, TempPosition);

	OwnerPawn->TeleportArrow->SetWorldRotation(FRotator(0, 0, TeleportRotation.Yaw + TempRot.Yaw));
	OwnerPawn->RoomScaleMesh->SetWorldRotation(TeleportRotation);

}

void UHandMotionController::GetTeleportDestination(FVector& OutLocation, FRotator& OutRotation)
{
	FRotator TempRot;
	FVector TempPosition;

	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(TempRot, TempPosition);

	TempPosition.Z = 0.0f;

	FVector Result = TeleportRotation.RotateVector(TempPosition);

	OutLocation = OwnerPawn->TeleportCylinder->GetComponentLocation() - Result;

	OutRotation = TeleportRotation;
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