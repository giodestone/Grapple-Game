// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/SoftObjectPath.h"
#include "FireProjectilesAtPlayer.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CMP302GRAPPLEHOOK_API UFireProjectilesAtPlayer : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class ACMP302GrappleHookProjectile> ProjectileClass;
	
	UPROPERTY(BlueprintReadWrite, Category = "Turret")
	class UArrowComponent* LookFrom;

	UPROPERTY(BlueprintReadWrite, Category = "Turret")
	class UStaticMeshComponent* Body;

	UPROPERTY(BlueprintReadWrite, Category = "Turret")
	class UStaticMeshComponent* TurretMountLeft;

	UPROPERTY(BlueprintReadWrite, Category = "Turret")
	class UStaticMeshComponent* TurretMountRight;

	UPROPERTY(BlueprintReadWrite, Category = "Turret")
	TArray<class UArrowComponent*> FirePoints;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret")
	float FireRateBetweenRounds = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret")
	float LookAngle = 55.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret")
	float FireForce = 100000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	float NoLongerUprightDegrees = 35.f; // When the turret will switch off when no longer upright.

protected:
	FTimerHandle FireProjectileHandle;

	bool IsFiringProjectile = false;
	int CurrentFirePoint = 0; // For alternating between fire points.
	
public:	
	// Sets default values for this component's properties
	UFireProjectilesAtPlayer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// If the turret is upright.
	UFUNCTION(BlueprintCallable, Category = "Turret")
	bool IsUpright();
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Turret")
	void FireProjectileAtPlayer();
};
