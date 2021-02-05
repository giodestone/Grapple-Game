// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CMP302GrappleHookProjectile.generated.h"

UCLASS(config=Game)
class ACMP302GrappleHookProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category="Projectile")
	class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleDefaultsOnly, Category = "Projectile")
	float LifeSpan = 3.f;

	UPROPERTY(VisibleDefaultsOnly, Category = "Damage")
	float DamageToDeal = 20.f;

	UPROPERTY(VisibleDefaultsOnly, Category = "Damage")
	FName PlayerTag = TEXT("Player");

	UPROPERTY(VisibleDefaultsOnly, Category = "Damage")
	FName TurretTag = TEXT("Turret");

public:
	ACMP302GrappleHookProjectile();

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};

