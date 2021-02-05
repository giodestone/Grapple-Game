// Fill out your copyright notice in the Description page of Project Settings.


#include "GrappleComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CMP302GrappleHookCharacter.h"
#include "Components/ArrowComponent.h"
#include "CableComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values for this component's properties
UGrappleComponent::UGrappleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrappleComponent::OnGrappleFired()
{
	if (PreventGrappleCancel || IsWhipping) // Don't do anything if not allowed to prevent the cancel, or the grapple is whipping.
		return;

	if (IsGrappling)
	{
		StopLerping(true);
		return;
	}

	/* Create line trace. */
	FVector start = PlayerCamera->GetComponentLocation();
	FVector forward = PlayerCamera->GetForwardVector();
	FVector end = (forward * GrappleAndWhipRange) + start;

	FHitResult hitResult;
	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(PlayerCharacter->GetParentActor()); // Ignore self if hit
	
	bool hasHitSomething = GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility, collisionParams);
	if (hasHitSomething)
	{
		/*Check that can grapple to this actor.*/
		if (hitResult.GetActor()->ActorHasTag(NoGrappleTag))
			return;
		
		/*Whip Component if its whippable.*/
		if (hitResult.GetActor()->ActorHasTag(WhipableTag))
		{
			IsWhipping = true;
			
			GetWorld()->GetTimerManager().SetTimer(GrappleTimeToWhipHandle, this, &UGrappleComponent::WhipComponent, WhipDelay); //Whip in a moment so we can lerp across there.

			WhippedComponent = hitResult.GetComponent();
			WhipLocation = hitResult.ImpactPoint;

			Cable->SetVisibility(true);
			Cable->SetWorldLocation(WhipLocation);
			
			return;
		}

		/* Otherwise Grapple To Location*/

		GrappleDistance = hitResult.Distance; // Save distance for later.

		if (!PlayerCharacter->GetMovementComponent()->IsFlying() && !PlayerCharacter->GetMovementComponent()->IsFalling()) // Launch up if not already floating.
			PlayerCharacter->LaunchCharacter(FVector::UpVector * GrappleInitialLaunchVelocity, false, true);
		
		/* Begin grappling in some delay. */
		GetWorld()->GetTimerManager().SetTimer(StartGrapplingTimerHandle, this, &UGrappleComponent::GrappleBeginLerping, GrappleBeginGrapplingDelay);

		GrappleToLocation = hitResult.Location;
		PreventGrappleCancel = true;
	}
}

void UGrappleComponent::OnPlayerDied()
{
	StopLerping(false);
}


// Called when the game starts
void UGrappleComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGrappleComponent::GrappleBeginLerping()
{
	GrappleStartLocation = PlayerCharacter->GetActorLocation(); // Start lerping from here.
	PreventGrappleCancel = false; // Allow the player to cancel.
	IsGrappling = true; // Begin Lerping

	GrappleLerpProgress = 0.f;
	GrappleIncrements = GrappleSpeed / (GrappleToLocation - GrappleStartLocation).Size(); // For a constant velocity.

	Cable->SetVisibility(true);
	Cable->SetWorldLocation(GrappleToLocation);
	
	PlayerCharacter->Falling();
}

void UGrappleComponent::StopLerping(bool applyVelocity)
{
	IsGrappling = false;
	Cable->SetVisibility(false);

	PlayerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);

	if (GrappleDistance > GrappleStopLerpingAtDistanceFromTarget && applyVelocity)
	{
		ApplyVelocityWithRope();
		return;
	}		
}

void UGrappleComponent::ApplyVelocityWithRope()
{
	/*Launch player in direction of grapple.*/
	auto movementVector = (GrappleToLocation - GrappleStartLocation).GetSafeNormal(0.f);

	PlayerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	
	PlayerCharacter->LaunchCharacter(movementVector * GrappleSpeed, true, true);
}

bool UGrappleComponent::IsOverlapCapsuleOverlappingWithAnotherObject()
{
	TArray<UPrimitiveComponent*> overlappingComponents;
	OverlapCapsule->GetOverlappingComponents(overlappingComponents);
	
	if (overlappingComponents.Num() > 0)
	{
		for (auto component : overlappingComponents)
		{
			if (!component->ComponentHasTag("Player"))
			{
				return true;
			}
		}
	}
	return false;
}

void UGrappleComponent::WhipComponent()
{	
	WhippedComponent->AddForceAtLocation((PlayerCamera->GetComponentLocation() - WhipLocation).GetSafeNormal(0.f) * WhipForce, WhipLocation);
	
	Cable->SetWorldLocation(Cable->GetComponentLocation()); // Move cable back.
	
	GetWorld()->GetTimerManager().SetTimer(GrappleFinishWhippingHandle, this, &UGrappleComponent::FinishWhippingComponent, WhipDelay); // Make th
}

void UGrappleComponent::FinishWhippingComponent()
{
	IsWhipping = false;
	Cable->SetVisibility(false);
}

// Called every frame
void UGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsGrappling)
	{
		auto newLerpPos = FMath::Lerp(GrappleStartLocation, GrappleToLocation, GrappleLerpProgress); // Where the player will be next.

		/* Check if there will be anything in the way when moving to the location. */
		FHitResult sweepResult;
		auto movementVector = (GrappleStartLocation - GrappleToLocation).GetSafeNormal(0.f);
		PlayerCharacter->SetActorLocation(newLerpPos, true, &sweepResult, ETeleportType::ResetPhysics);

		if (sweepResult.IsValidBlockingHit()) // If would hit something on the way.
		{	
			StopLerping(false);
			return;
		}

		/* Check the capsule if the player has hit anything because the sweep isn't fully accurate. */
		if (IsOverlapCapsuleOverlappingWithAnotherObject())
		{
			StopLerping(false);
			return;
		}

		/* Stop lerping if we're there, or colliding with something .*/
		if ((GrappleToLocation - PlayerCharacter->GetActorLocation()).Size() < GrappleStopLerpingAtDistanceFromTarget ||
			GrappleLerpProgress >= 1.f)
		{
			StopLerping(false);
		}
		
		/* Haven't hit anything, add an increment to the grapple */
		GrappleLerpProgress += GrappleIncrements * DeltaTime;

		/* Need to update every frame because the location is based on relative to stop the rope from spazzing.  */
		Cable->SetWorldLocation(GrappleToLocation);
	}
}

