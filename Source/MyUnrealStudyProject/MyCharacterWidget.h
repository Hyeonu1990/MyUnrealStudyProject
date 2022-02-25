// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyCharacterWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYUNREALSTUDYPROJECT_API UMyCharacterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindHp(class UMyStatComponent* StatComp);

	void UpdateHp();
	
private:
	TWeakObjectPtr<class UMyStatComponent> CurrentStatComp; //weak_ptr

	UPROPERTY(meta=(BindWidget)) // 블루프린트에 있는거 알아서 찾아줌
	class UProgressBar* PB_HpBar;
};
