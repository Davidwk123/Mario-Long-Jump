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
	bWantsToCrouch = false;

	FHitResult Surfacehit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();

	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Surfacehit);
	SetMovementMode(MOVE_Walking);
}

