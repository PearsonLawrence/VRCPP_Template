// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupActorInterface.h"
#include "HandMotionController.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EGripState : uint8
{
	Open 		UMETA(DisplayName = "Open"),
	CanGrab 	UMETA(DisplayName = "Can Grab"),
	Grab		UMETA(DisplayName = "Grab")
};

UCLASS()
class VRCPP_API AHandMotionController : public AActor, public IPickupActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHandMotionController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	///////////////////// Variables //////////////////////


	//-------------- Object Variables --------------//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	USceneComponent* Scene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	class UMotionControllerComponent* MotionController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	USkeletalMeshComponent* HandMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	class UArrowComponent* ArcDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	class USplineComponent* ArcSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	class USphereComponent* GrabSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	UStaticMeshComponent* ArcEndPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	UStaticMeshComponent* TeleportCylinder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	UStaticMeshComponent* Ring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	UStaticMeshComponent* Arrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	UStaticMeshComponent* RoomScaleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	class USteamVRChaperoneComponent* SteamVRChaperone;


	//-------------- Hand Variables --------------//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	EControllerHand Hand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	EGripState GripState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	FRotator InitialControllerRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	bool bWantsToGrip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Owner")
	AActor* AttachedActor;

	//-------------- Teleport Variables --------------//

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	TArray<class USplineMeshComponent*> SplineMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	bool bIsRoomScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	bool bIsTeleporterActive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	bool bIsValidTeleportDestitination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	FVector TeleportDestination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	FRotator TeleportRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	float TeleportLaunchVelocity;


	///////////////////// Functions //////////////////////

	//-------------- Base Functions --------------//


	//-------------- Grab Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "SetupMotionControllers", Category = "Grabbing")
	AActor* GetActorNearHand();

	UFUNCTION(BlueprintCallable, DisplayName = "ReleaseActor", Category = "Grabbing")
	void ReleaseActor();

	UFUNCTION(BlueprintCallable, DisplayName = "GrabActor", Category = "Grabbing")
	void GrabActor();

	//-------------- Teleportation Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "ActivateTeleporter", Category = "Teleportation")
	void ActivateTeleporter();

	UFUNCTION(BlueprintCallable, DisplayName = "DisableTeleporter", Category = "Teleportation")
	void DisableTeleporter();

	UFUNCTION(BlueprintCallable, DisplayName = "TraceTeleportDestination", Category = "Teleportation")
	void TraceTeleportDestination(bool& OutSuccess, TArray<FVector>& OutTracePoints, FVector& OutNavMeshLocation, FVector& OutTraceLocation);

	UFUNCTION(BlueprintCallable, DisplayName = "ClearArc", Category = "Teleportation")
	void ClearArc();

	UFUNCTION(BlueprintCallable, DisplayName = "UpdateArcSpline", Category = "Teleportation")
	void UpdateArcSpline(bool FoundValidLocation, TArray<FVector> SplinePoints);

	UFUNCTION(BlueprintCallable, DisplayName = "UpdateArcEndpoint", Category = "Teleportation")
	void UpdateArcEndpoint(FVector NewLocation, bool ValidLocationFound);

	UFUNCTION(BlueprintCallable, DisplayName = "GetTeleportDestination", Category = "Teleportation")
	void GetTeleportDestination(FVector& OutLocation, FRotator& OutRotation);

	//-------------- Room-Scale Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "SetupRoomScaleOutline", Category = "Teleportation")
	void SetupRoomScaleOutline();

	UFUNCTION(BlueprintCallable, DisplayName = "UpdateRoomScaleOutline", Category = "Teleportation")
	void UpdateRoomScaleOutline();

};
