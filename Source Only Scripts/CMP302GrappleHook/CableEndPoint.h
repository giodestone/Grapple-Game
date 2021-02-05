// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CableEndPoint.generated.h"

// Just an empty actor which shows the end point of the cable.
UCLASS()
class CMP302GRAPPLEHOOK_API ACableEndPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACableEndPoint();

	class USceneComponent* AttachmentPoint = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
