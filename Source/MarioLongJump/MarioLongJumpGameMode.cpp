// Copyright Epic Games, Inc. All Rights Reserved.

#include "MarioLongJumpGameMode.h"
#include "MarioLongJumpCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMarioLongJumpGameMode::AMarioLongJumpGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
