#include "MarioLongJumpMovementComponent.h"
#include "MarioLongJumpCharacter.h"

UMarioLongJumpMovementComponent::UMarioLongJumpMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
	bCanWalkOffLedgesWhenCrouching = true;
}

void UMarioLongJumpMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	MarioLongJumpCharacterOwner = Cast<AMarioLongJumpCharacter>(GetOwner());
}

void UMarioLongJumpMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

bool UMarioLongJumpMovementComponent::CanAttemptJump() const
{
	return IsJumpAllowed() && (IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}

bool UMarioLongJumpMovementComponent::CanCrouchInCurrentState() const
{
	if (!CanEverCrouch())
	{
		return false;
	}

	return IsMovingOnGround() && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

bool UMarioLongJumpMovementComponent::GetSlideSurface(FHitResult& SurfaceHit)
{
	// Create a raycast that is shot under the player to check if it hits any surface 
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start - UpdatedComponent->GetUpVector();

	return GetWorld()->LineTraceSingleByProfile(SurfaceHit, Start, End, TEXT("BlockAll"), MarioLongJumpCharacterOwner->GetIgnoreCharacterParams());
}

void UMarioLongJumpMovementComponent::EnterSlide()
{
	// Applies impluse velocity when users enters slide
	Velocity += Velocity.GetSafeNormal2D() * SlideImpluseSpeed;

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

	// Apply gravity(type of acceleration)
	Velocity += FVector::DownVector * GravitySlideForce * DeltaTime;

	// Strafe functionality, allows left and right movement when character is sliding
	// First check if the dot product between the normalized acceleration(Acceleration is retrived from user's input WASD) and movement component's
	// right vector is more then .5, meaning that the user had to press the 'A' or 'D' keys 
	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5)
	{
		// Second, project the acceleration vector onto the movement compoment's right vector to allow for left/right movement 
		Acceleration += Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
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
	FVector TimeStep = Velocity * DeltaTime;
	// Aligns velocity vector to the plane of the surface the character is sliding on, surface could be sloped, 
	// so we need to adjust the velocity orientation correctly
	FVector VelocityPlaneCorrected = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal).GetSafeNormal2D();
	// Apply corrected velocity to rotation X axis and SurfaceHit's normal to the Z axis
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelocityPlaneCorrected, SurfaceHit.Normal).ToQuat();

	// Helper function that applies slide displacement 
	SafeMoveUpdatedComponent(TimeStep, NewRotation, true, Hit);

	// Checks when character hits a wall/slope
	if (Hit.Time < 1.f)
	{
		// Needed helper functions to deal with collisions 
		HandleImpact(Hit, DeltaTime, TimeStep);
		SlideAlongSurface(TimeStep, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	// Second check if character is not on a surface or if speed is less then min slide speed
	FHitResult NewSurfaceHit;
	if (!GetSlideSurface(NewSurfaceHit) || Velocity.SizeSquared() < pow(MinSlideSpeed, 2))
	{
		ExitSlide();
	}

	// Update Outgoing Velocity & Acceleration
	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		// Velocity is updated based off if there was a collision or not 
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
	}
	
}

