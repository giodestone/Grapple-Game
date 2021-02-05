// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CMP302GrappleHookProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"
#include "CMP302GrappleHookCharacter.h"
#include "Engine/Engine.h"

ACMP302GrappleHookProjectile::ACMP302GrappleHookProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ACMP302GrappleHookProjectile::OnHit);	// Set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = LifeSpan;
}

void ACMP302GrappleHookProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Deal damage to player and destroy.
	if ((OtherActor != NULL) && (OtherActor != this) && OtherActor->ActorHasTag(PlayerTag) && !OtherActor->ActorHasTag(TurretTag))
	{
		FDamageEvent damageEvent;
		
		OtherActor->TakeDamage(DamageToDeal, damageEvent, nullptr, this);
		
		Destroy();
	}
	else if ((OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics()) // Reflect off and destroy.
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		Destroy();
	}
}