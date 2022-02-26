// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAIController.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

AMyAIController::AMyAIController()
{
    static ConstructorHelpers::FObjectFinder<UBehaviorTree> BT(TEXT("BehaviorTree'/Game/AI/BT_MyCharacter.BT_MyCharacter'"));
    if (BT.Succeeded())
    {
        BehaviorTree = BT.Object;
    }

	static ConstructorHelpers::FObjectFinder<UBlackboardData> BD(TEXT("BlackboardData'/Game/AI/BB_MyCharacter.BB_MyCharacter'"));
    if (BD.Succeeded())
    {
        BlackboardData = BD.Object;
    }
}

void AMyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

    //GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMyAIController::RandomMove, 3.f, true);

    if (UseBlackboard(BlackboardData, Blackboard))
    {
        if (RunBehaviorTree(BehaviorTree))
        {
            //
        }
    }
}

void AMyAIController::OnUnPossess()
{
	Super::OnUnPossess();

    //GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void AMyAIController::RandomMove()
{
    UE_LOG(LogTemp, Log, TEXT("RandomMove"));
	auto CurrentPawn = GetPawn();

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
    if (NavSystem == nullptr)
        return;
    
    FNavLocation RandomLocation;

    if (NavSystem->GetRandomPointInNavigableRadius(FVector::ZeroVector, 500.f, RandomLocation))
    {
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, RandomLocation);
    }
}

