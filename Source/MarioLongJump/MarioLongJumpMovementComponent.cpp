#include "MarioLongJumpMovementComponent.h"
#include "MarioLongJumpCharacter.h"
#include "Components/CapsuleComponent.h"

UMarioLongJumpMovementComponent::UMarioLongJumpMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
	bCanWalkOffLedgesWhenCrouching = true;
}

bool UMarioLongJumpMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return (MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode);
}

void UMarioLongJumpMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	MarioLongJumpCharacterOwner = Cast<AMarioLongJumpCharacter>(GetOwner());
}

bool UMarioLongJumpMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump() || IsCustomMovementMode(CMOVE_Slide);
}

bool UMarioLongJumpMovementComponent::CanCrouchInCurrentState() const
{
	if (!CanEverCrouch())
	{
		return false;
	}
	// Character only can crouch on the ground 
	return (IsMovingOnGround()) && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

bool UMarioLongJumpMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

void UMarioLongJumpMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Trigger the crouch first, then potentially apply slide movement, bug occurs if this function is placed after movement checks 
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	

	// Check if wants to crouch	
	if (MovementMode == MOVE_Walking && bWantsToCrouch)
	{
		//UE_LOG(LogTemp, Warning, TEXT("walking and is croching"));
		FHitResult PossibleSurfaceHit;
		// Saftey checks
		//UE_LOG(LogTemp, Warning, TEXT("Surfacehit is: %s"), (GetSlideSurface(PossibleSurfaceHit) ? TEXT("True") : TEXT("False")));
		if (GetSlideSurface(PossibleSurfaceHit) && Velocity.SizeSquared() > pow(MinSlideSpeed, 2))
		{
			EnterSlide();
		}
	}

	// Check if character is walking 
	if (MovementMode == MOVE_Walking)
	{
		// Resets direction/rotation 
		SlideCharacterDirection = FVector::ZeroVector;
		RotationRate.Yaw = 500.f;
	}

	// Check if character stops sliding 
	if (IsCustomMovementMode(CMOVE_Slide) && bWantsToCrouch == false)
	{
		// Reset air drag 
		BrakingDecelerationFalling = 1000.f;
		SetMovementMode(MOVE_Walking);
	}
}

void UMarioLongJumpMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	// Add a new case where the custom movement is called, then apply slide physics function
	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	}
}

bool UMarioLongJumpMovementComponent::DoJump(bool bReplayingMoves)
{
	// Long jump functionality
	if (IsCustomMovementMode(CMOVE_Slide))
	{
		if (CharacterOwner && CharacterOwner->CanJump())
		{
			// Jump variables 
			// Played around with the variables to get an ideal long jump
			RotationRate = FRotator::ZeroRotator;
			AirControl = .3f;
			GravityScale = .9f;
			float ForwardScale = 900.f;
			float VerticalScale = .5f;

			// Don't jump if we can't move up/down.
			if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
			{
				// Long jump is decreased Z velocity and increased forward velocity
				Velocity.Z = FMath::Max<FVector::FReal>(Velocity.Z * VerticalScale, JumpZVelocity * VerticalScale);
				Velocity += Velocity.GetSafeNormal2D() * ForwardScale;

				// Get current direction of player before they land 
				SlideCharacterDirection = UpdatedComponent->GetForwardVector();

				SetMovementMode(MOVE_Falling);
				return true;
			}
		}

		return false;
	}
	// Regular jump
	else
	{
		// Reset to orignal values
		GravityScale = 1.75f;
		AirControl = 1.0f;
		return Super::DoJump(false);
	}

}

bool UMarioLongJumpMovementComponent::GetSlideSurface(FHitResult& SurfaceHit)
{
	// Create a raycast that is shot under the player to check if it hits any surface 
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + FVector::DownVector * MarioLongJumpCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f;

	return GetWorld()->LineTraceSingleByProfile(SurfaceHit, Start, End, TEXT("BlockAll"), MarioLongJumpCharacterOwner->GetIgnoreCharacterParams());
}

void UMarioLongJumpMovementComponent::EnterSlide()
{
	// Decrease air drag to allow for longer slide after character long jumps
	BrakingDecelerationFalling = 820.f;
	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UMarioLongJumpMovementComponent::ExitSlide()
{
	// Checks for case where character's velocity < MinSlideSpeed, we still want to stay in crouch
	if (Velocity.SizeSquared() < pow(MinSlideSpeed, 2))
	{
		bWantsToCrouch = true;
	}
	else
	{
		bWantsToCrouch = false;
	}

	// Variables that are needed for the movement udpate function 
	FHitResult Surfacehit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();

	// Reset rotation
	RotationRate.Yaw = 500.f;
	// Reset air drag 
	BrakingDecelerationFalling = 1000.f;

	// Delta timestep is a zero vector to stop movement, the rotation is current components foward vector and a regular up vector
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Surfacehit);
	SetMovementMode(MOVE_Walking);	
}

void UMarioLongJumpMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	// Check to prevent division by zero
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}
	// Needed helper function
	RestorePreAdditiveRootMotionVelocity();

	// Check if character is not on a surface or if speed is less then min slide speed, if so, exit slide movement
	FHitResult SurfaceHit;
	if (GetSlideSurface(SurfaceHit) == false || Velocity.SizeSquared() < pow(MinSlideSpeed, 2))
	{
		ExitSlide();
		// Needed helper function
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	// Apply gravity to allow increased speed down slopes 
	Velocity += FVector::DownVector * GravitySlideForce * DeltaTime;

	// Strafe functionality, allows left and right movement when character is sliding
	// First check if the dot product between the normalized acceleration(Acceleration is retrived from user's input WASD) and movement component's
	// right vector is more then .5, meaning that the user is pressing the 'A' or 'D' key
	float StrafeThreshold = FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector());
	if (FMath::Abs(StrafeThreshold) > .5f)
	{
		// Set accelration to zero so that charcter's direction does not change 
		Acceleration = FVector::ZeroVector;
		FVector Displacement = FVector::ZeroVector;
		float LaterlImpulse = Velocity.Length() * .5f;
		
		if (StrafeThreshold != 0)
		{
			Displacement = UpdatedComponent->GetRightVector() * LaterlImpulse * StrafeThreshold;
		}
		
		// Move the character left/right by applying the displacement helper function
		UpdatedComponent->MoveComponent(Displacement * DeltaTime, UpdatedComponent->GetComponentRotation(), true);
		
	}
	else
	{
		// If user is pressing 'W' or 'D' don't add any extra movement 
		Acceleration = FVector::ZeroVector;
	}

	// Calc Velocity
	// Needed helper functions
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		// Build in function used to calculate the velocity, also applies the friction force  
		CalcVelocity(DeltaTime, SlideFriction, true, GetMaxBrakingDeceleration());

		// Check if velocity hits max speed
		if (Velocity.SizeSquared() > pow(MaxSlideSpeed, 2))
		{
			Velocity = Velocity.GetSafeNormal() * MaxSlideSpeed;
		}

		// Check if player long jumped, then apply current direction to slide velocity
		if (SlideCharacterDirection != FVector::ZeroVector)
		{
			Velocity = Velocity.ProjectOnTo(SlideCharacterDirection).GetSafeNormal() * Velocity.Length();
		}
	}
	// Needed helper function
	ApplyRootMotionToVelocity(DeltaTime);

	// Slide displacement 
	Iterations++;
	bJustTeleported = false;

	// Variables needed to apply slide displacement 
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	// FHitResult variable is needed in case character collides with a wall
	FHitResult Hit;
	// How much player will move each slide 
	FVector TimeStep = Velocity * DeltaTime;

	// Debug
	/*FVector Start = SurfaceHit.ImpactPoint;
	FVector End = Start + UpdatedComponent->GetForwardVector() * 100.f;
	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, -1, 0, 2.0f);
	UE_LOG(LogTemp, Log, TEXT("Character Acceleration: X = %f"), Velocity.Length());*/

	// Helper function that applies slide displacement 
	SafeMoveUpdatedComponent(TimeStep, UpdatedComponent->GetComponentQuat(), true, Hit);

	// Checks when character hits a wall/slope
	if (Hit.Time < 1.f)
	{
		// Needed helper functions to deal with collisions and sliding phyiscs 
		HandleImpact(Hit, DeltaTime, TimeStep);
		SlideAlongSurface(TimeStep, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	// Second saftey check so that character does not slide forever 
	FHitResult NewSurfaceHit;
	if (!GetSlideSurface(NewSurfaceHit) || Velocity.SizeSquared() < pow(MinSlideSpeed, 2))
	{
		ExitSlide();
	}

	// Update Outgoing Velocity & Acceleration, the checks are needed helper functions
	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		// Velocity is updated based off if there was a collision or not 
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
	}
	
}
