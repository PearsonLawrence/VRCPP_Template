// Fill out your copyright notice in the Description page of Project Settings.

#include "HandMotionController.h"
#include "MotionControllerPawn.h"
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
#include "AI/Navigation/NavigationSystem.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "HandAnimInstance.h"
// Sets default values
AHandMotionController::AHandMotionController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>("Scene");
	Scene->AttachTo(RootComponent);
	RootComponent = Scene;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>("MotionController");
	MotionController->AttachTo(Scene);

	HandMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HandMesh");
	HandMesh->AttachTo(MotionController);

	ArcDirection = CreateDefaultSubobject<UArrowComponent>("ArcDirection");
	ArcDirection->AttachTo(HandMesh);

	ArcSpline = CreateDefaultSubobject<USplineComponent>("ArcSpline");
	ArcSpline->AttachTo(HandMesh);

	GrabSphere = CreateDefaultSubobject<USphereComponent>("GrabSphere");
	GrabSphere->AttachTo(HandMesh);

	ArcEndPoint = CreateDefaultSubobject<UStaticMeshComponent>("ArcEndPoint");
	ArcEndPoint->AttachTo(MotionController);

	TeleportCylinder = CreateDefaultSubobject<UStaticMeshComponent>("TeleportCylinder");
	TeleportCylinder->AttachTo(MotionController);

	Ring = CreateDefaultSubobject<UStaticMeshComponent>("Ring");
	Ring->AttachTo(TeleportCylinder);

	Arrow = CreateDefaultSubobject<UStaticMeshComponent>("Arrow");
	Arrow->AttachTo(MotionController);

	RoomScaleMesh = CreateDefaultSubobject<UStaticMeshComponent>("RoomScaleMesh");
	RoomScaleMesh->AttachTo(Arrow);

	SteamVRChaperone = CreateDefaultSubobject<USteamVRChaperoneComponent>("SteamVRChaperone");


	bPreMadeUpdate = true;
	bPreMadeBeginPlay = true;

}

// Called when the game starts or when spawned
void AHandMotionController::BeginPlay()
{
	Super::BeginPlay();

	if (!bPreMadeBeginPlay) return;

	PreBuiltBeginPlay();
}

// Called every frame
void AHandMotionController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bPreMadeUpdate) return;

	PreBuiltTick();
}

void AHandMotionController::PreBuiltTick()
{
	UpdateRoomScaleOutline();

	if (HandAnimBP) Cast<UHandAnimInstance>(HandAnimBP)->GripState = GripState;

	UpdateGripState();
	UpdateTeleportationArc();

}

void AHandMotionController::PreBuiltBeginPlay()
{
	SetupRoomScaleOutline();
	TeleportCylinder->SetVisibility(false, true);
	RoomScaleMesh->SetVisibility(false);

	if (Hand == EControllerHand::Left) HandMesh->SetWorldScale3D(FVector(1.0f, 1.0f, -1.0f));

}

void AHandMotionController::UpdateGripState()
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

	if (GripState == EGripState::Grab) { HandMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); }
	else { HandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); }

	/////////////////////////////////////////////////////


}

void AHandMotionController::UpdateTeleportationArc()
{

	//----------- Handle Teleportation Arc -----------//
	ClearArc();

	if (!bIsTeleporterActive) return;

	bool OutSuccess;
	TArray<FVector> OutPoints;
	FVector OutNavMeshLocation = FVector::ZeroVector;
	FVector OutTraceLocation;

	TraceTeleportDestination(OutSuccess, OutPoints, OutNavMeshLocation, OutTraceLocation);

	TeleportCylinder->SetVisibility(bIsValidTeleportDestitination, true);


	FHitResult hit;

	GetWorld()->LineTraceSingleByObjectType(hit, OutNavMeshLocation + FVector(0, 0, -200), OutNavMeshLocation, FCollisionObjectQueryParams::AllStaticObjects);

	FVector Result = UKismetMathLibrary::SelectVector(hit.ImpactPoint, OutNavMeshLocation, hit.bBlockingHit);

	TeleportCylinder->SetWorldLocation(Result);



	if (((bIsValidTeleportDestitination && !bLastFrameValidDestination) || (!bIsValidTeleportDestitination && bLastFrameValidDestination)) && HapticType)
		RumbleController(HapticType, .3);

	bLastFrameValidDestination = OutSuccess;

	UpdateArcSpline(OutSuccess, OutPoints);
	UpdateArcEndpoint(OutTraceLocation, OutSuccess);

	/////////////////////////////////////////////////////

}


AActor* AHandMotionController::GetActorNearHand()
{
	AActor* NearestOverlappingActor = nullptr;
	float NearestOverlapDistance = 10000.0f;

	TArray<AActor*> OverlappingActors;
	GrabSphere->GetOverlappingActors(OverlappingActors);

	for(int i = 0; i < OverlappingActors.Num(); i++)
	{
		bool bDoesUseGrab = UKismetSystemLibrary::DoesImplementInterface(OverlappingActors[i], UPickupActorInterface::StaticClass());
		
		float DistanceBetweenActors = FVector(OverlappingActors[i]->GetActorLocation() - GrabSphere->GetComponentLocation()).Size();

		if (bDoesUseGrab && DistanceBetweenActors < NearestOverlapDistance)
		{
			NearestOverlappingActor = OverlappingActors[i];
			NearestOverlapDistance = DistanceBetweenActors;
		}
	}

	return NearestOverlappingActor;
}

void AHandMotionController::ReleaseActor()
{
	bWantsToGrip = false;

	if (AttachedActor == nullptr) return;

	if (AttachedActor->K2_GetRootComponent()->GetAttachParent() == MotionController && AttachedActor->GetClass()->ImplementsInterface(UPickupActorInterface::StaticClass()))
	{
		RumbleController(HapticType, .3);
		IPickupActorInterface::Execute_Drop(AttachedActor);
	}

	AttachedActor = nullptr;
}

void AHandMotionController::GrabActor()
{
	bWantsToGrip = true;
	AActor* TempActor = GetActorNearHand();

	if (TempActor == nullptr) return;
	
	AttachedActor = TempActor;

	if (AttachedActor->GetClass()->ImplementsInterface(UPickupActorInterface::StaticClass()))
	{
		IPickupActorInterface::Execute_Pickup(AttachedActor, MotionController);

		RumbleController(HapticType, .3);
	}
	else
	{
		ReleaseActor();
	}
}

void AHandMotionController::ActivateTeleporter()
{
	bIsTeleporterActive = true;
	TeleportCylinder->SetVisibility(true, true);

	RoomScaleMesh->SetVisibility(bIsRoomScale);

	InitialControllerRotation = MotionController->GetComponentRotation();
	
}


void AHandMotionController::DisableTeleporter()
{
	if (!bIsTeleporterActive) return;

	bIsTeleporterActive = false;

	TeleportCylinder->SetVisibility(false, true);
	ArcEndPoint->SetVisibility(false);
	RoomScaleMesh->SetVisibility(false);

}

//TODO: make sure function works
void AHandMotionController::TraceTeleportDestination(bool& OutSuccess, TArray<FVector>& OutTracePoints, FVector& OutNavMeshLocation, FVector& OutTraceLocation)
{
	FHitResult hit;

	
	FVector Temp;
	TArray<AActor*> NullArray;

	TArray<FVector> TempTracePoints;

	/*UGameplayStatics::PredictProjectilePath(GetWorld(), hit, TempTracePoints, Temp, ArcDirection->GetComponentLocation(),
											ArcDirection->GetForwardVector() * TeleportLaunchVelocity, true, 0.0f, 
											UEngineTypes::ConvertToObjectType(ECC_WorldStatic), false, 
											NullArray,EDrawDebugTrace::None,0.0f,30.0f,2.0f,0.0f);
*/
	OutTracePoints = TempTracePoints;
	OutTraceLocation = hit.Location;

	float ProjectNavExtends = 500;

    FVector ProjectedLocation = GetWorld()->GetNavigationSystem()->ProjectPointToNavigation(GetWorld(), hit.Location, (ANavigationData *)0, 0, FVector(ProjectNavExtends));

	FNavLocation NavLoc;
	FVector QueryingExtent = FVector(50.0f, 50.0f, 250.0f);
	FNavAgentProperties NavAgentProps;

	bool bProjectedLocationValid = GetWorld()->GetNavigationSystem()->ProjectPointToNavigation(GetActorLocation(), NavLoc, QueryingExtent, GetWorld()->GetNavigationSystem()->MainNavData);

	OutSuccess = (hit.bBlockingHit && bProjectedLocationValid) ? true : false;
	OutNavMeshLocation = ProjectedLocation;
}

void AHandMotionController::ClearArc()
{
	for (int i = 0; i < SplineMeshes.Num(); i++)
	{
		SplineMeshes[i]->DestroyComponent();
		SplineMeshes.RemoveAt(i);
	}

	ArcSpline->ClearSplinePoints();
}

void AHandMotionController::UpdateArcSpline(bool FoundValidLocation, TArray<FVector> SplinePoints)
{
	if (!FoundValidLocation)
	{

		TArray<FVector> nullarray;
		SplinePoints = nullarray;

		SplinePoints.Add(ArcDirection->GetComponentLocation());
		SplinePoints.Add(ArcDirection->GetComponentLocation() + (ArcDirection->GetForwardVector() * 20.0f));
	}

	for (int i = 0; i < SplinePoints.Num(); i++)
	{
		ArcSpline->AddSplinePoint(SplinePoints[i], ESplineCoordinateSpace::Local);
	}
	ArcSpline->SetSplinePointType(SplinePoints.Num() - 1, ESplinePointType::CurveClamped);

	for (int i = 0; i < ArcSpline->GetNumberOfSplinePoints() - 2; i++)
	{
		USplineMeshComponent* tempMesh = CreateDefaultSubobject<USplineMeshComponent>("Mesh");

		SplineMeshes.Add(tempMesh);
		SplineMeshes[SplineMeshes.Num() - 1]->SetStartAndEnd(SplinePoints[i], ArcSpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local),
			SplinePoints[i + 1], ArcSpline->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local));
	}
}

void AHandMotionController::UpdateArcEndpoint(FVector NewLocation, bool ValidLocationFound)
{
	ArcEndPoint->SetVisibility(ValidLocationFound && bIsTeleporterActive);
	ArcEndPoint->SetWorldLocation(NewLocation);

	FRotator TempRot;
	FVector TempPosition;

	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(TempRot, TempPosition);

	Arrow->SetWorldRotation(FRotator(0, 0, TeleportRotation.Yaw + TempRot.Yaw));
	RoomScaleMesh->SetWorldRotation(TeleportRotation);

}

void AHandMotionController::GetTeleportDestination(FVector& OutLocation, FRotator& OutRotation)
{
	FRotator TempRot;
	FVector TempPosition;

	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(TempRot, TempPosition);

	TempPosition.Z = 0.0f;

	FVector Result = TeleportRotation.RotateVector(TempPosition);

	OutLocation = TeleportCylinder->GetComponentLocation() - Result;

	OutRotation = TeleportRotation;
}

void AHandMotionController::SetupRoomScaleOutline()
{
	float ChaperoneMeshHeight = 70;

	TArray<FVector> Bounds = SteamVRChaperone->GetBounds();
	FVector OutRect;
	FRotator OutRot;
	float OutYSideLength, OutXSideLength;
	UKismetMathLibrary::MinimumAreaRectangle(GetWorld(), Bounds, FVector(0, 0, 1), OutRect, OutRot,OutXSideLength,OutYSideLength);

	bIsRoomScale = (OutXSideLength < 100) && (OutYSideLength < 100);

	if (bIsRoomScale)
	{
		RoomScaleMesh->SetWorldScale3D(FVector(OutXSideLength, OutYSideLength, ChaperoneMeshHeight));
		RoomScaleMesh->SetRelativeRotation(OutRot);
	}
}

void AHandMotionController::UpdateRoomScaleOutline()
{
	if (!RoomScaleMesh->IsVisible()) return;

	FRotator TempRot;
	FVector TempPosition;

	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(TempRot, TempPosition);

	TempRot.Roll = 0;
	TempRot.Pitch = 0;

	TempPosition.Z = 0;

	FVector Result = TempRot.UnrotateVector(TempPosition);


	RoomScaleMesh->SetRelativeLocation(TempPosition);
}

void AHandMotionController::RumbleController(UHapticFeedbackEffect_Base* HFeedback, float Intensity)
{
	if (HFeedback == nullptr) return;


	GetWorld()->GetFirstPlayerController()->PlayHapticEffect(HFeedback, Hand, Intensity);
}