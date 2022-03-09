// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirstPlayerGameMode.h"
#include "FirstPlayerHUD.h"
#include "FirstPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFirstPlayerGameMode::AFirstPlayerGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFirstPlayerHUD::StaticClass();
}
