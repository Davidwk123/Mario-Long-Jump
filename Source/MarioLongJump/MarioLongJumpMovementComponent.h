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

protected:
	// Funtion used to update movement 
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Overrided function to check if the character can jump
	virtual bool CanAttemptJump() const override;

	// Overrided function to check if the character can crouch
	virtual bool CanCrouchInCurrentState() const override;

};