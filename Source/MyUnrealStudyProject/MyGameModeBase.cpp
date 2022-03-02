// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "MyCharacter.h"
#include "MyHUD.h"

AMyGameModeBase::AMyGameModeBase()
{
	DefaultPawnClass = AMyCharacter::StaticClass();
#pragma region BP사용 시 할당하는 법
	static ConstructorHelpers::FClassFinder<ACharacter> BP_Char(TEXT("Blueprint'/Game/Blueprints/BP_MyCharacter.BP_MyCharacter_C'"));
	if (BP_Char.Succeeded())
	{
		DefaultPawnClass = BP_Char.Class;
	}
#pragma endregion

	static ConstructorHelpers::FClassFinder<UMyHUD> UI_HUD(TEXT("WidgetBlueprint'/Game/UI/WBP_HUD.WBP_HUD_C'"));
	if (UI_HUD.Succeeded())
	{
		HUD_Class = UI_HUD.Class;

		CurrentWidget = CreateWidget(GetWorld(), HUD_Class);
		if (CurrentWidget)
		{
			CurrentWidget->AddToViewport();
			//CurrentWidget->RemoveFromViewport();
		}
	}
}

