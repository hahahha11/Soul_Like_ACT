// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Soul_Like_ACT.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Abilities/SoulAbilitySystemComponent.h"
#include "Abilities/SoulAttributeSet.h"
#include "Interfaces/Targetable.h"
#include "SoulCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChanged, const TArray<float> &, values);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTrigger_OnMeleeAttack, AActor*, SourceActor, AActor*, TargetActor,
                                               const FHitResult, HitResult);

UENUM(BlueprintType)
enum class EIsControllerValid : uint8
{
    IsValid,
    IsNotValid,
};

#define ATTRIBUTE_GETTER(PropertyName) \
	UFUNCTION(BlueprintCallable) \
	float Get##PropertyName##() const \
	{ \
		return AttributeSet->Get##PropertyName##(); \
	}

#define ATTRIBUTE_GETTER_AND_HANDLECHANGED_OneParam(PropertyName) \
	ATTRIBUTE_GETTER(PropertyName) \
	void Handle##PropertyName##Changed(const FOnAttributeChangeData& Data) \
	{ \
		if(On##PropertyName##Changed.IsBound()) \
			On##PropertyName##Changed.Broadcast(TArray<float>{Get##PropertyName##(), -1.f}); \
	}

#define ATTRIBUTE_GETTER_AND_HANDLECHANGED_TwoParams(PropertyName) \
	ATTRIBUTE_GETTER(##PropertyName##) \
	void Handle##PropertyName##Changed(const FOnAttributeChangeData& Data) \
	{ \
		if(On##PropertyName##Changed.IsBound()) \
			On##PropertyName##Changed.Broadcast(TArray<float>{Get##PropertyName##(), GetMax##PropertyName##()}); \
	}

UENUM(BlueprintType)
enum class EActorFaction : uint8
{
    Untargetable,
    Player,
    Enemy
};

UENUM(BlueprintType)
enum class ESoulMovementMode : uint8
{
    Idle,
    Run,
    Sprint
};

UCLASS()
class SOUL_LIKE_ACT_API ASoulCharacterBase : public ACharacter, public ITargetable, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    ASoulCharacterBase();

    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;

protected:
    UFUNCTION(BlueprintCallable)
    void BindOnAttributesChanged();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UWidgetComponent* TargetIcon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UActorFXManager* FXManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class USoulModifierManager* ModifierManager;

    /** The component used to handle ability system interactions */
    UPROPERTY(BlueprintReadOnly)
    class USoulAbilitySystemComponent* AbilitySystemComponent;

    /** List of attributes modified by the ability system */
    UPROPERTY()
    USoulAttributeSet* AttributeSet;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = GameplayEffects)
    TSubclassOf<UGameplayEffect> DeadGE_Class;    

    FTimerHandle Handle_SlowMotion, Handler_SlowMotionDelay;

    void WaitForDilationReset()
    {
        CustomTimeDilation = 1.f;
    }

    void TriggerSlowMotion()
    {
        CustomTimeDilation = 0.003f;
        GetWorldTimerManager().SetTimer(Handle_SlowMotion, this, &ASoulCharacterBase::WaitForDilationReset, 1.f, false,
                                        0.2f);
    }


public:
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    virtual USoulModifierManager* GetModifierManager() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EActorFaction Faction;

    static bool IsInRivalFaction(ASoulCharacterBase* DamageDealer, ASoulCharacterBase* DamageReceiver);

    UFUNCTION(BlueprintCallable)
    virtual bool IsTargetable() const override
    {
        return (Faction != EActorFaction::Untargetable
            && !GetIsDead());
    }

    UFUNCTION(BlueprintCallable)
    virtual void ToggleLockIcon() override;

    UFUNCTION(BlueprintCallable)
    virtual bool IsAlive() const { return GetHealth() > 0.f; }

    //Called by WeaponActor and OnHit
    UFUNCTION(BlueprintCallable)
    void TriggerSlowMotion_WithDelay(float Delay);

    void Notify_OnMeleeAttack(AActor* TargetActor, const FHitResult HitResult)
    {
        if (OnMeleeAttack.IsBound())
            OnMeleeAttack.Broadcast(this, TargetActor, HitResult);
    }

    void Notify_OnMeleeKill(AActor* SourceActor, AActor* TargetActor, const FHitResult HitResult)
    {
        if (OnMeleeKill.IsBound())
            OnMeleeKill.Broadcast(SourceActor, TargetActor, HitResult);
    }

    ATTRIBUTE_GETTER_AND_HANDLECHANGED_TwoParams(Health);
    ATTRIBUTE_GETTER(MaxHealth);
    ATTRIBUTE_GETTER_AND_HANDLECHANGED_TwoParams(Posture);
    ATTRIBUTE_GETTER(MaxPosture);
    ATTRIBUTE_GETTER_AND_HANDLECHANGED_OneParam(Leech);
    ATTRIBUTE_GETTER_AND_HANDLECHANGED_OneParam(PostureStrength);
    ATTRIBUTE_GETTER_AND_HANDLECHANGED_OneParam(DefensePower);
    ATTRIBUTE_GETTER_AND_HANDLECHANGED_OneParam(AttackPower);
    ATTRIBUTE_GETTER_AND_HANDLECHANGED_OneParam(PostureCrumble);
    ATTRIBUTE_GETTER(MoveSpeed);
    virtual void HandleMoveSpeedChanged(const FOnAttributeChangeData& Data);
    ATTRIBUTE_GETTER_AND_HANDLECHANGED_OneParam(AttackSpeed);
    ATTRIBUTE_GETTER_AND_HANDLECHANGED_OneParam(CriticalStrike);
    ATTRIBUTE_GETTER_AND_HANDLECHANGED_OneParam(CriticalMulti);

    UFUNCTION(BlueprintCallable)
    float GetPosturePercent() { return GetPosture() / GetMaxPosture(); }

    /** Returns the character level that is passed to the ability system */
    UFUNCTION(BlueprintCallable)
    virtual int32 GetCharacterLevel() const { return 1; }

    UFUNCTION(BlueprintCallable)
    bool GetIsDead() const
    {
        return AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Ailment.Dead"));;
    }
    
    UFUNCTION(BlueprintCallable)
    bool GetIsStun() const
    {
        return AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Ailment.Stun"));
    }

    UFUNCTION(BlueprintCallable)
    bool GetIsCrumbled() const
    {
        return AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Ailment.Crumbled"));
    }

    UFUNCTION(BlueprintCallable)
    bool GetIsHealthZero() const { return FMath::IsNearlyEqual(GetHealth(), 0.f); }
    UFUNCTION(BlueprintCallable)
    bool GetIsPostureFull() const { return FMath::IsNearlyEqual(GetPosture(), GetMaxPosture()); }

protected:
    /** Apply the startup GAs and GEs */
    UFUNCTION(BlueprintCallable)
    void AddStartupGameplayAbilities();

    /**
     * Called when character takes damage, which may have killed them
     *
     * @param DamageAmount Amount of damage that was done, not clamped based on current health
     * @param HitInfo The hit info that generated this damage
     * @param DamageTags The gameplay tags of the event that did the damage
     * @param InstigatorCharacter The character that initiated this damage
     * @param DamageCauser The actual actor that did the damage, might be a weapon or projectile
     */    
    UFUNCTION(BlueprintImplementableEvent)
    void OnDamaged(float DamageAmount, const bool IsCriticaled, const bool bIsStun, const FHitResult& HitInfo,
                   const struct FGameplayTagContainer& DamageTags, ASoulCharacterBase* InstigatorCharacter,
                   AActor* DamageCauser);

    UFUNCTION(BlueprintImplementableEvent)
    void OnDotDamaged(float DamageAmount, const bool IsCriticaled, const bool bIsStun, const FHitResult& HitInfo,
                      const struct FGameplayTagContainer& DamageTags, ASoulCharacterBase* InstigatorCharacter,
                      AActor* DamageCauser);

    /**
    *Called when character takes posture damage
    */
    UFUNCTION(BlueprintImplementableEvent)
    void OnPostureDamaged(float PostureDamageAmount, const bool IsCriticaled, const FHitResult& HitInfo,
                          const struct FGameplayTagContainer& DamageTags, ASoulCharacterBase* InstigatorCharacter,
                          AActor* DamageCauser);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_OnDead(const FHitResult& HitInfo,
                   const struct FGameplayTagContainer& DamageTags, ASoulCharacterBase* InstigatorCharacter,
                   AActor* DamageCauser);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_OnCrumbled(float PostureDamageAmount, const bool IsCriticaled, const FHitResult& HitInfo,
                          const struct FGameplayTagContainer& DamageTags, ASoulCharacterBase* InstigatorCharacter,
                          AActor* DamageCauser);


#pragma region GameplayEffect_Delegate
    UFUNCTION(BlueprintImplementableEvent)
    void BP_OnGameplayEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle);
    UFUNCTION(BlueprintImplementableEvent)
    void BP_OnGameplayEffectRemoved(const FActiveGameplayEffect& EffectRemovalInfo);
#pragma endregion
    
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnHealthChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnPostureChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnMoveSpeedChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnPostureStrengthChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnDefensePowerChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnLeechChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnPostureCrumbleChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnAttackPowerChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnAttackSpeedChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnCriticalStrikeChanged;
    UPROPERTY(BlueprintAssignable)
    FOnChanged OnCriticalMultiChanged;

    UPROPERTY(BlueprintAssignable)
    FTrigger_OnMeleeAttack OnMeleeAttack;

    UPROPERTY(BlueprintAssignable)
    FTrigger_OnMeleeAttack OnMeleeKill;

    // Called from RPGAttributeSet, these call BP events above
    virtual void HandleDamage(float DamageAmount, const bool IsCriticaled, const bool bIsStun,
                              const FHitResult& HitInfo, const struct FGameplayTagContainer& DamageTags,
                              ASoulCharacterBase* InstigatorCharacter, AActor* DamageCauser);
    virtual void HandleDotDamage(float DamageAmount, const bool IsCriticaled, const bool bIsStun,
                                 const FHitResult& HitInfo, const struct FGameplayTagContainer& DamageTags,
                                 ASoulCharacterBase* InstigatorCharacter, AActor* DamageCauser);
    virtual void HandleOnDead(const FHitResult& HitInfo,
                          const struct FGameplayTagContainer& DamageTags, ASoulCharacterBase* InstigatorCharacter,
                          AActor* DamageCauser);

    virtual void HandlePostureDamage(float PostureDamageAmount, const bool IsCriticaled, const FHitResult& HitInfo,
                                     const struct FGameplayTagContainer& DamageTags,
                                     ASoulCharacterBase* InstigatorCharacter, AActor* DamageCauser);

    virtual void HandleOnCrumble(float PostureDamageAmount, const bool IsCriticaled, const FHitResult& HitInfo,
                      const struct FGameplayTagContainer& DamageTags, ASoulCharacterBase* InstigatorCharacter,
                      AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent)
    void MakeStepDecelAndSound();

    UFUNCTION(BlueprintCallable)
    void ResetPerilousStatus();

    UPROPERTY()
    TArray<ASoulCharacterBase*> CounterTargets;
    UPROPERTY()
    bool bCounterEnabled;

    virtual void ForceOverrideFacingDirection(float Alpha){ return; }

public:
    UFUNCTION(BlueprintCallable)
    static void TagContainerToString(const FGameplayTagContainer& Container, FString& Outp)
    {
        Outp = Container.ToString();
    }

    UFUNCTION(BlueprintCallable)
    static void MakeStepDecelAndSound_Notify(ASoulCharacterBase* CharacterRef);

    friend USoulAttributeSet;

    UFUNCTION(BlueprintCallable)
    float GetAP_TEST() const { return GetAttackPower(); }

    UFUNCTION(BlueprintCallable, category = Movement)
    virtual void GetMovementMode(ESoulMovementMode& MovementMode) const;

    UFUNCTION(BlueprintCallable, Category = Direction)
    void BP_ForceOverrideFacingDirection(float Speed) { return ForceOverrideFacingDirection(Speed);}

    /**
     * Call the function in ANS_BodySweeping
     * If bUseTarget and Target is valid, then it sweeps to the target.
     * Otherwise, it sweeps to the forward.
     */
    UFUNCTION(BlueprintCallable)
    bool BodySweep_Init(const AActor* Target, bool bUseTarget, float InSweepingSpeed);

    FVector BodySweep_ForwardVec;
    float SweepingSpeed;
    
    UFUNCTION(BlueprintCallable)
    void BodySweep_Tick(float Delta);
    
    UFUNCTION(BlueprintCallable)
    void BodySweep_Finished();
};
