// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyStatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnHpChanged); // 구성에 따라 인자값 포함해서 보내주는 것도 가능

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYUNREALSTUDYPROJECT_API UMyStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyStatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;

public:
	void SetLevel(int32 NewLevel);
	void SetHp(int32 NewHp);
	void OnAttacked(float DamageAmount);

	int32 GetLevel() { return Level; }
	int32 GetHp() { return Hp; }
	int32 GetMaxHp() { return MaxHp; }
	float GetHpRatio() { return Hp / static_cast<float>(MaxHp); }
	int32 GetAttack() { return Attack; }

private:
	UPROPERTY(EditAnywhere, Category=Stat, Meta=(AllowPrivateAccess=true))
	int32 Level;

	UPROPERTY(EditAnywhere, Category=Stat, Meta=(AllowPrivateAccess=true))
	int32 Hp;

	UPROPERTY(EditAnywhere, Category=Stat, Meta=(AllowPrivateAccess=true))
	int32 MaxHp;

	UPROPERTY(EditAnywhere, Category=Stat, Meta=(AllowPrivateAccess=true))
	int32 Attack;

public:
	FOnHpChanged OnHpChanged;
};
