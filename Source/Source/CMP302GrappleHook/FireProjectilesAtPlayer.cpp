// Fill out your copyright notice in the Description page of Project Settings.


#include "FireProjectilesAtPlayer.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "CMP302GrappleHookProjectile.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UFireProjectilesAtPlayer::UFireProjectilesAtPlayer()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UFireProjectilesAtPlayer::BeginPlay()
{
	Super::BeginPlay();
}

bool UFireProjectilesAtPlayer::IsUpright()
{
	return FMath::Abs(FVector::DotProduct(GetOwner()->GetActorUpVector(), FVector::UpVector)) > FMath::Cos(NoLongerUprightDegrees); // Use Cosine rule to check whether the object is up.
}


// Called every frame
void UFireProjectilesAtPlayer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (LookFrom == nullptr || Body == nullptr || TurretMountLeft == nullptr || TurretMountRight == nullptr || FirePoints.Num() < 1)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Red, TEXT("FireProjectilesAtPlayer doesn't have all of its references to components set. Make sure all are set inside of the construction scrip inside of the Turret Blueprint."));
		return;
	}
	
	if (!IsUpright()) // Check if turret isnt upright.
		return;

	if (IsFiringProjectile) // Already firing the projectile, don't fire again!
		return;

	if (GetWorld()->GetFirstPlayerController() == nullptr) // Safety check for if the player got deleted - No error needed as the lack of a player is already noticeable.
		return;
	
	/* Check if the player is within the firing angle. */
	auto playerLocation = GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorLocation();
	auto turretLocation = LookFrom->GetComponentLocation();

	auto turretToPlayer = playerLocation - turretLocation;
	turretToPlayer.Normalize(0.f);

	auto cosOfTurretToPlayer = FVector::DotProduct(turretToPlayer, LookFrom->GetForwardVector()); // Using Cosine rule.
	if (cosOfTurretToPlayer < FMath::Cos(FMath::DegreesToRadians(LookAngle / 2.f))) // If not in cone of vision.
		return;

	/* Check if nothing is obstructing path to player. */
	
	FHitResult hitResult;
	auto hitSomething = GetWorld()->LineTraceSingleByChannel(hitResult, LookFrom->GetComponentLocation(), playerLocation, ECC_Visibility);

	if (!hitSomething) //If not obstructed by something, fire projectile.
	{
		GetWorld()->GetTimerManager().SetTimer(FireProjectileHandle, this, &UFireProjectilesAtPlayer::FireProjectileAtPlayer, FireRateBetweenRounds);
		IsFiringProjectile = true;
	}
}

void UFireProjectilesAtPlayer::FireProjectileAtPlayer()
{
	if (CurrentFirePoint >= FirePoints.Num())
		CurrentFirePoint = 0;

	auto playerLocation = GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorLocation();
	
	auto* firePointArrow = FirePoints[CurrentFirePoint];
	auto firePointLocation = firePointArrow->GetComponentLocation();

	if (ProjectileClass == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(10, 10.f, FColor::Yellow, "Error - projectile set to null at FireProjectilesAtPlayer class (Check Blueprint with item).");
		return;
	}
	
	auto firedProjectile = GetWorld()->SpawnActor<ACMP302GrappleHookProjectile>(ProjectileClass, firePointLocation, firePointArrow->GetComponentRotation());

	auto test = UKismetMathLibrary::FindLookAtRotation(firePointLocation, playerLocation).Vector();
	if (!test.Normalize(0.f))
	{
		return;
	}
	
	Cast<UStaticMeshComponent>(firedProjectile->GetComponentByClass(UStaticMeshComponent::StaticClass()))->AddImpulse(test * FireForce);
	
	CurrentFirePoint++;
	IsFiringProjectile = false;
}

