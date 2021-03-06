// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockTargetComponent.generated.h"

UENUM(BlueprintType)
enum class ETargetFindingDirection : uint8
{
    Centre,
    Left,
    Right,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOUL_LIKE_ACT_API ULockTargetComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    FTimerHandle TargetBlockingHandler;

    bool bFreeCamera;

    bool bIsFacingOffsetEnabled;
    float FacingOffsetDelta;
    float FacingOffset_ForwardRotationYaw;
    float FacingOffset_CurrentRotationYaw;

    bool bOwnerControllerRotationYaw = false
         , bOwnerOrientRotToMovement = true
         , bOwnerControllerDesiredRot = false;

    TArray<AActor*> PotentialTargetActors;

    class UArrowComponent* PlayerArrow;
    class ACharacter* PlayerRef;

public:
    // Sets default values for this component's properties
    ULockTargetComponent();

    class AActor* LockedTarget;

    UPROPERTY(BlueprintReadOnly)
    bool isTargetingEnabled;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    //Detection Stages-----------
    void FindTarget(ETargetFindingDirection Direction = ETargetFindingDirection::Centre);

    void GetPotentialTargetsInScreen(TArray<AActor*>& OutPotentialTargets);
    void RuleOutBlockedTargets(TArray<AActor*> LocalPotentialTargets, TArray<AActor*>& OutPotentialTargets);

    void FindClosestTargetInScreen(TArray<AActor*>& LocalPotentialTargets, AActor*& ClosestTarget);
    void Find_InDirection(TArray<AActor*>& LocalPotentialTargets, AActor*& ClosestTarget,
                          ETargetFindingDirection Direction);
    //---------------------------

    void EnableLockingTarget();
    void DisableLockingTarget();

    void CacheRotationSetting();
    void ResetRotationSetting();

    void SetRotationMode_FaceTarget();

    bool IsTraceBlocked(AActor* SelectedTarget, TArray<AActor*> IgnoredActors, const ECollisionChannel TraceChannel);
    FVector GetLineTraceStartLocation();

    void Timer_CheckBlockingAndDistance();

    void Tick_UpdateRotation();

    void InitComponent(class UArrowComponent* ArrowComponentRef);
public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable)
    void ToggleCameraLock(bool FreeCamera);

    UFUNCTION(BlueprintCallable)
    void Toggle_InDirection(ETargetFindingDirection Direction) { FindTarget(Direction); }

    bool GetIsTargetingEnabled() { return isTargetingEnabled; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = TargetLocking)
    void GetLockedTarget(bool& isEnabled, AActor*& OutLockedTarget) const
    {
        isEnabled = isTargetingEnabled;
        OutLockedTarget = LockedTarget;
    }

    FVector GetNormalizedVec(FVector Inp);

    friend class ASoul_Like_ACTCharacter;
};
