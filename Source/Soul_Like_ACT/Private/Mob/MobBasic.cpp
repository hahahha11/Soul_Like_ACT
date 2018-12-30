// Fill out your copyright notice in the Description page of Project Settings.

#include "MobBasic.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "StatusComponent.h"

// Sets default values
AMobBasic::AMobBasic()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Faction = EActorFaction::Enemy;
}

// Called when the game starts or when spawned
void AMobBasic::BeginPlay()
{
	Super::BeginPlay();
	
	StatusComponent->OnActorHealthChanged.AddDynamic(this, &AMobBasic::OnDeath);
}



void AMobBasic::OnDeath_Implementation(int32 CurrHealth, int32 MaxHealth)
{
	if (CurrHealth <= 0.f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, GetName() + " is dead");

		//Remove Collision
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

		AController *LocalController = GetController();
		if (LocalController)
			LocalController->UnPossess();

		Faction = EActorFaction::Untargetable;

		ToggleLockIcon(0);
	}
}

// Called every frame
void AMobBasic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMobBasic::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

