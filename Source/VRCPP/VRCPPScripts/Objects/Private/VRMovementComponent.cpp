
#include "VRCPP/VRCPPScripts/Objects/Public/VRMovementComponent.h" 
#include "Components/ArrowComponent.h"
#include "SteamVRChaperoneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Public/DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"

// Sets default values for this component's properties
UVRMovementComponent::UVRMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}



void UVRMovementComponent::ActivateTeleporterForActor(AActor* Owner, UStaticMeshComponent* TeleportCylinder, UStaticMeshComponent* RoomScaleMesh)
{

	if (!Owner) return;

	bIsTeleporterActive = true;

	if (TeleportCylinder)
	{
		TeleportCylinder->SetVisibility(true, true);
	}
	if (RoomScaleMesh)
	{
		RoomScaleMesh->SetVisibility(true, true);
	}

}


void UVRMovementComponent::ActivateTeleporterForHand(UChildActorComponent* Owner, UStaticMeshComponent* TeleportCylinder, UStaticMeshComponent* RoomScaleMesh)
{

	if (!Owner) return;

	bIsTeleporterActive = true;

	if (TeleportCylinder)
	{
		TeleportCylinder->SetVisibility(true, true);
	}
	if (RoomScaleMesh)
	{
		RoomScaleMesh->SetVisibility(true, true);
	}

}

void UVRMovementComponent::DisableTeleporter(AActor* ActorToTeleport, FVector NewTeleportLocation, FRotator NewTeleportRotation, float TeleportationHeightOffset, bool bAutoActivateTeleport, UStaticMeshComponent* TeleportCylinder, UStaticMeshComponent* RoomScaleMesh)
{
	if (!ActorToTeleport)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner is NULL"));
		return;
	}

	if (!bIsTeleporterActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Teleporter is not active"));
		return;
	}

	bIsTeleporterActive = false;

	if (TeleportCylinder)
	{
		TeleportCylinder->SetVisibility(false, false);
	}
	if (RoomScaleMesh)
	{
		RoomScaleMesh->SetVisibility(false, false);
	}

	if (bAutoActivateTeleport && bIsValidTeleportDestitination)
		ExecuteTeleport(ActorToTeleport, NewTeleportLocation, NewTeleportRotation, TeleportationHeightOffset);
	
}


void UVRMovementComponent::TraceTeleportDestination(FVector Start, FVector Direction, ECollisionChannel TraceChannel, bool& OutSuccess, FVector& OutNavMeshLocation, FVector& OutTraceLocation, float TeleportTraceDistance, bool bDrawLineTrace, float LineThickness, FColor TraceColor, bool bChangeLineTraceColorOnHit, FColor TraceHitColor)
{


	FHitResult Result;
	GetWorld()->LineTraceSingleByChannel(Result, Start, Start + (Direction * TeleportTraceDistance), TraceChannel);
	
	if (bDrawLineTrace)
	{
		FColor Color = (bChangeLineTraceColorOnHit) ? ((Result.bBlockingHit) ? TraceHitColor : TraceColor) : TraceColor;
		DrawDebugLine(GetWorld(), Start, Start + (Direction * TeleportTraceDistance), Color, false, 0, (uint8)'\000', LineThickness);
	}

	OutSuccess = Result.bBlockingHit;

	OutTraceLocation = (Result.bBlockingHit) ? Result.ImpactPoint : Start;

	//Teleport use focused on only landing on nav mesh
	/*if (Result.bBlockingHit)
	{
		float ProjectNavExtends = 100;
		UNavigationSystem* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystem>(GetWorld());

		UNavigationSystemV1* NavigationSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());

		FVector ProjectedLocation = NavigationSystem->ProjectPointToNavigation(GetWorld(), Result.ImpactPoint, (ANavigationData *)0, 0, FVector(ProjectNavExtends));


		OutTraceLocation = Result.ImpactPoint;
		OutNavMeshLocation = ProjectedLocation;
		
	}*/
}


void UVRMovementComponent::ExecuteTeleport(AActor* ActorToTeleport, FVector NewLocation, FRotator NewRotation, float ZOffset)
{
	if (!ActorToTeleport)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActorToTeleport is NULL"));
		return;
	}

	FVector LocationPlusOffset = FVector(NewLocation.X, NewLocation.Y, NewLocation.Z + ZOffset);

	ActorToTeleport->SetActorLocation(LocationPlusOffset);
	ActorToTeleport->SetActorRotation(NewRotation);

}
