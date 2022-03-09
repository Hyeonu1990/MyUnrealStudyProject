// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingGameMode.h"
#include "ShootingCharacter.h"
#include "UObject/ConstructorHelpers.h"

AShootingGameMode::AShootingGameMode()
	: Super()
{
    DefaultPawnClass = AShootingCharacter::StaticClass();
	
    // static ConstructorHelpers::FClassFinder<ACharacter> BP_Char(TEXT("Blueprint'/Game/Blueprints/BP_ShootingCharacter.BP_ShootingCharacter_C'"));
	// if (BP_Char.Succeeded())
	// {
	// 	DefaultPawnClass = BP_Char.Class;
	// }
}
