// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CMP302GrappleHookCharacter.h"
#include "CMP302GrappleHookProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "GrappleComponent.h"
#include "LedgeClimberComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ArrowComponent.h"
#include "CableComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ACMP302GrappleHookCharacter
// 
ACMP302GrappleHookCharacter::ACMP302GrappleHookCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	 FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);


	
	// Create cable.
	Cable = CreateDefaultSubobject<UCableComponent>(TEXT("CABLE"));
	Cable->SetupAttachment(FP_MuzzleLocation);
	Cable->SetVisibility(false); // Hide in game for now.
	
	// Mark the base capsule as overlap tag.
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	
	// Create a secondary capsule collider that is just for detecting if we are colliding with something.
	OverlapCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision Capsule"));
	OverlapCapsule->ComponentTags.Add(TEXT("Player"));
	OverlapCapsule->SetupAttachment(GetCapsuleComponent());
	OverlapCapsule->SetCapsuleRadius(GetCapsuleComponent()->GetScaledCapsuleRadius() + 5.f); // Just a tad bigger
	OverlapCapsule->SetCapsuleHalfHeight(GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 5.f); // Just a tad bigger
	OverlapCapsule->SetCollisionProfileName(TEXT("OverlapAll")); // Set it to overlap all.
	OverlapCapsule->SetGenerateOverlapEvents(true); // Let know when colliding.
	
	// Create a grapple component
	GrappleComponent = CreateDefaultSubobject<UGrappleComponent>(TEXT("GrappleComponent"));

	// Create arrows.
	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Forward Arrow"));
	ForwardArrow->SetupAttachment(GetCapsuleComponent());
	ForwardArrow->SetRelativeLocation(FVector(0.f, 0.f, 35.f));
	
	FootArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Foot Arrow"));
	FootArrow->SetupAttachment(GetCapsuleComponent());
	FootArrow->SetRelativeLocation(FVector(0.f, 0.f, -95.f));
	
	// Create ledge climber component and set arrow references.
	LedgeClimberComponent = CreateDefaultSubobject<ULedgeClimberComponent>(TEXT("LedgeClimberComponent"));
	LedgeClimberComponent->ForwardArrow = ForwardArrow;
	LedgeClimberComponent->FootArrow = FootArrow;
}

void ACMP302GrappleHookCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));


	Tags.Add("Player");
	GetCapsuleComponent()->ComponentTags.Add("Player");
	OverlapCapsule->ComponentTags.Add("Player");
	
	// Setup constraint attachment to the capsule.
	
	// Setup references for grapple component.
	GrappleComponent->PlayerCharacter = this;
	GrappleComponent->PlayerCamera = FirstPersonCameraComponent;
	GrappleComponent->OverlapCapsule = OverlapCapsule;
	GrappleComponent->Cable = Cable;
	
	// Setup references for Ledge climber component.
	LedgeClimberComponent->PlayerCharacter = this;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACMP302GrappleHookCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACMP302GrappleHookCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACMP302GrappleHookCharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACMP302GrappleHookCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACMP302GrappleHookCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACMP302GrappleHookCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACMP302GrappleHookCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACMP302GrappleHookCharacter::LookUpAtRate);
}

void ACMP302GrappleHookCharacter::OnFire()
{
	GrappleComponent->OnGrappleFired();
}

void ACMP302GrappleHookCharacter::Jump()
{
	if (LedgeClimberComponent->OnJump()) // If not climbing
	{
		if (!GetCharacterMovement()->IsFalling() && !GetCharacterMovement()->IsFlying() && !HasPressedJump)
		{
			LaunchCharacter(GetActorUpVector() * JumpForce, false, false);
			HasPressedJump = true;
		}
	}
}

void ACMP302GrappleHookCharacter::StopJumping()
{
	if (LedgeClimberComponent->OnStopJumping()) // If not climbing
	{
		HasPressedJump = false;
	}
}

void ACMP302GrappleHookCharacter::Tick(float DeltaSeconds)
{
	if (Health <= 0.f)
	{
		PlayerReset();
	}
}

void ACMP302GrappleHookCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ACMP302GrappleHookCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ACMP302GrappleHookCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACMP302GrappleHookCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

float ACMP302GrappleHookCharacter::GetHealth()
{
	return Health;
}

float ACMP302GrappleHookCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	Health -= DamageAmount; // We really don't care who, what, or where it was hit - just that damage was taken.
	
	return Health;
}

void ACMP302GrappleHookCharacter::PlayerReset()
{
	Health = MaxHealth;
	GrappleComponent->OnPlayerDied();
	LedgeClimberComponent->OnPlayerDied();

	SetActorLocation(RespawnLocation);
}
