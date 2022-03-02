// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

/**
 *
 */
UCLASS()
class MYUNREALSTUDYPROJECT_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	AMyGameModeBase();

public:
	UPROPERTY()
	TSubclassOf<UUserWidget> HUD_Class; // 어떤 클래스인지

	UPROPERTY()
	UUserWidget* CurrentWidget; // 실제 위젯의 주소
};
