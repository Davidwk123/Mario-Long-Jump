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

	// Slide Variables 
	float MinSlideSpeed = 400;
	float MaxSlideSpeed = 800;
	float SlideImpluseSpeed = 200;
	float GravitySlideForce;
	float SlideFriction = .1;
	
protected:
	// Function used to setup component and connect custom characters variables 
	virtual void InitializeComponent() override;

	// Funtion used to update movement 
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Overrided function to check if the character can jump
	virtual bool CanAttemptJump() const override;

	// Overrided function to check if the character can crouch
	virtual bool CanCrouchInCurrentState() const override;

private:
	// Checks if character is on the ground first before sliding 
	bool GetSlideSurface(FHitResult& SurfaceHit);

	// Functions that triggers the slide movement 
	void EnterSlide();
	void ExitSlide();
};