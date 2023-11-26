// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MarioLongJumpMovementComponent.generated.h"

/**
 *
 */
UCLASS()
class MARIOLONGJUMP_API UMarioLongJumpMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UMarioLongJumpMovementComponent();

	UPROPERTY(EditDefaultsOnly)
	float CurrnetMaxWalkSpeed;

protected:
	// Funtion used to update movement 
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

};