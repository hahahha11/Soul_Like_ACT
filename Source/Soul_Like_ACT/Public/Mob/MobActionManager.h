// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MobActionManager.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOUL_LIKE_ACT_API UMobActionManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMobActionManager();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsActing;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsBlocking;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bCanParry;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsSwinging;
	UPROPERTY(BlueprintReadWrite)
		bool bIsStun;
};
