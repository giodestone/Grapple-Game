// Fill out your copyright notice in the Description page of Project Settings.


#include "CableEndPoint.h"

// Sets default values
ACableEndPoint::ACableEndPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Attachment")); //Just create an attachment Point
}

// Called when the game starts or when spawned
void ACableEndPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACableEndPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

