// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LedgeClimberComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CMP302GRAPPLEHOOK_API ULedgeClimberComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	/*References.*/
	UPROPERTY()
	class UArrowComponent* ForwardArrow = nullptr;

	UPROPERTY()
	class UArrowComponent* FootArrow = nullptr;

	UPROPERTY()
	class ACMP302GrappleHookCharacter* PlayerCharacter;
	
	/*User configurable.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallClimbing)
	float ClimbSpeed = 300.f; // How fast the player climbs in cm/s.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallClimbing)
	float MaxDistanceToWallToStartClimbing = 100.f; // How close the player has to be to a wall before they start climbing up it.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallClimbing)
	float EndClimbingLaunchMultiplier = 50.f; // How strong should the launch be once finished climbing.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallClimbing)
	float EndClimbingUpDirectionModifier = 3.f; // How much stronger should be the up vector than the forward vector when the player is launched forward at the end of their climb.

protected:
	bool IsClimbing = false;

	const ECollisionChannel LedgeTranceChannel = ECC_GameTraceChannel2; // Gotten from DefaultEngine.ini file.
	
public:	
	// Sets default values for this component's properties
	ULedgeClimberComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	// See whether we should be climbing, false if we are, true if we are not (to pass off the jump inside of the player character)
	bool OnJump();

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	// Stop climbing if we are and hand control back to player.
	bool OnStopJumping();

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void OnPlayerDied();

protected:
	// Set the player to start climbing up.
	void StartClimbing();

	// Reset the player movement, and stop climbing.
	void StopClimbing();

	// Climb/Launch character upwards, and check if still on ledge.
	void Climb();

	// Returns true if an arrow is on the ledge.
	bool IsArrowOnLedge(class UArrowComponent* arrow, FHitResult& hitResult);
};
