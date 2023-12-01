#include "MarioLongJumpMovementComponent.h"

UMarioLongJumpMovementComponent::UMarioLongJumpMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
	bCanWalkOffLedgesWhenCrouching = true;
}

void UMarioLongJumpMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

