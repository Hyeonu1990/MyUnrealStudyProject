// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterWidget.h"
#include "MyStatComponent.h"
#include "Components/ProgressBar.h"

void UMyCharacterWidget::BindHp(class UMyStatComponent* StatComp)
{
    //PB_HpBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PB_HpBar"))); // PB_HpBar 직접 찾을 경우
    CurrentStatComp = StatComp;
	StatComp->OnHpChanged.AddUObject(this, &UMyCharacterWidget::UpdateHp);

}

void UMyCharacterWidget::UpdateHp()
{
    if (CurrentStatComp.IsValid())
    {
	    PB_HpBar->SetPercent(CurrentStatComp->GetHpRatio());
    }
}

