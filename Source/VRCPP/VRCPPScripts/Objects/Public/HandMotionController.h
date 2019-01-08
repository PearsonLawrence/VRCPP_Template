// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "VRCPPScripts/Interfaces/Public/PickupActorInterface.h"
#include "HandMotionController.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EGripState : uint8
{
	Open 		UMETA(DisplayName = "Open"),
	CanGrab 	UMETA(DisplayName = "Can Grab"),
	Grab		UMETA(DisplayName = "Grab")
};

UCLASS()
class VRCPP_API UHandMotionController : public UMotionControllerComponent, public IPickupActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UHandMotionController();

protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	///////////////////// Variables //////////////////////

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UsePreMade")
	bool bPreMadeUpdate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UsePreMade")
	bool bPreMadeBeginPlay;


	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Owner")
	class AMotionControllerPawn* OwnerPawn;
	////-------------- VR Object Variables --------------//

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	//USkeletalMeshComponent* HandMesh;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	//class UArrowComponent* ArcDirection;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	//class USplineComponent* ArcSpline;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	class USphereComponent* GrabSphere;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	//UStaticMeshComponent* ArcEndPoint;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	//UStaticMeshComponent* TeleportCylinder;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	//UStaticMeshComponent* Ring;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	//UStaticMeshComponent* Arrow;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hand")
	//UStaticMeshComponent* RoomScaleMesh;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	//class USteamVRChaperoneComponent* SteamVRChaperone;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	class UHapticFeedbackEffect_Base* HapticType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	TSubclassOf<class UAnimInstance> HandAnimBP;

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
	bool bLastFrameValidDestination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	FVector TeleportDestination;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	FRotator TeleportRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Teleport")
	float TeleportLaunchVelocity;


	///////////////////// Functions //////////////////////

	//-------------- Base Functions --------------//

	/** Pre-built function meant to Handle VR Update through C++ (Meant to be called for default use and out of BP use)*/
	UFUNCTION(BlueprintCallable, DisplayName = "Pre-Built Update", Category = "VRCPP PreMadeFunction")
		void PreBuiltTick();

	/**  Pre-built function meant to Handle VR BeginPlay through C++ (Meant to be called for default use and out of BP use)*/
	UFUNCTION(BlueprintCallable, DisplayName = "Pre-Built BeginPlay", Category = "VRCPP PreMadeFunction")
		void PreBuiltBeginPlay();

	//-------------- Grab Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "SetupMotionControllers", Category = "VRCPP Grabbing")
	AActor* GetActorNearHand();

	UFUNCTION(BlueprintCallable, DisplayName = "ReleaseActor", Category = "VRCPP Grabbing")
	void ReleaseActor();

	UFUNCTION(BlueprintCallable, DisplayName = "GrabActor", Category = "VRCPP Grabbing")
	void GrabActor();

	/**  Pre-built function meant to Update VR GripState through C++*/
	UFUNCTION(BlueprintCallable, DisplayName = "Update Grip State", Category = "VRCPP Grabbing")
	void UpdateGripState();

	//-------------- Teleportation Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "ActivateTeleporter", Category = "VRCPP Teleportation")
	void ActivateTeleporter();

	UFUNCTION(BlueprintCallable, DisplayName = "DisableTeleporter", Category = "VRCPP Teleportation")
	void DisableTeleporter();

	UFUNCTION(BlueprintCallable, DisplayName = "TraceTeleportDestination", Category = "VRCPP Teleportation")
	void TraceTeleportDestination(bool& OutSuccess, TArray<FVector>& OutTracePoints, FVector& OutNavMeshLocation, FVector& OutTraceLocation);

	UFUNCTION(BlueprintCallable, DisplayName = "ClearArc", Category = "VRCPP Teleportation")
	void ClearArc();

	UFUNCTION(BlueprintCallable, DisplayName = "UpdateArcSpline", Category = "VRCPP Teleportation")
	void UpdateArcSpline(bool FoundValidLocation, TArray<FVector> SplinePoints);

	UFUNCTION(BlueprintCallable, DisplayName = "UpdateArcEndpoint", Category = "VRCPP Teleportation")
	void UpdateArcEndpoint(FVector NewLocation, bool ValidLocationFound);

	UFUNCTION(BlueprintCallable, DisplayName = "GetTeleportDestination", Category = "VRCPP Teleportation")
	void GetTeleportDestination(FVector& OutLocation, FRotator& OutRotation);

	/**  Pre-built function meant to Update Teleportation through C++*/
	UFUNCTION(BlueprintCallable, DisplayName = "Update Teleportation Arc", Category = "VRCPP Teleportation")
	void UpdateTeleportationArc();

	//-------------- Room-Scale Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "SetupRoomScaleOutline", Category = "VRCPP RoomScale")
	void SetupRoomScaleOutline();

	UFUNCTION(BlueprintCallable, DisplayName = "UpdateRoomScaleOutline", Category = "VRCPP RoomScale")
	void UpdateRoomScaleOutline();

	//-------------- Misc Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "RumbleController", Category = "VRCPP Rumble")
	void RumbleController(UHapticFeedbackEffect_Base* HFeedback, float Intensity);

};
