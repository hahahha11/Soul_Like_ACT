// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/MyBTService_IsPlayerAlive.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "TargetableActor.h"
#include "Mob/MobController.h"
#include "StatusComponent.h"


UMyBTService_IsPlayerAlive::UMyBTService_IsPlayerAlive()
{
	bCreateNodeInstance = 1;
}


void UMyBTService_IsPlayerAlive::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);


	AMobController *Owner = Cast<AMobController>(OwnerComp.GetOwner());
	if (!Owner)
	{
		//OwnerComp.GetBlackboardComponent()->SetValueAsObject(FName("SelfActor"), nullptr);
		Owner->GetBTComp()->StopTree(EBTStopMode::Forced);
		return;
	}
	ATargetableActor *PlayerPawn = Cast<ATargetableActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("PlayerPawn"));
	if (!PlayerPawn || PlayerPawn->StatusComponent->GetHealthPercent() <= 0)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), 0);
		return;
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), 1);
	}
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::FromInt(++counter));
}
