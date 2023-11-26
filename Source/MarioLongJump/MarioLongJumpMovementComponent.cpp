#include "MarioLongJumpMovementComponent.h"

UMarioLongJumpMovementComponent::UMarioLongJumpMovementComponent()
{
}

void UMarioLongJumpMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	// Assign defined MaxWalkSpeed
	MaxWalkSpeed = CurrnetMaxWalkSpeed;
}
