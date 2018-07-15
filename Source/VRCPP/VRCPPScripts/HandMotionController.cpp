// Fill out your copyright notice in the Description page of Project Settings.

#include "HandMotionController.h"
#include "Public/MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "SteamVRChaperoneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"

// Sets default values
AHandMotionController::AHandMotionController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>("Scene");
	Scene->AttachTo(RootComponent);
	RootComponent = Scene;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>("MotionController");
	MotionController->AttachTo(RootComponent);

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
}

// Called when the game starts or when spawned
void AHandMotionController::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AHandMotionController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
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
		//TODO: add ruble controller function here
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

		//TODO: Add Rumble Controller
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

}

void AHandMotionController::UpdateArcEndpoint(FVector NewLocation, bool ValidLocationFound)
{

}

void AHandMotionController::GetTeleportDestination(FVector& OutLocation, FRotator& OutRotation)
{

}

void AHandMotionController::SetupRoomScaleOutline()
{

}

void AHandMotionController::UpdateRoomScaleOutline()
{

}