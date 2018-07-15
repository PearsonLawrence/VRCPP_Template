// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MotionControllerPawn.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EHMDType : uint8
{
	HMDE_OculusRift 	UMETA(DisplayName = "OculusRift"),
	HMDE_Vive 			UMETA(DisplayName = "Vive"),
	HMDE_PSVR			UMETA(DisplayName = "PSVR")
};

UCLASS()
class VRCPP_API AMotionControllerPawn : public APawn
{
	GENERATED_BODY()

public:

	// Sets default values for this pawn's properties
	AMotionControllerPawn();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	///////////////////// Variables //////////////////////

	//-------------- Object Variables --------------//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BasePawn")
	class USceneComponent* VROrigin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BasePawn")
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
	class AHandMotionController* LeftController;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
	class AHandMotionController* RightController;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	TSubclassOf<class AHandMotionController> ControllerBlueprint;

	//-------------- Fade Variables --------------//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FadeVar")
	float FadeOutDuration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FadeVar")
	float FadeInDuration;

	//-------------- Teleport Variables --------------//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TeleportVar")
	float ThumbDeadzone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TeleportVar")
	bool bIsTeleporting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TeleportVar")
	FColor TeleportFadeColor;

	//-------------- ThumbStick Variables --------------//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThumbStick")
	bool bRightStickDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThumbStick")
	bool bLeftStickDown;
	
	//-------------- Misc Variables --------------//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	float DefaultPlayerHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	bool bUseControllerRollToRotate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	bool bShowChaperone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	bool bUsePreMadeInput;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Origin")
	EHMDType HMDType;

	///////////////////// Functions //////////////////////

	//-------------- Base Functions --------------//

	/** Pre-built function meant to be called to setup and detect headset (Oculus, PS4, and Vive).*/
	UFUNCTION(BlueprintCallable, DisplayName = "Detect Headset", Category = "VRCPP Setup")
	void DetectHeadset();

	/** Pre-built function meant to be called to setup, spawn, and attach Motion Controllers.*/
	UFUNCTION(BlueprintCallable, DisplayName = "Setup Motion Controllers", Category = "VRCPP Setup")
	void SetupMotionControllers();

	//-------------- Interaction Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "Do Grab Left", Category = "VRCPP Interaction")
	void DoGrabLeft();

	UFUNCTION(BlueprintCallable, DisplayName = "Do Grab Right", Category = "VRCPP Interaction")
	void DoGrabRight();

	UFUNCTION(BlueprintCallable, DisplayName = "Do Grab", Category = "VRCPP Interaction")
	void DoGrab(float Val, AHandMotionController* HandController);

	//-------------- Movement Input Functions --------------//

	UFUNCTION(BlueprintCallable, DisplayName = "Do Teleport Left", Category = "VRCPP Movement Input")
	void DoTeleportLeft();

	UFUNCTION(BlueprintCallable, DisplayName = "Do Teleport Right", Category = "VRCPP Movement Input")
	void DoTeleportRight();

	UFUNCTION(BlueprintCallable, DisplayName = "Do Teleport", Category = "VRCPP Movement Input")
	void DoTeleport(float Val, AHandMotionController* HandController);

};
