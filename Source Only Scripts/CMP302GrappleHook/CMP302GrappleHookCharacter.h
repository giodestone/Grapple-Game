// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"
#include "GameFramework/Character.h"
#include "CMP302GrappleHookCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class ACMP302GrappleHookCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where grapple should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UArrowComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/*-----Custom Components-----*/

	/*Cable*/
	class UCableComponent* Cable;
	
	/*Secondary Collision detection Capsule that is slight bigger than the original.*/
	UPROPERTY(VisibleDefaultsOnly, Category = Grapple)
	class UCapsuleComponent* OverlapCapsule; 
	
	/* Grapple hook */
	UPROPERTY(VisibleDefaultsOnly, Category = Grapple)
	class UGrappleComponent* GrappleComponent;

	/*Wall climber.*/
	UPROPERTY(VisibleDefaultsOnly, Category = WallClimbing)
	class ULedgeClimberComponent* LedgeClimberComponent;

	/*Arrows*/
	UPROPERTY(VisibleDefaultsOnly, Category = WallClimbing)
	class UArrowComponent* ForwardArrow = nullptr;

	UPROPERTY(VisibleDefaultsOnly, Category = WallClimbing)
	class UArrowComponent* FootArrow = nullptr;
	
public:
	ACMP302GrappleHookCharacter();

protected:
	virtual void BeginPlay() override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/*----- Custom Properties -----*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	FVector RespawnLocation = FVector(-350.f, -100.f, 300.f);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Jump)
	float JumpForce = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 200.f;
	
protected:
	float Health = MaxHealth; // Local health variable
	bool HasPressedJump = false; // Stop the player for jumping forever.
	
protected:
	/** Fires the grapple hook. */
	void OnFire();

	/* Check if we can climb ledge, and if not then jump. */
	void Jump() override;

	/* Stop climbing ledge if already climbing. */
	void StopJumping() override;
	
	void Tick(float DeltaSeconds) override;
	
	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/* Take damage override - Takes health away when collision happens. */
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/* Player Died - Reset their position, health, grapple and climbing status.*/
	UFUNCTION()
	void PlayerReset();
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/* Returns health - No class other than this should be able to modify it. */
	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	float GetHealth();
	
};

