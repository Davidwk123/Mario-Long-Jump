// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MarioLongJumpMovementComponent.generated.h"

// Defines Slide movementmode
UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_Slide			UMETA(DisplayName = "Slide"),
};

/**
 *
 */
UCLASS()
class MARIOLONGJUMP_API UMarioLongJumpMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UMarioLongJumpMovementComponent();

	// Variable used to replace GetOwner() function call to be able to call custom functions 
	UPROPERTY(Transient)
	class AMarioLongJumpCharacter* MarioLongJumpCharacterOwner;

	// Function to check if a custom movement is being used
	bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

	// Slide Variables 
	float MinSlideSpeed = 400;
	float MaxSlideSpeed = 550;
	float GravitySlideForce = 500;
	float SlideFriction = .5;
	
protected:
	// Function used to setup component and connect custom characters variables 
	virtual void InitializeComponent() override;

	// Overrided function to check if the character can jump
	virtual bool CanAttemptJump() const override;

	// Overrided function to check if the character can crouch
	virtual bool CanCrouchInCurrentState() const override;

	// Overrided function to check if character is moving on ground
	virtual bool IsMovingOnGround() const override;

	// Overrided function to update crouching movement before sliding/walking
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	// Set custom slide movement 
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	// Overrided function for jump 
	virtual bool DoJump(bool bReplayingMoves) override;

private:
	// Checks if character is on the ground first before sliding 
	bool GetSlideSurface(FHitResult& SurfaceHit);

	// Functions that triggers the slide movement 
	void EnterSlide();
	void ExitSlide();

	// Sliding functionality
	void PhysSlide(float DeltaTime, int32 Iterations);

	// Forward direction of character when starting slide
	FVector SlideCharacterDirection = FVector::ZeroVector;
};